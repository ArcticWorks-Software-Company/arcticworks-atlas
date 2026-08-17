import QtQuick

// Section wrapper with optional title inside a sidebar.
Item {
    id: root

    property string title: ""
    default property alias content: column.data

    implicitHeight: (titleText.visible ? titleText.implicitHeight + AWTheme.data["nav.section-title.margin-top"]
                                       : 0) + column.implicitHeight

    Column {
        id: column
        width: parent.width

        Text {
            id: titleText
            visible: title !== ""
            text: root.title
            color: AWTheme.data["nav.section-title.text"]
            font.pixelSize: AWTheme.data["nav.section-title.font-size"]
            font.weight: AWTheme.data["nav.section-title.font-weight"]
            topPadding: AWTheme.data["nav.section-title.margin-top"]
            bottomPadding: AWTheme.data["space.1"]
            elide: Text.ElideRight
            width: parent.width
        }
    }
}
