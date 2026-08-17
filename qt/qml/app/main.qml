import QtQuick
import QtQuick.Window

import aw

// CoMaps desktop shell — ArcticWorks design language.
// MapCanvasItem is exposed from C++ as the "mapCanvas" context property.
ApplicationWindow {
    id: window

    property bool developerMode: false
    property string currentPage: "map"

    width: 1280
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    // Shown from C++ after the shared GL graphics device is attached.
    visible: false
    title: "CoMaps"
    color: AWTheme.data["surface.0"]

    Component.onCompleted: {
        AWTooltip.parent = window.contentItem
        AWToaster.parent = overlayLayer
        mapCanvas.parent = mapArea
        mapCanvas.anchors.fill = mapArea
    }

    Item {
        id: rootLayout
        anchors.fill: parent

        Row {
            anchors.fill: parent

            AWSidebar {
                id: sidebar
                height: parent.height
                collapsed: false

                AWNavSection {
                    title: qsTr("Navigate")
                    width: parent.width

                    AWNavItem {
                        width: parent.width
                        iconName: "map"
                        text: qsTr("Map")
                        active: window.currentPage === "map"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "map"
                    }
                }

                AWNavSection {
                    title: qsTr("Places")
                    width: parent.width

                    AWNavItem {
                        width: parent.width
                        iconName: "search"
                        text: qsTr("Search")
                        active: window.currentPage === "search"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "search"
                    }

                    AWNavItem {
                        width: parent.width
                        iconName: "download"
                        text: qsTr("Downloader")
                        active: window.currentPage === "downloader"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "downloader"
                    }

                    AWNavItem {
                        width: parent.width
                        iconName: "bookmark"
                        text: qsTr("Bookmarks")
                        active: window.currentPage === "bookmarks"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "bookmarks"
                    }
                }

                AWNavSection {
                    title: qsTr("Activity")
                    width: parent.width

                    AWNavItem {
                        width: parent.width
                        iconName: "route"
                        text: qsTr("Trips")
                        active: window.currentPage === "trips"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "trips"
                    }

                    AWNavItem {
                        width: parent.width
                        iconName: "stats"
                        text: qsTr("Stats")
                        active: window.currentPage === "stats"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "stats"
                    }
                }

                AWNavSection {
                    visible: window.developerMode
                    title: qsTr("Developer")
                    width: parent.width

                    AWNavItem {
                        width: parent.width
                        iconName: "database"
                        text: qsTr("Data Explorer")
                        active: window.currentPage === "dataexplorer"
                        collapsed: sidebar.collapsed
                        onClicked: window.currentPage = "dataexplorer"
                    }
                }

                Item {
                    height: 1
                    width: parent.width
                }

                AWNavItem {
                    width: parent.width
                    iconName: "settings"
                    text: qsTr("Settings")
                    active: window.currentPage === "settings"
                    collapsed: sidebar.collapsed
                    onClicked: window.currentPage = "settings"
                }
            }

            Item {
                id: mainArea
                width: parent.width - sidebar.width
                height: parent.height

                Item {
                    id: mapArea
                    anchors.fill: parent
                }

                // Selection (developer mode) rubber band overlay.
                Rectangle {
                    visible: selectionRect.visible
                    x: selectionRect.x
                    y: selectionRect.y
                    width: selectionRect.width
                    height: selectionRect.height
                    color: AWTheme.data["interactive.subtle"]
                    border.width: 1
                    border.color: AWTheme.data["interactive.default"]
                    radius: AWTheme.data["radius.sm"]
                }

                // Floating map controls.
                Column {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: AWTheme.data["space.4"]
                    spacing: AWTheme.data["space.2"]

                    AWIconButton {
                        iconName: "position"
                        tooltipText: qsTr("My position")
                        onClicked: mapCanvas.ChoosePositionModeEnable()
                    }

                    AWIconButton {
                        iconName: "layers"
                        tooltipText: qsTr("Map layers")
                        onClicked: layersMenu.openAt(layersButton)
                    }

                    AWIconButton {
                        id: layersButton
                        iconName: AWTheme.theme === "dark" ? "sun" : "moon"
                        tooltipText: AWTheme.theme === "dark" ? qsTr("Light theme") : qsTr("Dark theme")
                        onClicked: {
                            AWTheme.theme = AWTheme.theme === "dark" ? "light" : "dark"
                            mapCanvas.SetMapStyleToDefault()
                        }
                    }
                }

                // Status bar.
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: AWTheme.data["layout.header-height"] * 0.6
                    color: AWTheme.data["surface.panel"]
                    border.color: AWTheme.data["border.subtle"]
                    border.width: 1

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: AWTheme.data["space.3"]
                        spacing: AWTheme.data["space.3"]

                        Text {
                            text: qsTr("Zoom") + ": " + zoomLevel
                            color: AWTheme.data["text.secondary"]
                            font.pixelSize: AWTheme.data["typography.size.xs"]
                        }
                    }
                }
            }
        }
    }

    // Placeholder pages; replaced by feature screens in later phases.
    Loader {
        anchors.fill: parent
        active: window.currentPage !== "map"
        sourceComponent: Rectangle {
            anchors.fill: parent
            color: AWTheme.data["surface.0"]
            Text {
                anchors.centerIn: parent
                text: qsTr("Coming soon: ") + window.currentPage
                color: AWTheme.data["text.tertiary"]
                font.pixelSize: AWTheme.data["typography.size.lg"]
            }
        }
        z: -1
    }

    property int zoomLevel: 10
    property rect selectionRect

    Connections {
        target: mapCanvas
        function onZoomChanged(zoom) {
            window.zoomLevel = zoom
        }
        function onSelectionRectChanged(rect) {
            window.selectionRect = rect
        }
    }

    // Layers context menu.
    AWMenu {
        id: layersMenu

        AWMenuItem {
            text: qsTr("Default")
            onClicked: mapCanvas.SetMapStyleToDefault()
        }
        AWMenuItem {
            text: qsTr("Vehicle")
            onClicked: mapCanvas.SetMapStyleToVehicle()
        }
        AWMenuItem {
            text: qsTr("Outdoors")
            onClicked: mapCanvas.SetMapStyleToOutdoors()
        }
        AWMenuSeparator {
        }
        AWMenuItem {
            text: qsTr("Ruler")
            onClicked: mapCanvas.SetRuler(true)
        }
    }

    Item {
        id: overlayLayer
        anchors.fill: parent
    }
}
