import QtQuick
import QtQuick.Controls

import "../primitives/AWIconButton.qml" as _

// Modal dialog: scrim + centered panel on surface.3 with shadow.3.
Dialog {
    id: root

    property string subtitle: ""
    property bool dismissOnScrimClick: true

    modal: true
    padding: AWTheme.data["space.5"]
    leftPadding: AWTheme.data["space.5"]
    rightPadding: AWTheme.data["space.5"]

    Overlay.modal: Rectangle {
        color: AWTheme.data["scrim"]
        Behavior on opacity { NumberAnimation { duration: AWTheme.data["motion.slow"]; easing.type: Easing.OutCubic } }

        TapHandler {
            enabled: root.dismissOnScrimClick
            onTapped: root.close()
        }
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: AWTheme.data["motion.slow"]; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: AWTheme.data["motion.slow"]; easing.type: Easing.OutCubic }
        }
    }

    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: AWTheme.data["motion.normal"]; easing.type: Easing.OutCubic }
    }

    background: Item {
        AWShadow {
            anchors.fill: parent
            spec: AWTheme.data["shadow.3"]
            radius: AWTheme.data["radius.xl"]
        }

        Rectangle {
            anchors.fill: parent
            radius: AWTheme.data["radius.xl"]
            color: AWTheme.data["surface.dialog"]
            border.width: 1
            border.color: AWTheme.data["border.strong"]
        }
    }

    header: Column {
        width: parent.width
        spacing: AWTheme.data["space.1"]

        Text {
            text: root.title
            color: AWTheme.data["text.primary"]
            font.pixelSize: AWTheme.data["typography.size.lg"]
            font.weight: AWTheme.data["typography.weight.semibold"]
            width: parent.width - AWTheme.data["space.8"]
        }

        Text {
            visible: subtitle !== ""
            text: root.subtitle
            color: AWTheme.data["text.secondary"]
            font.pixelSize: AWTheme.data["typography.size.sm"]
        }
    }

    AWIconButton {
        iconName: "close"
        parent: root.background
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: AWTheme.data["space.2"]
        onClicked: root.close()
    }
}
