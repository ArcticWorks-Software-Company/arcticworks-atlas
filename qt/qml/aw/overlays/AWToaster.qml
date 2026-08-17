pragma Singleton
import QtQuick

// Toast notification manager. AWToaster.show({text, kind, timeout}).
Item {
    id: root

    readonly property int maxToasts: 4

    function show(options) {
        var text = options && options.text !== undefined ? options.text : ""
        var kind = options && options.kind !== undefined ? options.kind : "info"
        var timeout = options && options.timeout !== undefined ? options.timeout : 4000
        if (text === "")
            return

        var component = Qt.createComponent("AWToast.qml")
        if (component.status !== Component.Ready) {
            console.warn("AWToaster: could not create AWToast")
            return
        }

        var toast = component.createObject(stack, { text: text, kind: kind, timeout: timeout })
        if (toast) {
            while (stack.children.length > maxToasts)
                stack.children[stack.children.length - 1].destroy()
            toast.showToast()
        }
    }

    function hideAll() {
        for (var i = 0; i < stack.children.length; ++i)
            stack.children[i].destroy()
    }

    Column {
        id: stack
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: AWTheme.data["space.4"]
        spacing: AWTheme.data["space.2"]
    }
}
