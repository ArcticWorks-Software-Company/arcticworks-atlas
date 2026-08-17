# ArcticWorks Theme

This document describes how the ArcticWorks design language (from the
`@arcticworks/design` npm package, v0.1.0) is mapped onto CoMaps, and lists the
files that carry the theme.

## Token source

`@arcticworks/design` ships W3C Design Tokens (JSON, `tokens/*.json`) and a
generated CSS theme. As of v0.1.0 **only the dark theme is published**
(`themes/dark/index.css`); the light theme is on the package roadmap. The light
theme used here was derived from the dark tokens following the package's
documented methodology: keep the same palette and token structure, remap the
semantic layer (surfaces, text, borders, shadows) — e.g. the README example
`text.primary: #111111` for a light theme.

### Palette (shared by both themes)

| Ramp | Values |
|---|---|
| neutral | 0 `#FFFFFF` · 50 `#F2F2F4` · 100 `#E8E8EB` · 200 `#D9D9DD` · 300 `#C4C4CA` · 400 `#A8A8B0` · 500 `#8B8B93` · 550 `#7A7A84` · 600 `#6F6F78` · 700 `#5A5A62` · 750 `#4E4E56` · 800 `#3A3A42` · 850 `#2C2C33` · 900 `#1D1D22` · 925 `#17171B` · 950 `#111114` · 975 `#0B0B0D` |
| blue (interactive only) | 300 `#8FBCFF` · 400 `#5C9CFF` · 500 `#3D8BFF` · 550 `#3D82F6` · 600 `#2B6FD9` · 700 `#245BC2` |
| status | success (green 400) `#3FB950` · warning (yellow 400) `#E3B341` · danger (red 400) `#F04A4A` |

### Semantic tokens — dark (official)

| Group | Token | Value |
|---|---|---|
| surface | 0 / 1 / 2 / 3 | neutral 975 / 950 / 925 / 900 (`#0B0B0D` / `#111114` / `#17171B` / `#1D1D22`) |
| surface | hover | `rgba(255,255,255,0.04)` |
| text | primary / secondary / tertiary / disabled | neutral 50 / 400 / 550 / 750 (`#F2F2F4` / `#A8A8B0` / `#7A7A84` / `#4E4E56`) |
| text | on-accent | `#FFFFFF` |
| border | subtle / default / strong | neutral 900 / 850 / 800 (`#1D1D22` / `#2C2C33` / `#3A3A42`) |
| interactive | default / hover / primary / active | blue 500 / 400 / 600 / 700 (`#3D8BFF` / `#5C9CFF` / `#2B6FD9` / `#245BC2`) |
| interactive | subtle | `rgba(61,139,255,0.14)` |
| status | success / warning / danger (+ subtle variants) | `#3FB950` / `#E3B341` / `#F04A4A` |
| scrim | | `rgba(0,0,0,0.55)` |

### Semantic tokens — light (derived, per package methodology)

| Group | Token | Value |
|---|---|---|
| surface | 0 / 1 / 2 / 3 | `#FFFFFF` / neutral 50 `#F2F2F4` / `#FFFFFF` / neutral 100 `#E8E8EB` |
| surface | hover | `rgba(0,0,0,0.04)` |
| text | primary / secondary / tertiary / disabled | neutral 925 / 600 / 550 / 400 (`#17171B` / `#6F6F78` / `#7A7A84` / `#A8A8B0`) |
| border | subtle / default / strong | neutral 100 / 200 / 300 (`#E8E8EB` / `#D9D9DD` / `#C4C4CA`) |
| interactive | default / hover / primary / active | blue 500 / 600 / 600 / 700 (`#3D8BFF` / `#2B6FD9` / `#2B6FD9` / `#245BC2`) |
| status | success / warning / danger | same as dark (they pass contrast on light) |
| scrim | | `rgba(0,0,0,0.55)` |

## Map styles

The map palette lives in `@variable` files per style and per mode; the drawing
rules themselves (in `include/*.mapcss`) only reference the variables, so a
re-theme is a pure palette swap.

| File | Content |
|---|---|
| `data/styles/default/{light,dark}/colors.mapcss` | Base cartography palette: neutral-ramp basemap, blue water (`#8FBCFF` light / `#245BC2` dark), muted desaturated vegetation, neutral roads with darker casings (light) / lighter major roads (dark), POI label colors harmonized to token hues, isolines |
| `data/styles/vehicle/{light,dark}/colors.mapcss` | Navigation basemap (same recipe, night bg `#0B0B0D`), route line contrast reserved for blue |
| `data/styles/outdoors/{light,dark}/colors.mapcss` | Overrides of the base palette |
| `data/styles/{default,vehicle}/{light,dark}/drape_colors.mapcss` | Runtime `colors{}` constants consumed by drape: `Route` `#3D8BFF` + outline `#245BC2`, pedestrian/bicycle/ruler blue variants, traffic = status colors, `Selection`/`GuiText`/`MyPositionAccuracy` = token colors, `Arrow3D` blue family, `SpeedCameraMarkBg` danger red, search/rating marks harmonized |
| `data/styles/default/{light,dark}/symbols/*.svg` | Compass (neutral ring, blue north needle), route point markers (start `#3D8BFF`, intermediate `#4E4E56`, finish red/white flag), route arrow, current-position, search pins |

Regenerate the binary drawing rules and symbol sprites after edits:

```bash
tools/unix/generate_drules.sh
tools/unix/generate_symbols.sh
```

The generated files (`data/drules_proto*.bin`, `data/colors.txt`,
`data/symbols/...`) are gitignored.

## Desktop (Qt) UI

`qt/qt_common/arcticworks_style.{hpp,cpp}` implements the tokens as a
`QPalette` (Fusion) + minimal QSS, with `arcticworks_style::Apply(app, dark)`.
It is applied at startup in `qt/main.cpp` and re-applied when the Night Mode
toggle in `qt/preferences_dialog.cpp` flips the map style. Icons (`qt/res/`)
are recolored to the token palette.

## Map data pipeline

The `.mwm` map files are generated with `tools/python/maps_generator` and the
`generator_tool`; the mapcss files above are consumed at *render time*
(`drules_proto*.bin`), so no map regeneration is needed for re-theming.

## TODO (not yet re-themed)

- Android resources (`android/app/src/main/res/values*/colors.xml`,
  `m3_colors.xml`, `themes.xml`, widget defaults).
- iOS (`iphone/Maps/Core/Theme/Colors.swift`, `Images.xcassets` color sets,
  speed-limit colors).
- App store icons / bundle icons on all platforms.
