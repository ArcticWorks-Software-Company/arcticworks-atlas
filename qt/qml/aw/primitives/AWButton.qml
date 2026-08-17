import QtQuick
import QtQuick.Controls

// ArcticWorks button. Three variants only: Primary, Secondary, Danger.
// Every visual value comes from the button.* tokens.
Button {
    id: root

    enum Variant { Primary, Secondary, Danger }

    property int variant: AWButton.Variant.Secondary
    property string iconName: ""
    property bool loading: false

    implicitHeight: 32
    implicitWidth: Math.max(content.implicitWidth, implicitHeight)

    hoverEnabled: true
    opacity: enabled ? 1.0 : 0.5

    contentItem: Item {
        id: content

        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: 4

            AWIcon {
                visible: iconName !== "" && !root.loading
                name: iconName
                size: 18
                tint: textColor()
                anchors.verticalCenter: parent.verticalCenter
            }

            AWSpinner {
                visible: root.loading
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                visible: text !== ""
                text: root.text
                color: textColor()
                font.pixelSize: 13
                font.weight: 500
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        function textColor() {
            if (!root.enabled)
                return "#A8A8B0"
            switch (root.variant) {
            case AWButton.Variant.Primary:
                return "#FFFFFF"
            case AWButton.Variant.Danger:
                return "#F04A4A"
            default:
                return "#F2F2F4"
            }
        }
    }

    background: Rectangle {
        id: bg
        anchors.fill: parent
        radius: 8
        color: bgColor()
        border.width: root.variant === AWButton.Variant.Secondary ? 1 : 0
        border.color: borderColor()

        function bgColor() {
            if (!root.enabled)
                return "#17171B"
            switch (root.variant) {
            case AWButton.Variant.Primary:
                if (root.down) return "#245BC2"
                if (root.hovered) return "#5C9CFF"
                return "#2B6FD9"
            case AWButton.Variant.Danger:
                if (root.hovered || root.down) return "rgba(240,74,74,0.10)"
                return "transparent"
            default:
                if (root.hovered || root.down) return "rgba(255,255,255,0.04)"
                return "#17171B"
            }
        }

        function borderColor() {
            if (root.variant === AWButton.Variant.Secondary && root.enabled)
                return root.hovered ? "#3A3A42" : "#2C2C33"
            return "transparent"
        }

        Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
        Behavior on border.color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
    }
}
