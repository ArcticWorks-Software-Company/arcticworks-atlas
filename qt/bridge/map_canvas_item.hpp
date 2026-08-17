#pragma once

#include "drape/pointers.hpp"
#include "drape_frontend/gui/skin.hpp"
#include "drape_frontend/user_event_stream.hpp"

#include "qt/qt_common/qtoglcontextfactory.hpp"
#include "qt/routing_turns_visualizer.hpp"
#include "qt/ruler.hpp"
#include "qt/selection.hpp"

#include "map/routing_manager.hpp"

#include "search/result.hpp"

#include "indexer/map_style.hpp"

#include "platform/location.hpp"

#include <QQuickFramebufferObject>
#include <QQuickWindow>

#include <QPointer>

#include <memory>
#include <mutex>
#include <optional>
#include <string>

class Framework;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QObject;
class QOpenGLContext;
class QTouchEvent;
class QTimer;
class QWheelEvent;
class ScreenBase;

namespace qt
{
class MapCanvasRenderer;

/// QML-embeddable replacement for qt::DrawWidget.
/// Renders the drape engine through QQuickFramebufferObject and forwards mouse/wheel/touch/key
/// input to Framework exactly like DrawWidget/MapWidget do. The item must be constructed from C++
/// (it requires a Framework&) and exposed to QML via QQmlContext::setContextProperty,
/// see qt/bridge/README.md.
class MapCanvasItem : public QQuickFramebufferObject
{
  Q_OBJECT

  Q_PROPERTY(bool screenshotMode READ GetScreenshotMode WRITE SetScreenshotMode NOTIFY screenshotModeChanged)

public:
  explicit MapCanvasItem(Framework & framework, QOpenGLContext * sharedContext, QQuickItem * parent = nullptr);
  ~MapCanvasItem() override;

  // QQuickFramebufferObject overrides:
  Renderer * createRenderer() const override;

  // API mirrored from DrawWidget.
  Q_INVOKABLE void ShowAll();
  Q_INVOKABLE void ChoosePositionModeEnable();
  Q_INVOKABLE void ChoosePositionModeDisable();
  Q_INVOKABLE void FollowRoute();
  Q_INVOKABLE void ClearRoute();
  Q_INVOKABLE void SetMapStyleToDefault();
  Q_INVOKABLE void SetMapStyleToVehicle();
  Q_INVOKABLE void SetMapStyleToOutdoors();
  Q_INVOKABLE void SetRuler(bool enabled);
  Q_INVOKABLE void UpdateAfterSettingsChanged();
  Q_INVOKABLE void PrepareShutdown();
  Q_INVOKABLE void SetSelectionMode(int mode);
  Q_INVOKABLE void DropSelectionIfMWMBordersMode();

  Framework & GetFramework() { return m_framework; }

  std::string GetDistance(search::Result const & res) const;

  void OnLocationUpdate(location::GpsInfo const & info);

  void SetMapStyle(MapStyle mapStyle);
  void RefreshDrawingRules();

  RouteMarkType GetRoutePointAddMode() const { return m_routePointAddMode; }
  void SetRoutePointAddMode(RouteMarkType mode) { m_routePointAddMode = mode; }

  void OnRouteRecommendation(RoutingManager::Recommendation recommendation);

  /// Pass empty \a mode to drop selection.
  void SetSelectionMode(std::optional<SelectionMode> mode) { m_selectionMode = mode; }

  bool GetScreenshotMode() const { return m_screenshotMode; }
  void SetScreenshotMode(bool mode);

signals:
  /// Mirrors MapWidget::BeforeEngineCreation: emitted on the GUI thread right before
  /// Framework::CreateDrapeEngine.
  void beforeEngineCreation();
  /// Mirrors MapWidget::UpdateScaleControl (widgets ScaleSlider); QML binds its slider to this.
  void zoomChanged(int zoomLevel);
  /// Mirrors MapWidget::OnContextMenuRequested (right button released, global coordinates).
  void contextMenuRequested(QPointF const & position);
  /// Mirrors DrawWidget::ShowInfoPopup (right button pressed over a feature, global coordinates);
  /// the popup itself is owned by QML now.
  void infoPopupRequested(QPointF const & position);
  /// Mirrors DrawWidget::ShowPlacePage (Framework place page onOpen listener).
  void placePageRequested();
  /// Selection (developer) mode rubber band, to be drawn by QML. Empty rect hides it.
  void selectionRectChanged(QRectF const & rect);
  void screenshotModeChanged();

protected:
  // QQuickItem overrides:
  bool eventFilter(QObject * watched, QEvent * event) override;
  void itemChange(ItemChange change, ItemChangeData const & value) override;
  void geometryChange(QRectF const & newGeometry, QRectF const & oldGeometry) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void mouseDoubleClickEvent(QMouseEvent * event) override;
  void wheelEvent(QWheelEvent * event) override;
  void touchEvent(QTouchEvent * event) override;
  void keyPressEvent(QKeyEvent * event) override;
  void keyReleaseEvent(QKeyEvent * event) override;

private:
  friend class MapCanvasRenderer;

  /// Shared between the item (GUI thread) and the renderer (scene graph render thread).
  /// The shared_ptr keeps the mutex (and the drape context factory) alive until the renderer
  /// is destroyed, so a late render() can never touch freed item members.
  struct EngineState
  {
    std::mutex m_mutex;
    drape_ptr<common::QtOGLContextFactory> m_contextFactory;
    // Last frame timestamp, written by the render thread, read by the GUI thread.
    double m_lastFrameMs = 0.0;
  };

  struct MapFrame
  {
    unsigned int m_textureId = 0;
    QRectF m_texRect;
  };

  int L2D(int px) const { return px * m_ratio; }
  m2::PointD GetDevicePoint(QMouseEvent * event) const;
  df::Touch GetDfTouchFromQMouseEvent(QMouseEvent * event) const;
  df::TouchEvent GetDfTouchEventFromQMouseEvent(QMouseEvent * event, df::TouchEvent::ETouchType type) const;
  df::Touch GetSymmetrical(df::Touch const & touch) const;

  /// Engine creation must stay on the GUI thread (drape spawns threads and the framework is not
  /// thread-safe), so unlike the QFBO synchronize() hook (render thread in Qt 6) this is driven
  /// from QQuickWindow::sceneGraphInitialized and itemChange.
  void InitializeEngine();
  void OnViewportChanged();
  void OnSize(int width, int height);

  /// Called on the scene graph render thread; serialized against engine teardown by EngineState::m_mutex.
  static bool AcquireFrame(EngineState & state, MapFrame & frame);

  void SubmitFakeLocationPoint(m2::PointD const & pt);
  void SubmitRulerPoint(m2::PointD const & pt);
  void SubmitRoutingPoint(m2::PointD const & pt, bool pointIsMercator);
  void SubmitBookmark(m2::PointD const & pt);
  void VisualizeMwmsBordersInRect(m2::RectD const & rect, bool withVertices, bool fromPackedPolygon, bool boundingBox);
  void ProcessSelectionMode();

  m2::PointD P2G(m2::PointD const & pt) const;
  m2::PointD GetCoordsFromSettingsIfExists(bool start, m2::PointD const & pt, bool pointIsMercator) const;

  Framework & m_framework;
  /// Root OpenGL context shared by both the Qt scene graph and the drape engine
  /// (the scene graph device is created from it, see main.cpp).
  QOpenGLContext * m_sharedContext;
  bool m_screenshotMode = false;
  bool m_engineInitialized = false;
  float m_ratio = 1.0f;
  int m_lastZoomLevel = -1;

  std::shared_ptr<EngineState> m_engineState;

  std::unique_ptr<gui::Skin> m_skin;
  std::unique_ptr<QTimer> m_updateTimer;

  // QQuickWindow doesn't forward QNativeGestureEvent to items, so the item filters the window
  // itself (Linux only) to keep trackpad zoom gestures working, like DrawWidget::event.
  QPointer<QQuickWindow> m_window;

  // Selection (developer) mode rubber band state; the band itself is drawn in QML.
  QPointF m_rubberBandOrigin;
  QPointF m_rubberBandCurrent;
  bool m_rubberBandActive = false;

  bool m_emulatingLocation = false;
  std::optional<SelectionMode> m_selectionMode;
  RouteMarkType m_routePointAddMode = RouteMarkType::Finish;

  Ruler m_ruler;
  RoutingTurnsVisualizer m_turnsVisualizer;
};
}  // namespace qt
