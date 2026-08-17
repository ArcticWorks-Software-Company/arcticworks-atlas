pragma Singleton
import QtQuick

import "themes/generated/AWThemeData.qml" as Data

// ArcticWorks theme singleton. All aw/ components consume tokens through
// AWTheme.data — components must never read raw palette or hardcode values.
QtObject {
    property string theme: "dark"
    property string density: "compact"

    readonly property var themes: Data.themes
    readonly property var themeNames: Data.themeNames
    readonly property var densityTiers: Data.densityTiers

    // Resolved token map for the current theme + density. Bindings on
    // sub-properties re-evaluate when this object is swapped.
    readonly property var data: {
        const t = Data.themes[theme]
        if (!t) return Data.themes["dark"]["compact"]
        return t[density] || t["compact"]
    }

    // Token accessor for imperative code (not binding-safe).
    function t(path) {
        return data[path]
    }

    function setTheme(name) {
        if (themeNames.indexOf(name) >= 0)
            theme = name
    }

    function setDensity(name) {
        if (densityTiers.indexOf(name) >= 0)
            density = name
    }
}
