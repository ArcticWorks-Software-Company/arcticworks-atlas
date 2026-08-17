import QtQuick
import QtQuick.Controls

// Switch: token-driven track and knob.
Switch {
    id: root

    hoverEnabled: true
    opacity: enabled ? 1.0 : AWTheme.data["opacity.disabled"]

    implicitWidth: AWTheme.data["control.height"] * 1.75
    implicitHeight: AWTheme.data["control.height"]

    indicator: Item {
        implicitWidth: AWTheme.data["control.height"] * 1.75
        implicitHeight: AWTheme.data["control.height"]

        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: root.checked ? AWTheme.data["interactive.primary"] : AWTheme.data["border.strong"]
            Behavior on color { ColorAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }

        Rectangle {
            width: parent.height - 2 * AWTheme.data["space.0-5"]
            height: width
            radius: width / 2
            x: root.checked ? parent.width - width - AWTheme.data["space.0-5"] : AWTheme.data["space.0-5"]
            anchors.verticalCenter: parent.verticalCenter
            color: AWTheme.data["surface.2"]
            Behavior on x { NumberAnimation { duration: AWTheme.data["motion.fast"]; easing.type: Easing.OutCubic } }
        }
    }

    contentItem: Text {
        leftPadding: root.indicator.width + AWTheme.data["space.2"]
        text: root.text
        color: root.enabled ? AWTheme.data["text.primary"] : AWTheme.data["text.disabled"]
        font.pixelSize: AWTheme.data["typography.size.sm"]
        verticalAlignment: Text.AlignVCenter
    }
}
