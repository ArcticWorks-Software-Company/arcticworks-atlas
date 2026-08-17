import QtQuick
import QtQuick.Controls

// Radio button: circular, token-driven.
RadioButton {
    id: root

    property real circleSize: AWTheme.data["control.height"] * 0.55

    hoverEnabled: true
    opacity: enabled ? 1.0 : AWTheme.data["opacity.disabled"]

    indicator: Rectangle {
        implicitWidth: circleSize
        implicitHeight: circleSize
        radius: circleSize / 2
        anchors.verticalCenter: parent.verticalCenter
        color: root.hovered ? AWTheme.data["surface.hover"] : "transparent"
        border.width: 1
        border.color: root.checked ? AWTheme.data["interactive.primary"]
                                   : root.visualFocus ? AWTheme.data["focus.ring"]
                                   : root.hovered ? AWTheme.data["border.strong"]
                                   : AWTheme.data["border.strong"]

        Rectangle {
            anchors.centerIn: parent
            width: circleSize * 0.5
            height: circleSize * 0.5
            radius: circleSize / 2
            color: AWTheme.data["interactive.primary"]
            visible: root.checked
        }

        Behavior on border.color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
    }

    contentItem: Text {
        leftPadding: AWTheme.data["space.2"]
        rightPadding: AWTheme.data["space.2"]
        text: root.text
        color: root.enabled ? AWTheme.data["text.primary"] : AWTheme.data["text.disabled"]
        font.pixelSize: AWTheme.data["typography.size.sm"]
        verticalAlignment: Text.AlignVCenter
    }
}
