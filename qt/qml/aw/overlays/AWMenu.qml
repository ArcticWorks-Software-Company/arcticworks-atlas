import QtQuick
import QtQuick.Controls

// Styled dropdown/context menu: surface.2 panel, shadow.2, rounded.
Menu {
    id: root

    function openAt(targetItem) {
        if (!targetItem)
            return
        var pt = targetItem.mapToItem(root.parent, 0, targetItem.height)
        root.x = Math.max(0, pt.x)
        root.y = Math.max(0, pt.y)
        root.open()
    }

    padding: AWTheme.data["space.1"]

    background: Item {
        AWShadow {
            anchors.fill: parent
            spec: AWTheme.data["shadow.2"]
            radius: AWTheme.data["radius.md"]
        }

        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["radius.md"]
            color: AWTheme.data["surface.2"]
            border.width: 1
            border.color: AWTheme.data["border.strong"]
        }
    }
}
