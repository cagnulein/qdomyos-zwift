# Settings Catalog

`src/settings-catalog.json` is a generated catalog of every persistent setting declared in the settings QML files:

- `src/settings.qml`
- `src/settings-tiles.qml`
- `src/settings-tts.qml`
- `src/settings-shortcuts.qml`
- `src/settings-treadmill-inclination-override.qml`

The file is JSON so it can be consumed by QML (`JSON.parse`), native Android, native iOS, documentation generators, and tests without a custom parser.

## Shape

Top-level fields:

- `schemaVersion`: catalog schema version.
- `settingCount`: number of unique setting keys.
- `pages`: visible settings sub-pages opened by `NewPageElement`.
- `virtualSettings`: non-persistent UI settings that map one visible control to one or more persistent settings.
- `settings`: one entry per unique setting key.

Each setting contains:

- `key`: persistent setting key.
- `name`: UI label when one can be found, otherwise a title-cased key.
- `description`: nearby descriptive label from QML, or `null`.
- `parent`: nearest QML group/accordion title, or an inferred group.
- `type` / `qmlType`: portable JSON type and original QML property type.
- `control`: inferred UI control (`switch`, `text`, `select`, or `button`).
- `visible`: whether the setting has a real UI control in the current settings QML.
- `defaultValue` / `defaultExpression`: parsed literal default when possible and the original QML expression.
- `options`: fixed ComboBox values or the model expression when available.
- `virtualParent`: present when this persistent setting is controlled by a virtual setting instead of directly by its own UI control.
- `defaultConflicts`: present only when duplicate declarations use different defaults.

Groups are intentionally not stored as a separate top-level list. Consumers can reconstruct groups by collecting settings with the same `parent` value. This keeps the catalog small and avoids maintaining two copies of the same hierarchy.

`virtualSettings` handles UI controls that do not correspond to a single persistent setting. For example, a "Specific Model" ComboBox can expose many model names while actually resetting a set of boolean settings and enabling only the boolean for the selected model. In that case the ComboBox is represented once in `virtualSettings`, and the underlying boolean settings use `control: "virtualOption"` plus `virtualParent`.

Each virtual setting contains:

- `key`: stable, domain-scoped virtual key, not persisted directly.
- `name`, `description`, `parent`, `type`, `control`, `defaultValue`: same meaning as persistent settings.
- `options`: ordered options. An option with `sets` enables that persistent setting; an option without `sets` means no backing setting is enabled, such as "Other".

`pages` handles visible navigation entries, such as "Wahoo Options", that do not map directly to a persistent setting but should still be searchable. Each page contains `key`, `name`, `description`, `parent`, `type: "page"`, `control: "page"`, `target`, and `visible`.

## Updating

When adding, removing, or renaming a setting in any settings QML file, regenerate the catalog:

```bash
python tools/generate_settings_catalog.py
```

Then validate that the catalog still exactly covers the QML declarations:

```bash
python tools/generate_settings_catalog.py --check
```

The check fails if any QML setting is missing from the catalog, if the catalog contains an undeclared setting, if a setting is duplicated, or if declaration metadata no longer matches the QML sources.
