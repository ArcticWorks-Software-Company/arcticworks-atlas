import QtQuick
import QtQuick.Controls
import "../overlays/AWShadow.qml" as _

// Select: shared field style + styled dropdown popup.
ComboBox {
    id: root

    implicitHeight: AWTheme.data["input.height"]
    implicitWidth: 200

    hoverEnabled: true
    font.pixelSize: AWTheme.data["input.font-size"]

    contentItem: Text {
        text: root.displayText
        color: root.enabled ? AWTheme.data["input.text"] : AWTheme.data["input.disabled-text"]
        font: root.font
        verticalAlignment: Text.AlignVCenter
        leftPadding: AWTheme.data["input.padding-x"]
        elide: Text.ElideRight
    }

    indicator: AWIcon {
        name: "chevron-down"
        size: AWTheme.data["icon.size.default"]
        tint: AWTheme.data["text.tertiary"]
        anchors.right: parent.right
        anchors.rightMargin: AWTheme.data["space.3"]
        anchors.verticalCenter: parent.verticalCenter
    }

    background: Item {
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: AWTheme.data["input.radius"] + 2
            color: "transparent"
            border.width: 2
            border.color: AWTheme.data["focus.ring"]
            visible: root.visualFocus
        }

        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["input.radius"]
            color: AWTheme.data["input.background"]
            border.width: 1
            border.color: root.visualFocus ? AWTheme.data["input.border-focus"]
                                           : root.hovered ? AWTheme.data["input.border-hover"]
                                           : AWTheme.data["input.border"]
            Behavior on border.color { ColorAnimation { duration: AWTheme.data["input.transition-duration"]; easing.type: Easing.OutCubic } }
        }
    }

    delegate: ItemDelegate {
        width: root.popup ? root.popup.width : 0
        height: AWTheme.data["control.height"]
        hoverEnabled: true

        contentItem: Text {
            text: root.textRole ? (Array.isArray(root.model) ? modelData[root.textRole] : model[root.textRole])
                                : modelData
            color: AWTheme.data["text.primary"]
            font: root.font
            verticalAlignment: Text.AlignVCenter
            leftPadding: AWTheme.data["input.padding-x"]
            elide: Text.ElideRight
        }

        background: Rectangle {
            color: parent.hovered ? AWTheme.data["surface.hover"] : "transparent"
            Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }
    }

    popup: Popup {
        y: root.height + AWTheme.data["space.0-5"]
        width: root.width
        implicitHeight: contentItem.implicitHeight
        padding: AWTheme.data["space.1"]
        z: AWTheme.data["zindex.dropdown"]

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            radius: AWTheme.data["radius.md"]
            color: AWTheme.data["surface.2"]
            border.width: 1
            border.color: AWTheme.data["border.strong"]
        }
    }
}
