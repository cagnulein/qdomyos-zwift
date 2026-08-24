# Settings UI refactor status

This document tracks the staged refactor of the QZ settings UI. Persistent QML settings are compatibility-sensitive, especially on iOS, so every phase keeps the persistence ABI protected while the modern renderer is validated.

## Invariants

- Existing persistent setting keys must not be renamed or removed.
- Existing persistent setting QML types and default expressions must not change as part of the UI refactor.
- Existing persistent declarations must keep their relative order. New persistent properties are append-only.
- `src/settings-catalog.json` coverage must never regress. Any new persistent setting must be cataloged in the same change.
- The modern category hierarchy is derived from the existing legacy accordion/page nesting in `src/settings.qml`; it is not a separately maintained taxonomy.
- A PR validation run compares against the current `master` head and fails if master contains persistent settings that are absent from the PR catalog/layout.
- Generic modern controls must not expose internal serialized storage. Persistent string state that is write-only in the legacy settings UI is treated as internal, and serialized defaults containing both `|` and `;` are rejected if visible.
- UI behavior, visibility, enabled state, side effects, and virtual-setting mappings must be preserved unless a separate behavioral change is explicitly requested.
- The legacy controls remain compiled as a temporary fallback, but the legacy settings UX is hidden by default while runtime validation proceeds.

## Current baseline

The original Phase 1 audit found **998 unique persistent setting keys** across the settings QML files. The feature branch has since been synchronized with newer `master` additions and currently covers **1001/1001** persistent keys.

The original 30 catalog gaps were classified before being added. User-facing examples include ANT+ Garmin, OpenBikeControl/MyWhoosh Link controls, Peloton Bike OCR, Wattbike emulator, and Zwift Play button/gear mappings. Storage-only values such as Garmin token-expiry state and Bluetooth service-changed state are cataloged with `visible: false`.

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

- [x] Classify and add all originally uncataloged persistent keys.
- [x] Reach complete catalog coverage and keep it synchronized with current master.
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
- [x] Keep the complete legacy controls compiled as a fallback during final validation.

## Phase 4: behavior parity hardening

Status: **repository parity complete; runtime smoke testing pending**

- [x] Persistent ABI compatibility remains unchanged.
- [x] Catalog coverage remains complete against the synchronized master baseline.
- [x] Add explicit `restartRequired` metadata to every persistent catalog setting and every virtual setting.
- [x] Derive restart policy from the legacy handlers instead of assuming every generic edit needs a restart.
- [x] Preserve metric/imperial display and storage conversions for weight, bike weight, height, autolap distance, treadmill speeds, and Peloton treadmill speed thresholds.
- [x] Preserve race-pace total-time formatting/storage for 1 mile, 5 km, 10 km, half marathon, and marathon targets.
- [x] Preserve Domyos profile and Kingsmith encryption mutual exclusion.
- [x] Preserve Peloton auto-start mutual exclusion.
- [x] Preserve Zwift OCR mutual exclusion and Android notification side effect.
- [x] Preserve Zwift Play / Wattbike emulator conflict handling, including the existing confirmation dialog behavior.
- [x] Preserve calibration cache invalidation for watt offset, watt gain, and power sensor changes.
- [x] Preserve legacy literal selector models in the catalog where they can be extracted safely; `app_language` now remains a selector instead of becoming a free-form text field.
- [x] Add permanent tests for the above behavior layer and explicit restart metadata.

## Phase 5: legacy-derived information architecture and drift protection

Status: **implemented; runtime validation pending**

- [x] Derive root categories and subcategories from the actual nested legacy accordion structure in `src/settings.qml`.
- [x] Derive root and nested `NewPageElement` placement, including `Tiles Options` and secondary settings pages.
- [x] Store the generated hierarchy and setting/page/virtual-setting node mappings in the catalog so the runtime does not maintain a second manual taxonomy.
- [x] Navigate nested legacy categories in the modern UI instead of promoting every catalog `parent` to root.
- [x] Preserve legacy source order when mixing direct settings, subcategories, and pages.
- [x] Remove root icons after cross-platform runtime testing showed inconsistent glyph availability/alignment; root rows are now text-only with a consistent chevron.
- [x] Preserve UTF-8 titles without mojibake while still decoding QML string escapes.
- [x] Keep `Experimental Features` as a root category and keep MQTT, OSC, and Template Settings nested below it; permanent tests enforce this relationship.
- [x] Hide legacy write-only persistent string storage from generic modern TextFields.
- [x] Explicitly prevent serialized string defaults containing both `|` and `;` from being exposed as normal visible settings.
- [x] `ergDataPoints` and `treadmillDataPoints` are now internal and no longer appear as duplicate `Watt Offset` fields.
- [x] Fetch current `master` during Settings audit and fail if master has added persistent settings missing from the PR/catalog/layout.
- [x] Hide the legacy settings UX by default while keeping its controls compiled as a fallback during validation.
- [x] Keep hierarchy/layout generators and checks permanent and the workflow read-only.
- [x] Add a read-only semantic parity report with HTML/JSON/CSV output and per-category PNG contact sheets; high-confidence control mismatches fail CI.
- [x] Preserve modern-settings navigation history and the previous scroll offset when entering nested categories.
- [x] Restore the exact previous `contentY` when backing out of nested categories instead of jumping to the top.
- [x] Preserve and restore the modern-settings state when opening dedicated QML pages through `stackView.push(...)`; returning from pages such as Custom Gears reopens the modern drawer at the previous category/search/scroll position.
- [x] Keep Search Back local to the search results before consuming category-navigation history.
- [ ] Full multiplatform build green on the final cleaned-up head.
- [ ] Manual smoke test of root categories/subcategory navigation on iOS.
- [ ] Manual smoke test on at least one non-iOS platform.
- [ ] Reduce remaining semantic-parity cases that are still intentionally unclassified.
- [ ] Add real rendered QML screenshot coverage where it can run reliably in CI.

### Current stop condition

The modern renderer is the default, mirrors the legacy category/page hierarchy, restores navigation and scroll state, and the old settings experience is no longer shown by default. The remaining blocker for physically deleting the legacy renderer is runtime visual/interaction validation on iOS and at least one non-iOS platform, plus any additional dynamic visibility/enabled behavior discovered during those smoke tests. Semantic parity remains the deterministic CI gate while real rendered screenshot coverage is added incrementally.
