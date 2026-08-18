# Settings UI refactor status

This document tracks the staged refactor of the QZ settings UI. The persistent QML settings are compatibility-sensitive, especially on iOS, so the refactor is intentionally split into independently reviewable phases.

## Invariants

- Existing persistent setting keys must not be renamed or removed.
- Existing persistent setting QML types and default expressions must not change as part of the UI refactor.
- Existing persistent declarations must keep their relative order. New persistent properties are append-only.
- `src/settings-catalog.json` coverage must never regress. Any new persistent setting must be cataloged in the same change.
- Full catalog coverage is required before the new settings UI can replace the legacy UI.
- UI behavior, visibility, enabled state, side effects, and virtual-setting mappings must be preserved unless a separate behavioral change is explicitly requested.
- The legacy UI stays available until the new UI reaches complete coverage and behavioral parity.

## Current baseline

The Phase 1 audit found **998 unique persistent setting keys** across the settings QML files. The existing catalog contains **968 keys**, leaving **30 pre-existing persistent keys not cataloged**. This is now explicit technical debt rather than an invisible gap.

The missing baseline includes both user-facing legacy controls and internal storage. User-facing examples include ANT+ Garmin, MyWhoosh Link controls, Peloton Bike OCR, Wattbike emulator, and Zwift Play button/gear mappings. Internal examples include Garmin token-expiry values.

The Phase 1 CI permits exactly this pre-existing gap when comparing a pull request with its base commit, but fails if a change introduces any additional uncataloged persistent setting. The 30 baseline keys must be classified and cataloged (including `visible: false` for storage-only keys) before Phase 3 can reach 100% coverage.

## Phase 1: safety infrastructure

Status: **in progress**

Goal: make accidental persistence or catalog regressions fail quickly before any visual refactor starts.

- [x] Add a parser restricted to persistent `Settings {}` blocks in all settings QML files.
- [x] Compare each pull request against its actual base commit.
- [x] Reject removal, reorder, rename, QML type changes, and default-expression changes for existing persistent properties.
- [x] Allow new persistent properties only by appending after existing declarations.
- [x] Detect and report current catalog coverage and prevent it from regressing relative to the PR base.
- [x] Validate `settingCount` and catalog `qmlType` values.
- [x] Produce a machine-readable settings audit artifact, including catalog gaps, references, direct writes, and simple `visible`/`enabled` references from the legacy QML.
- [x] Add unit tests for the audit parser and ABI comparison.
- [x] Add a lightweight GitHub Actions workflow dedicated to settings auditing.
- [ ] Phase 1 CI green on the pull request.

## Phase 2: catalog completion and shared settings behavior layer

Status: **not started**

Goal: classify every persistent key, complete the catalog, and centralize setting mutations and their side effects without changing the visible legacy UI.

Planned work:

- Classify and add the 30 baseline persistent keys missing from `src/settings-catalog.json`; storage-only keys remain cataloged with `visible: false`.
- Reach 998/998 catalog coverage before starting the new renderer.
- Define the behavior metadata/controller used by search and the future settings UI.
- Preserve per-setting restart requirements, toasts, validation, special setters, and multi-setting virtual controls.
- Add tests for representative simple and complex settings before migrating the whole set.
- Remove the current assumption in generic search editing that every change requires restart.

Exit criteria: catalog coverage is 100%, legacy UI behavior remains unchanged, and behavior tests are green.

## Phase 3: iOS-style settings UI

Status: **not started**

Goal: add the new hierarchical UI as a renderer over the same persistent settings and shared behavior layer.

Planned work:

- Root category screen with iOS-style grouped navigation.
- Reusable rows for toggles, values, choices, text, navigation, and custom components.
- Search driven by the catalog.
- Lazy page loading.
- Developer switch to move between legacy and new settings UIs during validation.

Exit criteria: every legacy-visible setting/page/virtual setting is represented in the new UI.

## Phase 4: parity verification and legacy removal

Status: **not started**

Goal: prove parity before deleting the legacy renderer.

Required gates:

- Persistent ABI compatibility: 100%.
- Catalog coverage: 100%.
- Visible setting/page coverage: 100%.
- Behavior and side-effect coverage: 100% for migrated controls.
- Visibility/enabled-condition coverage: 100% for cataloged conditions.
- Manual smoke test on iOS plus at least one non-iOS platform.

Only after these gates are satisfied should the old settings renderer be removed.
