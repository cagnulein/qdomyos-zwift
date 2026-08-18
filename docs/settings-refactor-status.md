# Settings UI refactor status

This document tracks the staged refactor of the QZ settings UI. Persistent QML settings are compatibility-sensitive, especially on iOS, so every phase keeps the persistence ABI protected and the legacy renderer available until parity is proven.

## Invariants

- Existing persistent setting keys must not be renamed or removed.
- Existing persistent setting QML types and default expressions must not change as part of the UI refactor.
- Existing persistent declarations must keep their relative order. New persistent properties are append-only.
- `src/settings-catalog.json` coverage must never regress. Any new persistent setting must be cataloged in the same change.
- UI behavior, visibility, enabled state, side effects, and virtual-setting mappings must be preserved unless a separate behavioral change is explicitly requested.
- The legacy UI stays available until the new UI reaches complete behavioral parity and manual smoke testing is complete.

## Current baseline

The Phase 1 audit found **998 unique persistent setting keys** across the settings QML files. Phase 2 completed the catalog from the previous 968/998 baseline to **998/998**.

The 30 former gaps were classified before being added. User-facing examples include ANT+ Garmin, OpenBikeControl/MyWhoosh Link controls, Peloton Bike OCR, Wattbike emulator, and Zwift Play button/gear mappings. Storage-only values such as Garmin token-expiry state and Bluetooth service-changed state are cataloged with `visible: false`.

The catalog and persistent ABI are now continuously checked by the `Settings audit` workflow.

## Phase 1: safety infrastructure

Status: **complete**

Goal: make accidental persistence or catalog regressions fail quickly before any visual refactor starts.

- [x] Parse only persistent `Settings {}` blocks in all settings QML files.
- [x] Compare each pull request against its actual base commit.
- [x] Reject removal, reorder, rename, QML type changes, and default-expression changes for existing persistent properties.
- [x] Allow new persistent properties only by appending after existing declarations.
- [x] Validate catalog coverage, `settingCount`, and catalog `qmlType` values.
- [x] Produce a machine-readable audit artifact with references, direct writes, and simple `visible`/`enabled` references.
- [x] Add unit tests and a dedicated GitHub Actions workflow.
- [x] Phase 1 gate green.

## Phase 2: catalog completion and shared settings behavior layer

Status: **complete**

Goal: complete catalog coverage and introduce a renderer-independent mutation path without changing hand-written legacy controls.

- [x] Classify and add all 30 previously uncataloged persistent keys.
- [x] Reach 998/998 catalog coverage.
- [x] Add a shared `settingsBehavior` controller for catalog/search/new-UI mutations.
- [x] Route generic persistent writes through the shared controller.
- [x] Route virtual multi-setting selections through the shared controller.
- [x] Support explicit `restartRequired` metadata while defaulting to `true`, preserving the previous generic-search behavior.
- [x] Keep hand-written legacy controls and their special conversions/side effects untouched.
- [x] Add structural tests that reject recursive/dead controller wiring and verify legacy controls remain present.

Important boundary: Phase 2 intentionally does **not** claim that every hand-written legacy side effect has been converted into metadata. That is a Phase 4 parity gate before legacy removal.

## Phase 3: modern iOS-style settings preview

Status: **implemented, validation pending**

Goal: add the new hierarchical renderer over the same catalog and behavior layer while keeping the legacy renderer as the default.

- [x] Add an opt-in `New UI` entry point from the existing settings search bar.
- [x] Add a right-edge modern settings Drawer.
- [x] Build root categories dynamically from the catalog.
- [x] Add in-UI search across catalog-visible settings, virtual settings, and pages.
- [x] Add modern rows for booleans, free-form values, catalog choices, virtual choices, and page navigation.
- [x] Reuse `settingValue`, option helpers, virtual helpers, and the shared behavior controller rather than duplicating mutation semantics.
- [x] Keep the complete legacy renderer in place as the fallback/default UI.
- [ ] Full project CI/build green on the final cleaned-up head.
- [ ] Manual iOS visual/interaction smoke test.

## Phase 4: parity verification and legacy removal

Status: **blocked on runtime parity validation**

The following gates are required before the legacy renderer may be removed:

- [x] Persistent ABI compatibility gate exists.
- [x] Catalog coverage: 998/998.
- [x] Modern renderer is catalog-driven, so every catalog-visible setting/page/virtual setting has a representation path.
- [ ] Audit every legacy-only validation/conversion/side effect and encode it in the shared behavior layer where the modern renderer needs it.
- [ ] Audit legacy `visible`/`enabled` conditions against the modern renderer.
- [ ] Manual smoke test on iOS.
- [ ] Manual smoke test on at least one non-iOS platform.
- [ ] Only after all gates above: make the modern renderer the default and remove the legacy renderer.

### Current stop condition

There is no safe basis yet for deleting the legacy UI. Static coverage and persistence safety are in place, but runtime behavior parity for special hand-written controls and platform smoke testing cannot be proven by the repository-only CI. The PR therefore deliberately keeps the modern UI opt-in and retains the legacy implementation.
