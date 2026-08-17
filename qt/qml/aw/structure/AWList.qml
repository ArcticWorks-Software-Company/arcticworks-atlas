import QtQuick
import QtQuick.Controls

// ListView with token-driven rows, dividers and keyboard navigation.
ListView {
    id: root

    property alias itemModel: root.model
    property alias itemDelegate: root.delegate
    property alias itemCurrentIndex: root.currentIndex
    property alias itemWidth: root.width

    signal itemActivated(int index)

    clip: true
    spacing: 0
    currentIndex: -1
    highlightMoveDuration: 0

    Keys.onUpPressed: {
        if (currentIndex > 0)
            currentIndex -= 1
        else if (count > 0)
            currentIndex = 0
    }

    Keys.onDownPressed: {
        if (currentIndex < count - 1)
            currentIndex += 1
    }

    Keys.onPressed: (event) => {
        if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && currentIndex >= 0)
            root.itemActivated(currentIndex)
    }
}
