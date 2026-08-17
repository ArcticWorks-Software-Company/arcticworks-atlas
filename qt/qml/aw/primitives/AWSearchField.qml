import QtQuick
import QtQuick.Controls

// Search field: shared input style + leading search icon + trailing clear button.
TextField {
    id: root

    property bool error: false

    implicitHeight: AWTheme.data["input.height"]
    implicitWidth: 280

    selectByMouse: true
    color: enabled ? AWTheme.data["input.text"] : AWTheme.data["input.disabled-text"]
    placeholderTextColor: AWTheme.data["input.placeholder"]
    font.pixelSize: AWTheme.data["input.font-size"]
    verticalAlignment: TextInput.AlignVCenter
    leftPadding: AWTheme.data["input.height"]
    rightPadding: clearButton.width + AWTheme.data["space.1"]

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
            id: bg
            anchors.fill: parent
            radius: AWTheme.data["input.radius"]
            color: AWTheme.data["input.background"]
            border.width: 1
            border.color: root.error ? AWTheme.data["input.error-border"]
                                     : root.visualFocus ? AWTheme.data["input.border-focus"]
                                     : root.hovered ? AWTheme.data["input.border-hover"]
                                     : AWTheme.data["input.border"]
            Behavior on border.color { ColorAnimation { duration: AWTheme.data["input.transition-duration"]; easing.type: Easing.OutCubic } }
        }
    }

    AWIcon {
        name: "search"
        size: AWTheme.data["icon.size.default"]
        tint: AWTheme.data["text.tertiary"]
        anchors.left: parent.left
        anchors.leftMargin: AWTheme.data["space.3"]
        anchors.verticalCenter: parent.verticalCenter
    }

    AWIconButton {
        id: clearButton
        visible: root.text !== ""
        iconName: "close"
        width: AWTheme.data["control.height"]
        height: AWTheme.data["control.height"]
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        onClicked: root.clear()
    }
}
