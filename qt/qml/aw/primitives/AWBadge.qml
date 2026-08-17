import QtQuick

// Small status pill. Kind: neutral | success | warning | danger.
Item {
    id: root

    property string text: ""
    property string kind: "neutral"

    implicitWidth: label.implicitWidth + 2 * AWTheme.data["space.2"]
    implicitHeight: label.implicitHeight + 2 * AWTheme.data["space.1"]

    Rectangle {
        anchors.fill: parent
        radius: AWTheme.data["radius.sm"]
        color: badgeBackground()
        border.width: 0
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: badgeText()
        font.pixelSize: AWTheme.data["typography.size.xs"]
        font.weight: AWTheme.data["typography.weight.medium"]
    }

    function badgeBackground() {
        switch (root.kind) {
        case "success": return AWTheme.data["status.success-subtle"]
        case "warning": return AWTheme.data["status.warning-subtle"]
        case "danger": return AWTheme.data["status.danger-subtle"]
        default: return AWTheme.data["interactive.subtle"]
        }
    }

    function badgeText() {
        switch (root.kind) {
        case "success": return AWTheme.data["status.success"]
        case "warning": return AWTheme.data["status.warning"]
        case "danger": return AWTheme.data["status.danger"]
        default: return AWTheme.data["interactive.default"]
        }
    }
}
