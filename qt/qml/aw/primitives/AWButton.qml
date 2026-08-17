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

    implicitHeight: AWTheme.data["button.height"]
    implicitWidth: Math.max(content.implicitWidth, implicitHeight)

    hoverEnabled: true
    opacity: enabled ? 1.0 : AWTheme.data["button.disabled-opacity"]

    contentItem: Item {
        id: content

        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: AWTheme.data["space.1"]

            AWIcon {
                visible: iconName !== "" && !root.loading
                name: iconName
                size: AWTheme.data["icon.size.default"]
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
                font.pixelSize: AWTheme.data["button.font-size"]
                font.weight: AWTheme.data["button.font-weight"]
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        function textColor() {
            if (!root.enabled)
                return AWTheme.data["text.disabled"]
            switch (root.variant) {
            case AWButton.Variant.Primary:
                return AWTheme.data["button.primary.text"]
            case AWButton.Variant.Danger:
                return AWTheme.data["button.danger.text"]
            default:
                return AWTheme.data["button.secondary.text"]
            }
        }
    }

    background: Item {
        // 2px offset focus ring, shown only on keyboard focus.
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: AWTheme.data["button.radius"] + 2
            color: "transparent"
            border.width: 2
            border.color: AWTheme.data["button.focus-ring"]
            visible: root.visualFocus
        }

        Rectangle {
            id: bg
            anchors.fill: parent
            radius: AWTheme.data["button.radius"]
            color: bgColor()
            border.width: root.variant === AWButton.Variant.Secondary ? 1 : 0
            border.color: borderColor()

            function bgColor() {
                if (!root.enabled)
                    return AWTheme.data["button.secondary.background"]
                switch (root.variant) {
                case AWButton.Variant.Primary:
                    if (root.down) return AWTheme.data["button.primary.active-background"]
                    if (root.hovered) return AWTheme.data["button.primary.hover-background"]
                    return AWTheme.data["button.primary.background"]
                case AWButton.Variant.Danger:
                    if (root.hovered || root.down) return AWTheme.data["button.danger.hover-background"]
                    return "transparent"
                default:
                    if (root.hovered || root.down) return AWTheme.data["button.secondary.hover-background"]
                    return AWTheme.data["button.secondary.background"]
                }
            }

            function borderColor() {
                if (root.variant === AWButton.Variant.Secondary && root.enabled)
                    return root.hovered ? AWTheme.data["button.secondary.border-hover"]
                                       : AWTheme.data["button.secondary.border"]
                return "transparent"
            }

            Behavior on color { ColorAnimation { duration: AWTheme.data["button.transition-duration"]; easing.type: Easing.OutCubic } }
            Behavior on border.color { ColorAnimation { duration: AWTheme.data["button.transition-duration"]; easing.type: Easing.OutCubic } }
        }
    }
}
