import QtQuick
import QtQuick.Controls

// Single-line text input. One shared field style for all products (input.* tokens).
TextField {
    id: root

    property bool error: false
    property bool invalid: error

    implicitHeight: AWTheme.data["input.height"]
    implicitWidth: 240

    selectByMouse: true
    color: enabled ? AWTheme.data["input.text"] : AWTheme.data["input.disabled-text"]
    placeholderTextColor: AWTheme.data["input.placeholder"]
    font.pixelSize: AWTheme.data["input.font-size"]
    verticalAlignment: TextInput.AlignVCenter
    leftPadding: AWTheme.data["input.padding-x"]
    rightPadding: AWTheme.data["input.padding-x"]

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
}
