# Pending Tasks

Leftovers from the ArcticWorks desktop UI work session (2026-08-18).

## Blocked / environment

- [ ] **Fix WSL→Windows TCP networking** (blocks visual verification of the QML UI).
      Symptoms: instant `Connection refused` from WSL to any Windows-host port
      (gateway `172.31.96.1` AND the `127.0.0.1` localhost relay); Windows→WSL and
      WSL→internet both work. Soft fixes already tried without success:
      `wsl --shutdown`, hns/vmcompute/LxssManager restart, vEthernet adapter
      bounce, Windows Firewall allow-rule for TCP 6000 ("VcXsrv X11 WSL").
      Likely fix: Windows reboot (or `hnsdiag delete networks`).
      After reboot: start VcXsrv via XLaunch as usual, then verify with
      `wsl -d Ubuntu -e bash /mnt/p/tools/x-net-test.sh` and launch the app with
      `wsl -d Ubuntu -e bash /mnt/p/tools/run-desktop.sh`.
- [ ] **Visually verify the QML app shell** once X works: sidebar, map rendering
      (expect llvmpipe software rendering), layers menu, theme toggle, toasts,
      status bar zoom readout. `xwininfo` is installed in WSL for window checks.
- [ ] WSL VM instability: the VM randomly shuts down mid-build (builds die,
      /tmp wiped). `P:\tools\desktop-build.sh` has a retry loop that resumes via
      ninja. Worth investigating (power settings / podman interactions) later.

## Bugs found in the new aw/ QML components (fix on main)

- [ ] `qt/qml/aw/overlays/AWToast.qml` — re-showing a toast while a previous one
      is fading out destroys the new toast (dismiss/destroy timers race).
- [ ] `qt/qml/aw/structure/AWTabs.qml` — delegate reads `modelData.text`, so
      plain string-list models render empty tabs.
- [ ] `qt/qml/aw/structure/AWNavItem.qml` + `AWListItem.qml` — redeclare
      `signal clicked()` which shadows AbstractButton's built-in signal; remove
      the redeclaration and verify clicks still work at runtime.
- [ ] `qt/qml/aw/primitives/AWSelect.qml` — popup background has no shadow
      (AWShadow imported but unused).
- [ ] Light theme overlay (`tools/design/vendor/themes/light/tokens.json`) needs
      visual + contrast validation against the design-system spec (>=4.5:1).
- [ ] Regenerate `qt/qml/qml_resources.qrc` whenever QML/JS files are added or
      removed (it is a static qrc; `qt_add_resources` globbing was broken on
      Qt 6.4.2).

## Desktop feature work (per the ArcticWorks UI plan)

Phase 1 — mobile parity:
- [ ] Map downloader manager (country tree, sizes, download/update/delete,
      progress, toasts)
- [ ] Bookmarks manager (categories, table view, KML/KMZ import/export)
- [ ] Search (category browse, history, suggestions, results)
- [ ] Rich place page (address, hours, phone/website, edit/route actions)
- [ ] Navigation panel (turn-by-turn, ETA/speed) + optional voice (Qt
      TextToSpeech / speech-dispatcher)
- [ ] Settings screen (units, language, style, cache, routing defaults)
- [ ] OSM editor login/upload ported to QML; system tray + notifications
- [ ] Delete the old Qt Widgets UI once parity is reached (currently reachable
      via `QCOMAPS_UI=widgets`; screenshot mode + BUILD_DESIGNER still use it)

Phase 2 — beyond parity:
- [ ] Tracks + GPX record/import/export with elevation profiles
- [ ] Trip planner / route library
- [ ] Stats dashboard (charts)
- [ ] Data explorer (property-grid OSM inspection, developer tool)
- [ ] Command palette (Ctrl+K) + keyboard shortcut system
- [ ] Split-pane dockable workspace with saved layouts

## Tooling / housekeeping

- [ ] Android Fdroid build re-verification after the CMake LTO/lld change:
      `gradlew assembleFdroidBeta -Parm64` in `android/` (needs Git Bash with
      `SKIP_GENERATE_SYMBOLS=1` etc. — see committed setup).
- [ ] `tools/python/mwm_downloader.py` points at dead maps.me servers; either
      fix the URLs to `https://mapgen-fi-1.comaps.app/maps/<series>/<version>/`
      or remove the script.
- [ ] Android Studio installed on C:\ (installer ignores /D=); optional: move
      to P: manually. SDK/NDK/gradle caches are already on P:.
- [ ] WSL checkout (`~/comaps`) is in sync with origin/main via `git am`
      patches; pull normally from now on (`git pull`).

## Code review bot (arcticworks-codepeer) iteration notes

- [ ] Aggregate inline comments into ONE review submission (currently 6-7 empty
      COMMENTED reviews per pass).
- [ ] Fix pass-to-pass nondeterminism (the try/catch finding vanished in pass 2).
- [ ] Suggestion blocks should not reintroduce the violation being reported
      (focus-ring suggestion hardcodes colors while another finding enforces
      tokens).
- [ ] Known blind spots: `+1` pixel input bias in
      `MapCanvasItem::GetDevicePoint`, unexplained magic constants in
      `wheelEvent` (both were missed twice).
