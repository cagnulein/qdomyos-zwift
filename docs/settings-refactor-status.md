# Settings UI refactor status

This document tracks the staged refactor of the QZ settings UI. The persistent QML settings are compatibility-sensitive, especially on iOS, so the refactor is intentionally split into independently reviewable phases.

## Invariants

- Existing persistent setting keys must not be renamed or removed.
- Existing persistent setting QML types and default expressions must not change as part of the UI refactor.
- Existing persistent declarations must keep their relative order. New persistent properties are append-only.
- `src/settings-catalog.json` must continue to contain every unique persistent setting declared by the settings QML files.
- UI behavior, visibility, enabled state, side effects, and virtual-setting mappings must be preserved unless a separate behavioral change is explicitly requested.
- The legacy UI stays available until the new UI reaches complete coverage and behavioral parity.

## Phase 1: safety infrastructure

Status: **in progress**

Goal: make accidental persistence or catalog regressions fail quickly before any visual refactor starts.

- [x] Add a parser for persistent settings in all settings QML files.
- [x] Compare each pull request against its actual base commit.
- [x] Reject removal, reorder, rename, QML type changes, and default-expression changes for existing persistent properties.
- [x] Allow new persistent properties only by appending after existing declarations.
- [x] Validate persistent-setting coverage against `src/settings-catalog.json`.
- [x] Validate `settingCount` and catalog `qmlType` values.
- [x] Produce a machine-readable settings audit artifact, including references, direct writes, and simple `visible`/`enabled` references from the legacy QML.
- [x] Add unit tests for the audit parser and ABI comparison.
- [x] Add a lightweight GitHub Actions workflow dedicated to settings auditing.
- [ ] Phase 1 CI green on the pull request.

## Phase 2: shared settings behavior layer

Status: **not started**

Goal: centralize setting mutations and their side effects without changing the visible legacy UI.

Planned work:

- Define the behavior metadata/controller used by search and the future settings UI.
- Preserve per-setting restart requirements, toasts, validation, special setters, and multi-setting virtual controls.
- Add tests for representative simple and complex settings before migrating the whole set.
- Remove the current assumption in generic search editing that every change requires restart.

Exit criteria: legacy UI behavior remains unchanged and behavior tests are green.

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
