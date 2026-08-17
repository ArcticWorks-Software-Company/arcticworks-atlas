pragma Singleton
import QtQuick
import QtQuick.Controls

import "../overlays/AWShadow.qml" as ShadowImport

// Transient tooltip singleton. AWTooltip.show(text, targetItem) shows it above
// the target, AWTooltip.hide() dismisses it.
Popup {
    id: root

    readonly property real gap: AWTheme.data["space.1"]

    function show(text, targetItem) {
        if (!targetItem)
            return
        label.text = text
        var pt = targetItem.mapToItem(root.parent, targetItem.width / 2, 0)
        root.x = pt.x - root.width / 2
        if (pt.y - root.height - gap >= 0)
            root.y = pt.y - root.height - gap
        else
            root.y = pt.y + targetItem.height + gap
        root.open()
        hideTimer.restart()
    }

    function hide() {
        hideTimer.stop()
        root.close()
    }

    padding: AWTheme.data["space.2"]
    closePolicy: Popup.NoAutoClose
    z: AWTheme.data["zindex.popover"]

    background: Item {
        ShadowImport.AWShadow {
            anchors.fill: parent
            spec: AWTheme.data["shadow.2"]
            radius: AWTheme.data["radius.md"]
        }

        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["radius.md"]
            color: AWTheme.data["surface.3"]
            border.width: 1
            border.color: AWTheme.data["border.strong"]
        }
    }

    contentItem: Text {
        id: label
        color: AWTheme.data["text.primary"]
        font.pixelSize: AWTheme.data["typography.size.xs"]
    }

    Timer {
        id: hideTimer
        interval: 4000
        onTriggered: root.close()
    }
}
