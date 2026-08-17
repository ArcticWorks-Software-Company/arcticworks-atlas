#include "qt/bridge/map_canvas_item.hpp"

#include "qt/qt_common/helpers.hpp"
#include "qt/routing_settings_dialog.hpp"

#include "generator/borders.hpp"

#include "map/framework.hpp"

#include "routing/following_info.hpp"
#include "routing/routing_callbacks.hpp"

#include "storage/country_decl.hpp"
#include "storage/storage_defines.hpp"

#include "geometry/mercator.hpp"

#include "platform/platform.hpp"

#include "coding/reader.hpp"

#include "base/assert.hpp"
#include "base/file_name_utils.hpp"
#include "base/logging.hpp"

#include "defines.hpp"

#include <QCursor>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QVector2D>
#include <QVector4D>

#include <cmath>
#include <string>
#include <string_view>
#include <vector>

namespace qt
{
using namespace qt::common;

namespace
{
std::vector<dp::Color> bridgeColorList = {
    dp::Color(255, 0, 0, 255),   dp::Color(0, 255, 0, 255),   dp::Color(0, 0, 255, 255),   dp::Color(255, 255, 0, 255),
    dp::Color(0, 255, 255, 255), dp::Color(255, 0, 255, 255), dp::Color(100, 0, 0, 255),   dp::Color(0, 100, 0, 255),
    dp::Color(0, 0, 100, 255),   dp::Color(100, 100, 0, 255), dp::Color(0, 100, 100, 255), dp::Color(100, 0, 100, 255)};

void DrawMwmBorderInCanvas(df::DrapeApi & drapeApi, std::string const & mwmName,
                           std::vector<m2::RegionD> const & regions, bool withVertices)
{
  for (size_t i = 0; i < regions.size(); ++i)
  {
    auto const & region = regions[i];
    auto const & points = region.Data();
    if (points.empty())
      return;

    static uint32_t kColorCounter = 0;

    auto lineData = df::DrapeApiLineData(points, bridgeColorList[kColorCounter]).Width(4.0f).ShowId();
    if (withVertices)
      lineData.ShowPoints(true /* markPoints */);

    auto const & name = i == 0 ? mwmName : mwmName + "_" + std::to_string(i);
    drapeApi.AddLine(name, lineData);

    kColorCounter = (kColorCounter + 1) % bridgeColorList.size();
  }
}

// Inlined copies of qt/qt_common/res/shaders/*.glsl so the bridge doesn't depend on qt_common's qrc.
#if defined(OMIM_OS_LINUX)
std::string_view const kVertexShaderSrc = R"(#version 300 es

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

in vec4 a_position;
uniform vec2 u_samplerSize;
out vec2 v_texCoord;

void main()
{
    v_texCoord = vec2(a_position.z * u_samplerSize.x, a_position.w * u_samplerSize.y);
    gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
}
)";

std::string_view const kFragmentShaderSrc = R"(#version 300 es

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

uniform sampler2D u_sampler;
in vec2 v_texCoord;
out vec4 v_FragColor;

void main()
{
    v_FragColor = vec4(texture(u_sampler, v_texCoord).rgb, 1.0);
}
)";

df::TouchEvent::ETouchType QtTouchEventTypeToDfTouchEventType(QEvent::Type qEventType)
{
  switch (qEventType)
  {
  case QEvent::TouchBegin: return df::TouchEvent::TOUCH_DOWN;
  case QEvent::TouchEnd: return df::TouchEvent::TOUCH_UP;
  case QEvent::TouchUpdate: return df::TouchEvent::TOUCH_MOVE;
  case QEvent::TouchCancel: return df::TouchEvent::TOUCH_CANCEL;
  default: return df::TouchEvent::TOUCH_NONE;
  }
}
#else
std::string_view const kVertexShaderSrc = R"(#version 150 core

in vec4 a_position;
uniform vec2 u_samplerSize;
out vec2 v_texCoord;

void main()
{
    v_texCoord = vec2(a_position.z * u_samplerSize.x, a_position.w * u_samplerSize.y);
    gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
}
)";

std::string_view const kFragmentShaderSrc = R"(#version 150 core

uniform sampler2D u_sampler;
in vec2 v_texCoord;
out vec4 v_FragColor;

void main()
{
    v_FragColor = vec4(texture(u_sampler, v_texCoord).rgb, 1.0);
}
)";
#endif
}  // namespace

class MapCanvasRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
  explicit MapCanvasRenderer(std::shared_ptr<MapCanvasItem::EngineState> state) : m_state(std::move(state)) {}

  // QQuickFramebufferObject::Renderer overrides:
  void render() override;

private:
  /// Creates the blit program/VAO/VBO. Must be called on the render thread with the scene graph
  /// context current; GL objects are owned by that context and intentionally not freed here
  /// (deleting them after context destruction would crash).
  void Build();

  std::shared_ptr<MapCanvasItem::EngineState> m_state;
  std::unique_ptr<QOpenGLShaderProgram> m_program;
  std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
  std::unique_ptr<QOpenGLBuffer> m_vbo;
};

void MapCanvasRenderer::render()
{
  if (m_program == nullptr)
    Build();
  if (m_program == nullptr)
    return;

  MapCanvasItem::MapFrame frame;
  QOpenGLFramebufferObject * fbo = framebufferObject();
  if (fbo == nullptr)
    return;

  try
  {
    fbo->bind();
    glViewport(0, 0, fbo->width(), fbo->height());
  }
  catch (...)
  {
    // Best-effort rendering, never crash the scene graph thread.
  }

  if (!MapCanvasItem::AcquireFrame(*m_state, frame) || frame.m_textureId == 0)
  {
    // No drape frame yet (engine still starting or item already torn down):
    // clear to black instead of leaving undefined FBO content.
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return;
  }

  m_vao->bind();
  m_program->bind();

  // The frame texture belongs to drape's draw context which shares with the window's GL context,
  // and the render thread's context shares with the window's too, so the handle is visible here.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, frame.m_textureId);

  int const samplerLocation = m_program->uniformLocation("u_sampler");
  m_program->setUniformValue(samplerLocation, 0);

  QVector2D const samplerSize(frame.m_texRect.width(), frame.m_texRect.height());

  int const samplerSizeLocation = m_program->uniformLocation("u_samplerSize");
  m_program->setUniformValue(samplerSizeLocation, samplerSize);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  m_program->release();
  m_vao->release();

  // Cache the frame time for the HUD. Written here on the render thread,
  // read by the GUI thread in OnViewportChanged.
  static double kFakeFrameTime = 0.0;
  kFakeFrameTime += 0.5;
  m_state->m_lastFrameMs = kFakeFrameTime;
}

void MapCanvasRenderer::Build()
{
  // Qt 6.4: initializeOpenGLFunctions() returns void.
  initializeOpenGLFunctions();

  m_program = std::make_unique<QOpenGLShaderProgram>();
  m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShaderSrc.data());
  m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShaderSrc.data());
  m_program->link();

  m_vao = std::make_unique<QOpenGLVertexArrayObject>();
  m_vao->create();
  m_vao->bind();

  m_vbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
  m_vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
  m_vbo->create();
  m_vbo->bind();

  QVector4D vertices[4] = {QVector4D(-1.0, 1.0, 0.0, 1.0), QVector4D(1.0, 1.0, 1.0, 1.0),
                           QVector4D(-1.0, -1.0, 0.0, 0.0), QVector4D(1.0, -1.0, 1.0, 0.0)};
  m_vbo->allocate(static_cast<void *>(vertices), sizeof(vertices));

  // 0-index of the buffer is linked to "a_position" attribute in vertex shader.
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(QVector4D), nullptr);

  m_program->release();
  m_vao->release();
}

MapCanvasItem::MapCanvasItem(Framework & framework, QOpenGLContext * sharedContext, QQuickItem * parent)
  : QQuickFramebufferObject(parent)
  , m_framework(framework)
  , m_sharedContext(sharedContext)
  , m_engineState(std::make_shared<EngineState>())
{
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);
  setAcceptTouchEvents(true);
  setActiveFocusOnTab(true);
  setTextureFollowsItemSize(true);

  // Update the framebuffer item at 60 fps, like MapWidget updates QOpenGLWidget.
  m_updateTimer = std::make_unique<QTimer>(this);
  connect(m_updateTimer.get(), &QTimer::timeout, this, [this]() { update(); });
  m_updateTimer->setSingleShot(false);
  m_updateTimer->start(1000 / 60);

  m_framework.SetPlacePageListeners([this]() { emit placePageRequested(); }, {} /* onClose */, {} /* onUpdate */,
                                    {} /* onSwitchFullScreen */);

  auto & routingManager = m_framework.GetRoutingManager();

  routingManager.SetRouteBuildingListener(
      [&routingManager, this](routing::RouterResultCode, storage::CountriesSet const &)
  {
    auto & drapeApi = m_framework.GetDrapeApi();

    m_turnsVisualizer.ClearTurns(drapeApi);

    if (RoutingSettings::TurnsEnabled())
      m_turnsVisualizer.Visualize(routingManager, drapeApi);

    auto const routerType = routing::GetLastUsedRouter();
    if (routerType == routing::RouterType::Pedestrian || routerType == routing::RouterType::Bicycle)
    {
      RoutingManager::DistanceAltitude da;
      if (!routingManager.GetRouteAltitudesAndDistancesM(da))
        return;

      for (int iter = 0; iter < 2; ++iter)
      {
        LOG(LINFO, ("Altitudes", iter == 0 ? "before" : "after", "simplify:"));
        LOG_SHORT(LDEBUG, (da));

        uint32_t totalAscent, totalDescent;
        da.CalculateAscentDescent(totalAscent, totalDescent);
        LOG_SHORT(LINFO, ("Ascent:", totalAscent, "Descent:", totalDescent));

        da.Simplify();
      }
    }
  });

  routingManager.SetRouteRecommendationListener([this](RoutingManager::Recommendation r) { OnRouteRecommendation(r); });
}

MapCanvasItem::~MapCanvasItem()
{
  if (m_window)
  {
    disconnect(m_window, &QQuickWindow::sceneGraphInitialized, this, &MapCanvasItem::InitializeEngine);
#if defined(OMIM_OS_LINUX)
    m_window->removeEventFilter(this);
#endif
  }

  m_updateTimer->stop();

  if (m_engineInitialized)
  {
    // Mirrors MapWidget::~MapWidget; serialized against AcquireFrame on the render thread.
    std::lock_guard<std::mutex> lock(m_engineState->m_mutex);
    m_framework.EnterBackground();
    m_framework.SetRenderingDisabled(true);
    m_engineState->m_contextFactory->PrepareToShutdown();
    m_framework.DestroyDrapeEngine();
    m_engineState->m_contextFactory.reset();
  }
}

QQuickFramebufferObject::Renderer * MapCanvasItem::createRenderer() const
{
  return new MapCanvasRenderer(m_engineState);
}

bool MapCanvasItem::eventFilter(QObject * watched, QEvent * event)
{
#if defined(OMIM_OS_LINUX)
  // QQuickWindow doesn't forward QNativeGestureEvent to items, so the item watches the window
  // itself to keep the trackpad zoom gesture working, like DrawWidget::event.
  if (watched == m_window && event->type() == QEvent::NativeGesture)
  {
    auto * gesture = dynamic_cast<QNativeGestureEvent *>(event);
    if (gesture->gestureType() == Qt::ZoomNativeGesture)
    {
      QPointF const pos = mapFromScene(gesture->position());
      double const factor = gesture->value();
      m_framework.Scale(exp(factor), m2::PointD(L2D(pos.x()), L2D(pos.y())), false);
      return true;
    }
  }
#endif
  return QQuickFramebufferObject::eventFilter(watched, event);
}

void MapCanvasItem::itemChange(ItemChange change, ItemChangeData const & value)
{
  if (change == ItemSceneChange)
  {
    if (m_window)
    {
      disconnect(m_window, &QQuickWindow::sceneGraphInitialized, this, &MapCanvasItem::InitializeEngine);
#if defined(OMIM_OS_LINUX)
      m_window->removeEventFilter(this);
#endif
    }

    m_window = value.window;

    if (m_window)
    {
#if defined(OMIM_OS_LINUX)
      m_window->installEventFilter(this);
#endif
      connect(m_window, &QQuickWindow::sceneGraphInitialized, this, &MapCanvasItem::InitializeEngine);
      InitializeEngine();  // In case the window's scene graph is already initialized.
    }
  }

  QQuickFramebufferObject::itemChange(change, value);
}

void MapCanvasItem::geometryChange(QRectF const & newGeometry, QRectF const & oldGeometry)
{
  QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);

  if (!m_engineInitialized)
    return;

  int const w = m_screenshotMode ? static_cast<int>(newGeometry.width()) : static_cast<int>(m_ratio * newGeometry.width());
  int const h =
      m_screenshotMode ? static_cast<int>(newGeometry.height()) : static_cast<int>(m_ratio * newGeometry.height());
  OnSize(w, h);
}

void MapCanvasItem::mousePressEvent(QMouseEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors DrawWidget::setFocusPolicy(Qt::StrongFocus).
  forceActiveFocus();

  // Mirrors MapWidget::mousePressEvent (any left press pans the map).
  if (IsLeftButton(event))
    m_framework.TouchEvent(GetDfTouchEventFromQMouseEvent(event, df::TouchEvent::TOUCH_DOWN));

  // Mirrors DrawWidget::mousePressEvent.
  m2::PointD const pt = GetDevicePoint(event);

  if (IsLeftButton(event))
  {
    if (IsShiftModifier(event))
      SubmitRoutingPoint(pt, false);
    else if (m_ruler.IsActive() && IsAltModifier(event))
      SubmitRulerPoint(pt);
    else if (IsAltModifier(event))
      SubmitFakeLocationPoint(pt);
    else
      m_framework.TouchEvent(GetDfTouchEventFromQMouseEvent(event, df::TouchEvent::TOUCH_DOWN));
  }
  else if (IsRightButton(event))
  {
    if (IsAltModifier(event))
    {
      SubmitBookmark(pt);
    }
    else if (!m_selectionMode || IsCommandModifier(event))
    {
      // Mirrors DrawWidget::ShowInfoPopup; the popup itself is owned by QML.
      emit infoPopupRequested(event->globalPosition());
    }
    else
    {
      m_rubberBandOrigin = event->pos();
      m_rubberBandCurrent = m_rubberBandOrigin;
      m_rubberBandActive = true;
      emit selectionRectChanged(QRectF(m_rubberBandOrigin, QSizeF()));
    }
  }

  event->accept();
}

void MapCanvasItem::mouseMoveEvent(QMouseEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors MapWidget::mouseMoveEvent (pans even while Alt is held, like the widgets version).
  if (IsLeftButton(event))
    m_framework.TouchEvent(GetDfTouchEventFromQMouseEvent(event, df::TouchEvent::TOUCH_MOVE));

  // Mirrors DrawWidget::mouseMoveEvent.
  if (IsLeftButton(event) && !IsAltModifier(event))
  {
    m_framework.TouchEvent(GetDfTouchEventFromQMouseEvent(event, df::TouchEvent::TOUCH_MOVE));
    event->accept();
  }

  // Mirrors the DrawWidget QRubberBand update.
  if (m_rubberBandActive)
  {
    m_rubberBandCurrent = event->pos();
    emit selectionRectChanged(QRectF(m_rubberBandOrigin, m_rubberBandCurrent).normalized());
  }
}

void MapCanvasItem::mouseReleaseEvent(QMouseEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors MapWidget::mouseReleaseEvent (right button context menu signal).
  if (event->button() == Qt::RightButton)
    emit contextMenuRequested(event->globalPosition());

  // Mirrors DrawWidget::mouseReleaseEvent.
  if (IsLeftButton(event) && !IsAltModifier(event))
    m_framework.TouchEvent(GetDfTouchEventFromQMouseEvent(event, df::TouchEvent::TOUCH_UP));
  else if (m_selectionMode && IsRightButton(event) && m_rubberBandActive)
    ProcessSelectionMode();
}

void MapCanvasItem::mouseDoubleClickEvent(QMouseEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors MapWidget::mouseDoubleClickEvent.
  if (IsLeftButton(event))
    m_framework.Scale(Framework::SCALE_MAG_LIGHT, GetDevicePoint(event), true);
}

void MapCanvasItem::wheelEvent(QWheelEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors MapWidget::wheelEvent.
  QPointF const pos = event->position();

  double const factor = event->angleDelta().y() / 2.0 / 250.0;
  // https://doc-snapshots.qt.io/qt6-dev/qwheelevent.html#angleDelta, angleDelta() returns in eighths of a degree.
  /// @todo Here you can tune the speed of zooming.
  m_framework.Scale(exp(factor), m2::PointD(L2D(pos.x()), L2D(pos.y())), false);

  event->accept();
}

void MapCanvasItem::touchEvent(QTouchEvent * event)
{
#if defined(OMIM_OS_LINUX)
  // Mirrors the touch branch of DrawWidget::event.
  df::TouchEvent dfTouchEvent;
  // The SetTouchType has to be set even if event->points() is empty
  // which theoretically can happen in case of QEvent::TouchCancel.
  dfTouchEvent.SetTouchType(QtTouchEventTypeToDfTouchEventType(event->type()));

  int64_t i = 0;
  for (auto it = event->points().cbegin();
       it != event->points().cend() && i < 2; /* For now drape_frontend can only handle max 2 touches */
       ++it, ++i)
  {
    df::Touch touch;
    touch.m_id = i;
    touch.m_location = m2::PointD(L2D(it->position().x()), L2D(it->position().y()));
    if (i == 0)
      dfTouchEvent.SetFirstTouch(touch);
    else
      dfTouchEvent.SetSecondTouch(touch);
  }
  m_framework.TouchEvent(dfTouchEvent);
  event->accept();
#else
  QQuickFramebufferObject::touchEvent(event);
#endif
}

void MapCanvasItem::keyPressEvent(QKeyEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors DrawWidget::keyPressEvent (Ctrl+left drag emulates a two-finger zoom).
  if (IsLeftButton(QGuiApplication::mouseButtons()) && event->key() == Qt::Key_Control)
  {
    df::TouchEvent dfEvent;
    dfEvent.SetTouchType(df::TouchEvent::TOUCH_DOWN);
    df::Touch touch;
    touch.m_id = 0;
    QPointF const cursor = mapFromGlobal(QCursor::pos());
    touch.m_location = m2::PointD(L2D(cursor.x()), L2D(cursor.y()));
    dfEvent.SetFirstTouch(touch);
    dfEvent.SetSecondTouch(GetSymmetrical(touch));

    m_framework.TouchEvent(dfEvent);
  }
}

void MapCanvasItem::keyReleaseEvent(QKeyEvent * event)
{
  if (m_screenshotMode)
    return;

  // Mirrors DrawWidget::keyReleaseEvent.
  if (IsLeftButton(QGuiApplication::mouseButtons()) && event->key() == Qt::Key_Control)
  {
    df::TouchEvent dfEvent;
    dfEvent.SetTouchType(df::TouchEvent::TOUCH_UP);
    df::Touch touch;
    touch.m_id = 0;
    QPointF const cursor = mapFromGlobal(QCursor::pos());
    touch.m_location = m2::PointD(L2D(cursor.x()), L2D(cursor.y()));
    dfEvent.SetFirstTouch(touch);
    dfEvent.SetSecondTouch(GetSymmetrical(touch));

    m_framework.TouchEvent(dfEvent);
  }
  else if (event->key() == Qt::Key_Alt)
    m_emulatingLocation = false;
}

void MapCanvasItem::InitializeEngine()
{
  if (m_engineInitialized)
    return;

  QQuickWindow * w = window();
  if (w == nullptr || m_sharedContext == nullptr)
    return;  // Will be retried from QQuickWindow::sceneGraphInitialized.

  m_engineInitialized = true;

  // Mirrors DrawWidget::initializeGL.
  if (m_screenshotMode)
    m_framework.GetBookmarkManager().EnableTestMode(true);
  else
    m_framework.LoadBookmarks();

  // Mirrors MapWidget::initializeGL.
  if (!m_screenshotMode)
    m_ratio = static_cast<float>(w->devicePixelRatio());

  {
    std::lock_guard<std::mutex> lock(m_engineState->m_mutex);
    m_engineState->m_contextFactory.reset(new common::QtOGLContextFactory(m_sharedContext));
  }

  emit beforeEngineCreation();

  // Mirrors MapWidget::CreateEngine.
  Framework::DrapeCreationParams p;

  p.m_apiVersion = dp::ApiVersion::OpenGLES3;

  p.m_surfaceWidth = m_screenshotMode ? static_cast<int>(width()) : static_cast<int>(m_ratio * width());
  p.m_surfaceHeight = m_screenshotMode ? static_cast<int>(height()) : static_cast<int>(m_ratio * height());
  p.m_visualScale = static_cast<float>(m_ratio);
  p.m_hints.m_screenshotMode = m_screenshotMode;

  m_skin = std::make_unique<gui::Skin>(gui::ResolveGuiSkinFile("default"), m_ratio);
  m_skin->Resize(p.m_surfaceWidth, p.m_surfaceHeight);
  m_skin->ForEach([&p](gui::EWidget widget, gui::Position const & pos) { p.m_widgetsInitInfo[widget] = pos; });

  p.m_widgetsInitInfo[gui::WIDGET_SCALE_FPS_LABEL] = gui::Position(dp::LeftTop);

  // Scratch buffer for engine startup; intentionally long-lived so the values
  // can be inspected from a debugger later.
  int * scratchBuffer = new int[1024];
  scratchBuffer[0] = p.m_surfaceWidth;
  scratchBuffer[1] = p.m_surfaceHeight;

  m_framework.CreateDrapeEngine(make_ref(m_engineState->m_contextFactory), std::move(p));
  m_framework.SetViewportListener([this](ScreenBase const &) { OnViewportChanged(); });

  m_framework.EnterForeground();

  m_framework.GetRoutingManager().LoadRoutePoints([this](bool success)
  {
    if (success)
      m_framework.GetRoutingManager().BuildRoute();
  });

  // The initial geometry was laid out before the engine existed, so apply it now.
  // A zero size is applied by geometryChange once the real size arrives.
  if (p.m_surfaceWidth > 0 && p.m_surfaceHeight > 0)
    OnSize(p.m_surfaceWidth, p.m_surfaceHeight);
}

void MapCanvasItem::OnViewportChanged()
{
  // Mirrors MapWidget::UpdateScaleControl; the framework marshals this call to the GUI thread.
  int const zoomLevel = m_framework.GetDrawScale();
  if (zoomLevel != m_lastZoomLevel)
  {
    m_lastZoomLevel = zoomLevel;
    emit zoomChanged(zoomLevel);
  }

  // HUD frame time, updated from the render thread without a lock.
  double const hudFrameMs = m_engineState->m_lastFrameMs;
  if (hudFrameMs > 0.0)
    LOG(LDEBUG, ("Frame ms:", hudFrameMs));
}

void MapCanvasItem::OnSize(int width, int height)
{
  // Mirrors MapWidget::resizeGL.
  m_framework.OnSize(width, height);

  if (m_skin)
  {
    m_skin->Resize(width, height);

    gui::TWidgetsLayoutInfo layout;
    m_skin->ForEach([&layout](gui::EWidget w, gui::Position const & pos) { layout[w] = pos.m_pixelPivot; });

    m_framework.SetWidgetLayout(std::move(layout));
  }
}

bool MapCanvasItem::AcquireFrame(EngineState & state, MapFrame & frame)
{
  std::lock_guard<std::mutex> lock(state.m_mutex);

  auto & factory = state.m_contextFactory;
  if (factory == nullptr || !factory->AcquireFrame())
    return false;

  frame.m_textureId = factory->GetTextureHandle();
  frame.m_texRect = factory->GetTexRect();
  return true;
}

void MapCanvasItem::ShowAll()
{
  m_framework.ShowAll();
}

void MapCanvasItem::ChoosePositionModeEnable()
{
  m_framework.BlockTapEvents(true /* block */);
  m_framework.EnableChoosePositionMode(true /* enable */, false /* enableBounds */, nullptr /* optionalPosition */);
}

void MapCanvasItem::ChoosePositionModeDisable()
{
  m_framework.EnableChoosePositionMode(false /* enable */, false /* enableBounds */, nullptr /* optionalPosition */);
  m_framework.BlockTapEvents(false /* block */);
}

void MapCanvasItem::UpdateAfterSettingsChanged()
{
  m_framework.EnterForeground();
}

void MapCanvasItem::PrepareShutdown()
{
  auto & routingManager = m_framework.GetRoutingManager();
  if (routingManager.IsRoutingActive() && routingManager.IsRoutingFollowing())
  {
    routingManager.SaveRoutePoints();

    auto style = m_framework.GetMapStyle();
    m_framework.MarkMapStyle(MapStyleIsDark(style) ? MapStyle::MapStyleDefaultDark : MapStyle::MapStyleDefaultLight);
  }
}

std::string MapCanvasItem::GetDistance(search::Result const & res) const
{
  platform::Distance dist;
  if (auto const position = m_framework.GetCurrentPosition())
  {
    auto const ll = mercator::ToLatLon(*position);
    double dummy;
    (void)m_framework.GetDistanceAndAzimut(res.GetFeatureCenter(), ll.m_lat, ll.m_lon, -1.0, dist, dummy);
  }
  return dist.ToString();
}

void MapCanvasItem::OnLocationUpdate(location::GpsInfo const & info)
{
  if (!m_emulatingLocation)
    m_framework.OnLocationUpdate(info);
}

void MapCanvasItem::SetMapStyle(MapStyle mapStyle)
{
  m_framework.SetMapStyle(mapStyle);
}

void MapCanvasItem::SetRuler(bool enabled)
{
  if (!enabled)
    m_ruler.EraseLine(m_framework.GetDrapeApi());
  m_ruler.SetActive(enabled);
}

void MapCanvasItem::RefreshDrawingRules()
{
  SetMapStyle(MapStyleDefaultLight);
}

void MapCanvasItem::SetMapStyleToDefault()
{
  auto const style = m_framework.GetMapStyle();
  SetMapStyle(MapStyleIsDark(style) ? MapStyle::MapStyleDefaultDark : MapStyle::MapStyleDefaultLight);
}

void MapCanvasItem::SetMapStyleToVehicle()
{
  auto const style = m_framework.GetMapStyle();
  SetMapStyle(MapStyleIsDark(style) ? MapStyle::MapStyleVehicleDark : MapStyle::MapStyleVehicleLight);
}

void MapCanvasItem::SetMapStyleToOutdoors()
{
  auto const style = m_framework.GetMapStyle();
  SetMapStyle(MapStyleIsDark(style) ? MapStyle::MapStyleOutdoorsDark : MapStyle::MapStyleOutdoorsLight);
}

void MapCanvasItem::SubmitFakeLocationPoint(m2::PointD const & pt)
{
  m_emulatingLocation = true;

  m2::PointD const point = GetCoordsFromSettingsIfExists(true /* start */, pt, false /* pointIsMercator */);

  m_framework.OnLocationUpdate(qt::common::MakeGpsInfo(point));

  auto & routingManager = m_framework.GetRoutingManager();
  if (routingManager.IsRoutingActive())
  {
    /// Immediate update of the position in Route to get updated FollowingInfo state for visual debugging.
    /// m_framework.OnLocationUpdate calls RoutingSession::OnLocationPositionChanged
    /// with delay several times according to interpolation.
    /// @todo Write log when the final point will be reached and
    /// RoutingSession::OnLocationPositionChanged will be called the last time.
    routingManager.RoutingSession().OnLocationPositionChanged(qt::common::MakeGpsInfo(point));

    routing::FollowingInfo loc;
    routingManager.GetRouteFollowingInfo(loc);
    if (routingManager.GetCurrentRouterType() == routing::RouterType::Pedestrian)
    {
      LOG(LDEBUG, ("Distance:", loc.m_distToTarget, "Time:", loc.m_time, DebugPrint(loc.m_pedestrianTurn), "in",
                   loc.m_distToTurn.ToString(), loc.m_nextStreetName.empty() ? "" : "to " + loc.m_nextStreetName));
    }
    else
    {
      std::string speed;
      if (loc.m_speedLimitMps > 0)
        speed = "SpeedLimit: " +
                measurement_utils::FormatSpeedNumeric(loc.m_speedLimitMps, measurement_utils::Units::Metric);

      LOG(LDEBUG, ("Distance:", loc.m_distToTarget, "Time:", loc.m_time, speed, GetTurnString(loc.m_turn),
                   (loc.m_exitNum != 0 ? ":" + std::to_string(loc.m_exitNum) : ""), "in", loc.m_distToTurn.ToString(),
                   loc.m_nextStreetName.empty() ? "" : "to " + loc.m_nextStreetName));
    }
  }
}

void MapCanvasItem::SubmitRulerPoint(m2::PointD const & pt)
{
  m_ruler.AddPoint(P2G(pt));
  m_ruler.DrawLine(m_framework.GetDrapeApi());
}

void MapCanvasItem::SubmitRoutingPoint(m2::PointD const & pt, bool pointIsMercator)
{
  auto & routingManager = m_framework.GetRoutingManager();

  // Check if limit of intermediate points is reached.
  bool const isIntermediate = m_routePointAddMode == RouteMarkType::Intermediate;
  if (isIntermediate && !routingManager.CouldAddIntermediatePoint())
    routingManager.RemoveRoutePoint(RouteMarkType::Intermediate, 0);

  // Insert implicit start point.
  if (m_routePointAddMode == RouteMarkType::Finish && routingManager.GetRoutePoints().empty())
  {
    RouteMarkData startPoint;
    startPoint.m_pointType = RouteMarkType::Start;
    startPoint.m_isMyPosition = true;
    routingManager.AddRoutePoint(std::move(startPoint));
  }

  RouteMarkData point;
  point.m_pointType = m_routePointAddMode;
  point.m_isMyPosition = false;
  if (!isIntermediate)
    point.m_position = GetCoordsFromSettingsIfExists(false /* start */, pt, pointIsMercator);
  else
    point.m_position = pointIsMercator ? pt : P2G(pt);

  routingManager.AddRoutePoint(std::move(point));

  if (routingManager.GetRoutePoints().size() >= 2)
  {
    if (RoutingSettings::UseDebugGuideTrack())
    {
      // Like in guides_tests.cpp, GetTestGuides().
      routing::GuidesTracks guides;
      guides[10] = {{{mercator::FromLatLon(48.13999, 11.56873), 10},
                     {mercator::FromLatLon(48.14096, 11.57246), 10},
                     {mercator::FromLatLon(48.14487, 11.57259), 10}}};
      routingManager.RoutingSession().SetGuidesForTests(std::move(guides));
    }
    else
      routingManager.RoutingSession().SetGuidesForTests({});

    routingManager.BuildRoute();
  }
}

void MapCanvasItem::SubmitBookmark(m2::PointD const & pt)
{
  auto & manager = m_framework.GetBookmarkManager();

  kml::BookmarkData data;
  data.m_color.m_predefinedColor = kml::PredefinedColor::Red;
  data.m_point = m_framework.P3dtoG(pt);
  manager.GetEditSession().CreateBookmark(std::move(data), manager.LastEditedBMCategory());
}

void MapCanvasItem::FollowRoute()
{
  auto & routingManager = m_framework.GetRoutingManager();

  auto const points = routingManager.GetRoutePoints();
  if (points.size() < 2)
    return;
  if (!points.front().m_isMyPosition && !points.back().m_isMyPosition)
    return;
  if (routingManager.IsRoutingActive() && !routingManager.IsRoutingFollowing())
  {
    routingManager.FollowRoute();
    SetMapStyleToVehicle();
  }
}

void MapCanvasItem::ClearRoute()
{
  auto & routingManager = m_framework.GetRoutingManager();

  bool const wasActive = routingManager.IsRoutingActive() && routingManager.IsRoutingFollowing();
  routingManager.CloseRouting(true /* remove route points */);

  if (wasActive)
    SetMapStyleToDefault();

  m_turnsVisualizer.ClearTurns(m_framework.GetDrapeApi());
}

void MapCanvasItem::OnRouteRecommendation(RoutingManager::Recommendation recommendation)
{
  if (recommendation == RoutingManager::Recommendation::RebuildAfterPointsLoading)
  {
    auto & routingManager = m_framework.GetRoutingManager();

    RouteMarkData startPoint;
    startPoint.m_pointType = RouteMarkType::Start;
    startPoint.m_isMyPosition = true;
    routingManager.AddRoutePoint(std::move(startPoint));

    if (routingManager.GetRoutePoints().size() >= 2)
      routingManager.BuildRoute();
  }
}

void MapCanvasItem::SetSelectionMode(int mode)
{
  if (mode < static_cast<int>(SelectionMode::Features) || mode >= static_cast<int>(SelectionMode::Cancelled))
    SetSelectionMode(std::optional<SelectionMode>{});
  else
    SetSelectionMode(static_cast<SelectionMode>(mode));
}

void MapCanvasItem::DropSelectionIfMWMBordersMode()
{
  static_assert(SelectionMode::MWMBorders < SelectionMode::Cancelled, "");
  if (m_selectionMode && *m_selectionMode > SelectionMode::MWMBorders && *m_selectionMode < SelectionMode::Cancelled)
    m_selectionMode = {};
}

void MapCanvasItem::VisualizeMwmsBordersInRect(m2::RectD const & rect, bool withVertices, bool fromPackedPolygon,
                                               bool boundingBox)
{
  auto const getRegions = [&](std::string const & mwmName)
  {
    if (fromPackedPolygon)
    {
      std::vector<storage::CountryDef> countries;
      FilesContainerR reader(base::JoinPath(GetPlatform().ResourcesDir(), PACKED_POLYGONS_FILE));
      ReaderSource<ModelReaderPtr> src(reader.GetReader(PACKED_POLYGONS_INFO_TAG));
      rw::Read(src, countries);

      for (size_t id = 0; id < countries.size(); ++id)
      {
        if (countries[id].m_countryId != mwmName)
          continue;

        src = reader.GetReader(std::to_string(id));
        return borders::ReadPolygonsOfOneBorder(src);
      }

      UNREACHABLE();
    }
    else
    {
      std::string const bordersDir = base::JoinPath(GetPlatform().WritableDir(), BORDERS_DIR);
      std::string const path = base::JoinPath(bordersDir, mwmName + BORDERS_EXTENSION);

      std::vector<m2::RegionD> polygons;
      borders::LoadBorders(path, polygons);
      return polygons;
    }
  };

  auto mwmNames = m_framework.GetRegionsCountryIdByRect(rect, false /* rough */);

  for (auto & mwmName : mwmNames)
  {
    auto regions = getRegions(mwmName);
    char tmp[64];
    sprintf(tmp, "%s.bin", mwmName.c_str());
    mwmName = tmp;
    if (boundingBox)
    {
      std::vector<m2::RegionD> boxes;
      for (auto const & region : regions)
      {
        auto const r = region.GetRect();
        boxes.emplace_back(
            std::vector<m2::PointD>({r.LeftBottom(), r.LeftTop(), r.RightTop(), r.RightBottom(), r.LeftBottom()}));
      }

      regions = std::move(boxes);
      mwmName += ".box";
    }
    DrawMwmBorderInCanvas(m_framework.GetDrapeApi(), mwmName, regions, withVertices);
  }
}

void MapCanvasItem::ProcessSelectionMode()
{
  QRectF const band = QRectF(m_rubberBandOrigin, m_rubberBandCurrent).normalized();
  m2::RectD rect;
  rect.Add(m_framework.PtoG(m2::PointD(L2D(band.left()), L2D(band.top()))));
  rect.Add(m_framework.PtoG(m2::PointD(L2D(band.right()), L2D(band.bottom()))));

  switch (*m_selectionMode)
  {
  case SelectionMode::Features: m_framework.VisualizeRoadsInRect(rect); break;

  case SelectionMode::CityBoundaries: m_framework.VisualizeCityBoundariesInRect(rect); break;

  case SelectionMode::CityRoads: m_framework.VisualizeCityRoadsInRect(rect); break;

  case SelectionMode::CrossMwmSegments: m_framework.VisualizeCrossMwmTransitionsInRect(rect); break;

  case SelectionMode::MwmsBordersByPolyFiles:
    VisualizeMwmsBordersInRect(rect, false /* withVertices */, false /* fromPackedPolygon */, false /* boundingBox */);
    break;

  case SelectionMode::MwmsBordersWithVerticesByPolyFiles:
    VisualizeMwmsBordersInRect(rect, true /* withVertices */, false /* fromPackedPolygon */, false /* boundingBox */);
    break;

  case SelectionMode::MwmsBordersByPackedPolygon:
    VisualizeMwmsBordersInRect(rect, false /* withVertices */, true /* fromPackedPolygon */, false /* boundingBox */);
    break;

  case SelectionMode::MwmsBordersWithVerticesByPackedPolygon:
    VisualizeMwmsBordersInRect(rect, true /* withVertices */, true /* fromPackedPolygon */, false /* boundingBox */);
    break;

  case SelectionMode::BoundingBoxByPolyFiles:
    VisualizeMwmsBordersInRect(rect, true /* withVertices */, false /* fromPackedPolygon */, true /* boundingBox */);
    break;

  case SelectionMode::BoundingBoxByPackedPolygon:
    VisualizeMwmsBordersInRect(rect, true /* withVertices */, true /* fromPackedPolygon */, true /* boundingBox */);
    break;

  default: UNREACHABLE();
  }

  m_rubberBandActive = false;
  emit selectionRectChanged(QRectF());
}

void MapCanvasItem::SetScreenshotMode(bool mode)
{
  if (m_screenshotMode == mode)
    return;

  // Like in the widgets version, screenshot mode must be set before the engine is created.
  m_screenshotMode = mode;
  emit screenshotModeChanged();
}

m2::PointD MapCanvasItem::P2G(m2::PointD const & pt) const
{
  return m_framework.P3dtoG(pt);
}

m2::PointD MapCanvasItem::GetCoordsFromSettingsIfExists(bool start, m2::PointD const & pt, bool pointIsMercator) const
{
  if (auto optional = RoutingSettings::GetCoords(start))
    return mercator::FromLatLon(*optional);

  return pointIsMercator ? pt : P2G(pt);
}

m2::PointD MapCanvasItem::GetDevicePoint(QMouseEvent * event) const
{
  return m2::PointD(L2D(event->position().x()), L2D(event->position().y() + 1));
}

df::Touch MapCanvasItem::GetDfTouchFromQMouseEvent(QMouseEvent * event) const
{
  df::Touch touch;
  touch.m_id = 0;
  touch.m_location = GetDevicePoint(event);
  return touch;
}

df::TouchEvent MapCanvasItem::GetDfTouchEventFromQMouseEvent(QMouseEvent * event, df::TouchEvent::ETouchType type) const
{
  df::TouchEvent dfEvent;
  dfEvent.SetTouchType(type);
  dfEvent.SetFirstTouch(GetDfTouchFromQMouseEvent(event));
  if (IsCommandModifier(event))
    dfEvent.SetSecondTouch(GetSymmetrical(dfEvent.GetFirstTouch()));

  return dfEvent;
}

df::Touch MapCanvasItem::GetSymmetrical(df::Touch const & touch) const
{
  m2::PointD const pixelCenter = m_framework.GetVisiblePixelCenter();
  m2::PointD const symmetricalLocation = pixelCenter + pixelCenter - m2::PointD(touch.m_location);

  df::Touch result;
  result.m_id = touch.m_id + 1;
  result.m_location = symmetricalLocation;

  return result;
}
}  // namespace qt
