# ArcticWorks Design

[English](README.md) · [简体中文](README.zh-CN.md)

The design system for every ArcticWorks application. One design language, four products, forty — the same tokens, components and rules everywhere.

## Repo layout

```
arcticworks-design/
├── tokens/               # Source of truth (W3C Design Tokens format)
├── themes/
│   └── dark/
│       └── index.css     # Generated — do not edit by hand
├── scripts/
│   └── build.mjs         # Zero-dependency build (node or bun)
├── preview.html          # Component reference — every component, token-driven
├── preview.css           #   reference styles (tokens only, no raw values)
├── ui/                   # SvelteKit component library (@arcticworks/svelte)
│   ├── src/lib/          #   the package: components, icons, toast store
│   ├── src/routes/       #   interactive demo app
│   ├── AGENTS.md         #   component conventions contract
│   └── dist/             #   built package (bun run package)
└── README.md
```

## Packages

Two MIT-licensed packages are published to npm:

- `@arcticworks/design` — the design tokens: W3C Design Tokens (JSON) in `tokens/`, plus generated CSS custom properties in `themes/`.
- `@arcticworks/svelte` — the Svelte 5 component library in `ui/`.

Planned: `icons/` (SVG source), `docs/`.

## The Svelte component library — `ui/`

Svelte 5 (runes) + TypeScript + SvelteKit. Every component consumes the `--aw-*` tokens only, and each component's API and styles follow `ui/AGENTS.md`.

```sh
cd ui
bun install          # first time
bun run dev          # interactive demo at http://localhost:5173
bun run check        # svelte-check
bun run build        # static demo build → ui/build
bun run package      # library → ui/dist (svelte-package)
```

### Using it in an application

```ts
import '@arcticworks/design/themes/dark.css';
import { Button, Table, Dialog, toast, Toaster } from '@arcticworks/svelte';
```

Apply a density tier by setting `data-density="dense"` (or `comfortable`) on a container — rows, controls and paddings reflow through `var()` chains.

### Component list

Actions: `Button`, `IconButton`, `Segmented`. Forms: `Input`, `Textarea`, `Search`, `Checkbox`, `Radio`, `Switch`, `Select`, `Kbd`. Feedback: `Badge`, `Spinner`, `Progress`, `Skeleton`. Structure: `Card`, `List`, `ListItem`, `Breadcrumbs`, `Pagination`, `NavItem`, `NavSection`, `Sidebar`, `Tabs`, `Tree`. Data: `Table`, `PropertyGrid`, `SplitPane`, `LineChart`, `BarChart`. Overlays: `Dialog`, `Tooltip`, `Menu`, `MenuItem`, `MenuSeparator`, `Toaster` + `toast`, `CommandPalette`, `Calendar`. Composite: `FilePicker`. Icons: `Icon` + `icons` map.


## The three token layers

Components must never read raw values. Every component token is a `var()` chain through the semantic layer:

```
palette.neutral.950  →  surface.1  →  surface.sidebar  →  nav.item-*
```

1. **Palette** — raw color ramps (`palette.neutral.950`). Never referenced by components.
2. **Semantic** — roles: `surface.*`, `interactive.*`, `text.*`, `border.*`, `status.*`. What components and themes actually override.
3. **Component** — `button.*`, `input.*`, `table.*`, `nav.*`, `tabs.*`, `card.*` — every state tokenized.

Because references compile to live `var()` chains (e.g. `--aw-button-height: var(--aw-control-height)`), density tiers and future themes cascade through the whole system without touching a single component.

## Token groups

| Group | Prefix | Example |
|---|---|---|
| `palette` | `--aw-palette-` | `--aw-palette-blue-500` |
| `surface` `interactive` `text` `border` `status` `chart` `scrim` | `--aw-color-*` | `--aw-color-interactive-primary` |
| `typography` `lineheight` | `--aw-font-*` `--aw-line-height-*` | `--aw-font-size-sm` |
| `space` | `--aw-space-*` | `--aw-space-4` |
| `radius` | `--aw-radius-*` | `--aw-radius-md` |
| `shadow` | `--aw-shadow-*` | `--aw-shadow-2` |
| `motion` | `--aw-motion-*` | `--aw-motion-fast` |
| `control` | `--aw-control-*` | `--aw-control-height` |
| `icon` | `--aw-icon-*` | `--aw-icon-stroke` |
| `layout` | `--aw-layout-*` | `--aw-layout-sidebar-width` |
| `button` `input` `card` `table` `nav` `tabs` | `--aw-<component>-*` | `--aw-table-row-selected-background` |
| `zindex` | `--aw-z-*` | `--aw-z-dialog` |
| `breakpoint` | `--aw-breakpoint-*` | `--aw-breakpoint-xl` |
| `opacity` `focus` | `--aw-*` | `--aw-opacity-disabled` |

## Density

Three tiers: `dense` (PLC engineering, DataExplorer, SecureNet), `compact` (default), `comfortable` (Continuity, dashboards).

```html
<html data-density="dense">
```

Density overrides four roots — `control.height`, `control.padding-x`, `card.padding`, `layout.page-gutter` — and everything else (buttons, inputs, table rows, nav items) follows through `var()` chains. Applied per-container via `data-density` if an app needs mixed densities.

## Motion

- `fast` 120ms — hover, focus, micro-interactions
- `normal` 180ms — panels, toasts
- `slow` 250ms — dialogs, large overlays (never more)
- `easing-standard` / `easing-emphasized`

`prefers-reduced-motion` collapses all durations to 0ms in the generated CSS.

## Icons

24px viewBox grid, outline only, `stroke: 1.75px`, rounded joins and caps (`corner-radius: 2px`). Sizes: toolbar 16, default 18, navigation 20. No filled icons by default; no per-icon stroke variation.

## Tables

Enterprise software lives inside tables. Every state is tokenized:

`table.header.*`, `table.row.*` (height, separator, hover, selected, expanded, striped, focus), `table.cell.*` (text, secondary text, tabular numerals), `table.sort.*`, `table.resize.*`, `table.pinned.*`.

## Accessibility

- Text contrast ≥ 4.5:1, UI components ≥ 3:1 — all shipped token combinations verified.
- Focus ring: `--aw-focus-ring` (2px outline + 2px offset) on every interactive element.
- Hit targets: controls ≥ 28px at all densities (WCAG 2.5.8).
- `color-scheme: dark` so native controls render dark.
- Reduced motion handled in the generated CSS.

## Build

```sh
node scripts/build.mjs        # or: bun scripts/build.mjs
```

New themes are overlay files: `themes/light/tokens.json` re-maps semantic groups (e.g. `text.primary: #111111`) and `node scripts/build.mjs light` emits `themes/light/index.css`. The palette and every component token stay untouched.

## Extending

1. Add tokens to the right group file, referencing semantic tokens via `{group.name}`.
2. Regenerate. Unknown top-level groups map to their own CSS prefix.
3. Never introduce a new color, radius, shadow, duration or height in a component file. If it doesn't exist yet, it doesn't exist yet.

## Roadmap

- `ui/` — done: SvelteKit component library (`@arcticworks/svelte`), the only place components are built.
- `icons/` — SVG source set matching the icon specification (the runtime map lives in `ui/src/lib/icons.ts`).
- `themes/light` — same structure, semantic overrides only (`node scripts/build.mjs light` + a `themes/light/tokens.json` overlay).
- `docs/` — usage guidance per component.
