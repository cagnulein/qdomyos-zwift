# Settings UI refactor status

This document tracks the staged refactor of the QZ settings UI. Persistent QML settings are compatibility-sensitive, especially on iOS, so every phase keeps the persistence ABI protected and the legacy renderer available until runtime smoke testing is complete.

## Invariants

- Existing persistent setting keys must not be renamed or removed.
- Existing persistent setting QML types and default expressions must not change as part of the UI refactor.
- Existing persistent declarations must keep their relative order. New persistent properties are append-only.
- `src/settings-catalog.json` coverage must never regress. Any new persistent setting must be cataloged in the same change.
- The modern category hierarchy is derived from the existing `AccordionElement` nesting in `src/settings.qml`; it is not a separately maintained taxonomy.
- A PR validation run compares against the current `master` head and fails if master contains persistent settings that are absent from the PR catalog/layout.
- Generic modern controls must not expose internal serialized storage. Persistent string state that is write-only in the legacy settings UI is treated as internal, and serialized defaults containing both `|` and `;` are rejected if visible.
- UI behavior, visibility, enabled state, side effects, and virtual-setting mappings must be preserved unless a separate behavioral change is explicitly requested.
- The legacy UI remains available as a fallback until runtime smoke testing is complete.

## Current baseline

The Phase 1 audit found **998 unique persistent setting keys** across the settings QML files. Phase 2 completed the catalog from the previous 968/998 baseline to **998/998**.

The 30 former gaps were classified before being added. User-facing examples include ANT+ Garmin, OpenBikeControl/MyWhoosh Link controls, Peloton Bike OCR, Wattbike emulator, and Zwift Play button/gear mappings. Storage-only values such as Garmin token-expiry state and Bluetooth service-changed state are cataloged with `visible: false`.

The catalog, persistent ABI, generated hierarchy, and latest-master setting coverage are continuously checked by the read-only `Settings audit` workflow.

## Phase 1: safety infrastructure

Status: **complete**

- [x] Parse only persistent `Settings {}` blocks in all settings QML files.
- [x] Compare each pull request against its actual base commit.
- [x] Reject removal, reorder, rename, QML type changes, and default-expression changes for existing persistent properties.
- [x] Allow new persistent properties only by appending after existing declarations.
- [x] Validate catalog coverage, `settingCount`, and catalog `qmlType` values.
- [x] Produce a machine-readable audit artifact with references, direct writes, and simple `visible`/`enabled` references.
- [x] Add unit tests and a dedicated GitHub Actions workflow.

## Phase 2: catalog completion and shared settings behavior layer

Status: **complete**

- [x] Classify and add all 30 previously uncataloged persistent keys.
- [x] Reach 998/998 catalog coverage.
- [x] Add a shared `settingsBehavior` controller for catalog/search/new-UI mutations.
- [x] Route generic persistent writes through the shared controller.
- [x] Route virtual multi-setting selections through the shared controller.
- [x] Keep hand-written legacy controls intact while the behavior layer is introduced.

## Phase 3: modern iOS-style settings UI

Status: **complete, modern renderer is now the default**

- [x] Add the modern hierarchical settings renderer.
- [x] Add in-UI search across catalog-visible settings, virtual settings, and pages.
- [x] Add rows for booleans, free-form values, catalog choices, virtual choices, and page navigation.
- [x] Reuse the same catalog helpers and shared mutation path.
- [x] Promote the modern renderer to open automatically when the settings page is entered.
- [x] Keep the complete legacy renderer available behind the modern Drawer as a fallback during final validation.

## Phase 4: behavior parity hardening

Status: **repository parity complete; runtime smoke testing pending**

- [x] Persistent ABI compatibility remains unchanged.
- [x] Catalog coverage remains 998/998.
- [x] Add explicit `restartRequired` metadata to every persistent catalog setting and every virtual setting.
- [x] Derive restart policy from the legacy handlers instead of assuming every generic edit needs a restart.
- [x] Preserve metric/imperial display and storage conversions for weight, bike weight, height, autolap distance, treadmill speeds, and Peloton treadmill speed thresholds.
- [x] Preserve race-pace total-time formatting/storage for 1 mile, 5 km, 10 km, half marathon, and marathon targets.
- [x] Preserve Domyos profile and Kingsmith encryption mutual exclusion.
- [x] Preserve Peloton auto-start mutual exclusion.
- [x] Preserve Zwift OCR mutual exclusion and Android notification side effect.
- [x] Preserve Zwift Play / Wattbike emulator conflict handling, including the existing confirmation dialog behavior.
- [x] Preserve calibration cache invalidation for watt offset, watt gain, and power sensor changes.
- [x] Add permanent tests for the above behavior layer and explicit restart metadata.

## Phase 5: legacy-derived information architecture and drift protection

Status: **implemented; runtime validation pending**

- [x] Derive root categories and subcategories from the actual nested `AccordionElement` structure in `src/settings.qml`.
- [x] Store the generated hierarchy and setting/page/virtual-setting node mappings in the catalog so the runtime does not maintain a second manual taxonomy.
- [x] Navigate nested legacy categories in the modern UI instead of promoting every catalog `parent` to root.
- [x] Add UTF-8/Unicode-only icons to root categories; no external graphical assets are required.
- [x] Hide legacy write-only persistent string storage from generic modern TextFields.
- [x] Explicitly prevent serialized string defaults containing both `|` and `;` from being exposed as normal visible settings.
- [x] `ergDataPoints` and `treadmillDataPoints` are now internal and no longer appear as duplicate `Watt Offset` fields.
- [x] Fetch current `master` during Settings audit and fail if master has added persistent settings missing from the PR/catalog/layout.
- [x] Keep the hierarchy generator/check permanent and the workflow read-only.
- [x] Unit tests and hierarchy/latest-master gate green after migration.
- [ ] Full multiplatform build green on the final cleaned-up head.
- [ ] Manual smoke test of root categories/subcategory navigation on iOS.
- [ ] Manual smoke test on at least one non-iOS platform.

### Current stop condition

The modern renderer is the default and now mirrors the legacy category hierarchy rather than the catalog's flat parent list. The remaining blocker for deleting the legacy renderer is runtime visual/interaction validation on iOS and at least one non-iOS platform, plus any additional dynamic visibility/enabled behavior discovered during that smoke test.
