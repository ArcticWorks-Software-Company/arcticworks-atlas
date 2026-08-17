# qt/bridge — QML map canvas

`MapCanvasItem` (`map_canvas_item.hpp/.cpp`) is a drop-in replacement for the widgets-based
`qt::DrawWidget`, built on `QQuickFramebufferObject`, so the future QML UI can embed the drape map.
It renders the map and forwards mouse/wheel/touch/key input to `Framework` exactly like
`DrawWidget`/`qt::common::MapWidget` do, and mirrors their signals for QML (see the mapping table below).

## Rendering / GL context strategy

- **Drape keeps its own contexts.** `qt::common::QtOGLContextFactory` creates drape's draw + upload
  `QOpenGLContext`s (offscreen surfaces), sharing with the `QQuickWindow`'s GL context
  (`window()->openglContext()`, passed at engine creation). The scene graph render thread's context
  also shares with the window's context, so the frame texture produced by drape is directly
  bindable from `MapCanvasRenderer::render()` — no pixel copy, same design as `MapWidget::paintGL`
  (Present/AcquireFrame double-buffering).
- **The renderer only blits.** `render()` (scene graph render thread) acquires the latest presented
  frame and draws it into the QFBO with the same fullscreen-quad shaders as `MapWidget::Build`.
  The shader sources are inlined (identical to `qt/qt_common/res/shaders/`) so the bridge has no
  dependency on qt_common's qrc.
- **Engine creation stays on the GUI thread.** `Framework::CreateDrapeEngine` must not run on the
  render thread, so (unlike the usual QFBO `synchronize()` pattern, which runs on the render thread
  in Qt 6) engine creation is triggered from `QQuickWindow::sceneGraphInitialized` and `itemChange`
  (both GUI thread). Drape owns its GL contexts; the window's context is only used as the share root.
- **Threading.** The item and the renderer share a heap-allocated `EngineState` (`shared_ptr`
  + mutex). Frame acquisition on the render thread is serialized against engine creation/teardown
  on the GUI thread, so a late `render()` can never touch freed item members.

## Integration

### 1. Build (qt/CMakeLists.txt)

```cmake
set(SRC
  ...
  bridge/map_canvas_item.cpp
  bridge/map_canvas_item.hpp
  ...
)
```

- `AUTOMOC` is already ON for the target, so the `Q_OBJECT` class is moc'ed automatically.
- Add `Quick` to the `qt_components` list in the root `CMakeLists.txt`
  (`find_package(Qt6 COMPONENTS ...)`), and link `Qt6::Quick` to the target containing the bridge.
- The bridge uses `generator/borders.hpp` (selection mode), `coding`, `storage`, `routing`, `map`,
  `platform`, `drape_frontend`, `qt_common` — the `desktop` target already links all of these.

### 2. Window setup (main.cpp)

```cpp
#include <QQuickWindow>
#include <QSGRendererInterface>

// Before the window is created:
qt::common::SetDefaultSurfaceFormat(QApplication::platformName());  // already called in qt/main.cpp
QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);  // QQuickFramebufferObject requires the GL backend

QQuickWindow window;
window.setPersistentOpenGLContext(true);   // keep the shared GL context for the window's lifetime
window.setPersistentSceneGraph(true);      // keep the render thread alive when the window is hidden
// ... set up QQmlEngine / content, then show().
```

Both persistent flags must be set **before** `show()`; `setGraphicsApi` is static and must be
called before any window is created. Without a persistent GL context the share root is destroyed
on hide/minimize and drape's contexts become invalid.

### 3. Item creation (main.cpp)

`MapCanvasItem` needs a `Framework&`, so QML can't construct it — create it in C++ and expose it:

```cpp
auto * map = new qt::MapCanvasItem(framework, window.contentItem());
engine.rootContext()->setContextProperty("mapCanvas", map);
// The QML engine does not own the item:
engine.setObjectOwnership(map, QQmlEngine::CppOwnership);
```

(QML registration via `qmlRegisterType` is not useful for a non-default-constructible type; use the
context property.)

### 4. QML

```qml
Item {
  id: mapHost
  anchors.fill: parent
  Component.onCompleted: {
    mapCanvas.parent = mapHost
    mapCanvas.forceActiveFocus()  // required for key handling (Ctrl+drag zoom emulation)
  }
  Connections {
    target: mapCanvas
    function onZoomChanged(zoom) { /* drive a QML zoom slider */ }
    function onPlacePageRequested() { /* open the QML place page */ }
    function onInfoPopupRequested(pos) { /* feature info popup at global pos */ }
    function onContextMenuRequested(pos) { /* context menu at global pos */ }
    function onSelectionRectChanged(rect) { /* draw developer-mode rubber band */ }
  }
}
```

## DrawWidget → MapCanvasItem mapping

| DrawWidget / MapWidget                          | MapCanvasItem                                        |
|-------------------------------------------------|------------------------------------------------------|
| `initializeGL` (context factory, engine, skin)  | `InitializeEngine()` on `sceneGraphInitialized`/`itemChange` (GUI thread) |
| `resizeGL`                                      | `geometryChange` → `OnSize` (DPI-multiplied surface) |
| `paintGL` (`AcquireFrame` + quad blit)          | `MapCanvasRenderer::render()`                        |
| 60 fps `QTimer` → `update()`                    | 60 fps `QTimer` → `QQuickItem::update()`             |
| `mousePress/Move/Release/DoubleClick`           | same QQuickItem overrides, same `TouchEvent` calls (including Ctrl = second symmetrical touch, Alt = ruler/fake location, Shift = routing point, right = bookmark/info popup/selection band) |
| `QRubberBand` (selection mode)                  | `selectionRectChanged(QRectF)` — band drawn in QML; `ProcessSelectionMode` logic unchanged |
| `wheelEvent`                                    | same (`angleDelta().y()/3/360` → `Scale(exp(factor))`) |
| `event()` touch branch (Linux)                  | `touchEvent()` override (Linux only, same 2-touch cap) |
| `event()` `QNativeGestureEvent` (Linux)         | window `eventFilter` (QQuickWindow doesn't forward native gestures to items) |
| `keyPress/ReleaseEvent` (Ctrl+drag pinch emu)   | same, cursor position mapped into item coords |
| `ShowInfoPopup` (QMenu)                         | `infoPopupRequested(QPointF)`                        |
| `OnContextMenuRequested`                        | `contextMenuRequested(QPointF)`                      |
| `ShowPlacePage` (QDialog)                       | `placePageRequested()`                               |
| `UpdateScaleControl` (ScaleSlider)              | `zoomChanged(int)`                                   |
| `BeforeEngineCreation`                          | `beforeEngineCreation()`                             |
| `Screenshoter`                                  | not ported; `screenshotMode` property only affects surface size / DPR handling |
| hotkeys / `ScaleSlider` / `BindHotkeys`         | not ported; use QML `Shortcut`/slider calling the `Q_INVOKABLE` methods |

## Risks / notes

- **OpenGL scene graph backend required**: `QQuickFramebufferObject` renders black under a
  non-OpenGL RHI backend (Windows default is D3D11). Force
  `QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL)` (or `QSG_RHI_BACKEND=opengl`).
- **Context loss**: if the scene graph context is still recreated (explicit `setGraphicsApi`
  after startup, driver resets), drape's shared textures die with it and the app must recreate the
  engine — not handled; persistent flags avoid the common causes.
- **One map item per Framework**, like the widgets UI (one DrawWidget per Framework).
- Touch delivery relies on `QQuickItem::setAcceptTouchEvents` (Qt 6.2+), set by the item.
  Native gesture forwarding is X11-specific, as in the widgets version.
- Key events require the item to have focus (`forceActiveFocus` from QML).
- Screenshot mode automation (`Screenshoter`) is intentionally left to a later step.
- Not built yet: verify against the Qt 6.4.2 headers before merging (see `qt/CMakeLists.txt` steps).
