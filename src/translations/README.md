# QDomyos-Zwift Translations

This folder contains translation files for 30 languages.

## Supported Languages

| Language | File | Locale Code |
|----------|------|-------------|
| Italian | qdomyos-zwift_it.ts | it |
| German | qdomyos-zwift_de.ts | de |
| French | qdomyos-zwift_fr.ts | fr |
| Spanish | qdomyos-zwift_es.ts | es |
| Portuguese | qdomyos-zwift_pt.ts | pt |
| Portuguese (Brazil) | qdomyos-zwift_pt_BR.ts | pt_BR |
| Russian | qdomyos-zwift_ru.ts | ru |
| Chinese (Simplified) | qdomyos-zwift_zh_CN.ts | zh_CN |
| Chinese (Traditional) | qdomyos-zwift_zh_TW.ts | zh_TW |
| Japanese | qdomyos-zwift_ja.ts | ja |
| Korean | qdomyos-zwift_ko.ts | ko |
| Arabic | qdomyos-zwift_ar.ts | ar |
| Hindi | qdomyos-zwift_hi.ts | hi |
| Turkish | qdomyos-zwift_tr.ts | tr |
| Vietnamese | qdomyos-zwift_vi.ts | vi |
| Polish | qdomyos-zwift_pl.ts | pl |
| Ukrainian | qdomyos-zwift_uk.ts | uk |
| Dutch | qdomyos-zwift_nl.ts | nl |
| Thai | qdomyos-zwift_th.ts | th |
| Indonesian | qdomyos-zwift_id.ts | id |
| Romanian | qdomyos-zwift_ro.ts | ro |
| Czech | qdomyos-zwift_cs.ts | cs |
| Greek | qdomyos-zwift_el.ts | el |
| Swedish | qdomyos-zwift_sv.ts | sv |
| Hungarian | qdomyos-zwift_hu.ts | hu |
| Finnish | qdomyos-zwift_fi.ts | fi |
| Norwegian | qdomyos-zwift_no.ts | no |
| Danish | qdomyos-zwift_da.ts | da |
| Hebrew | qdomyos-zwift_he.ts | he |
| Catalan | qdomyos-zwift_ca.ts | ca |

## How to Contribute Translations

### 1. Choose Your Language

Find your language file from the list above. For example, for German translations, edit `qdomyos-zwift_de.ts`.

### 2. Edit the Translation File

Each `.ts` file is an XML file with this structure:

```xml
<message>
    <source>Speed</source>
    <translation type="unfinished"></translation>
</message>
```

Add your translation:

```xml
<message>
    <source>Speed</source>
    <translation>Geschwindigkeit</translation>
</message>
```

**Important:** Remove `type="unfinished"` when you add the translation!

### 3. Translation Rules

- **Keep placeholders:** `%1`, `%2`, etc. must appear in the same order
  ```xml
  <source>Speed: %1 km/h</source>
  <translation>Geschwindigkeit: %1 km/h</translation>
  ```

- **Keep HTML tags:** `<b>`, `</b>`, etc. must be preserved
  ```xml
  <source>&lt;b&gt;Speed&lt;/b&gt;</source>
  <translation>&lt;b&gt;Geschwindigkeit&lt;/b&gt;</translation>
  ```

- **Don't translate units:** km/h, bpm, etc. stay the same
  ```xml
  <source>Heart rate (bpm)</source>
  <translation>Herzfrequenz (bpm)</translation>
  ```

- **Keep punctuation consistent:**
  ```xml
  <source>Speed:</source>
  <translation>Geschwindigkeit:</translation>
  ```

### 4. Tools for Translation

#### Option 1: Text Editor (Simple)
Open the `.ts` file in any text editor and add translations manually.

#### Option 2: Qt Linguist (Recommended)
Qt Linguist provides a GUI for translations:
```bash
linguist src/translations/qdomyos-zwift_de.ts
```

Features:
- Shows context (where the string appears)
- Highlights missing placeholders
- Validation for HTML tags
- Search and filter

#### Option 3: Online Tools
You can use online translation platforms like:
- Weblate (free for open source)
- Crowdin (free for open source)
- POEditor (free tier available)

See setup guide: `docs/60_translation_platforms.md`

### 5. Submit Your Translation

1. Fork the repository
2. Edit your language file
3. Commit your changes:
   ```bash
   git add src/translations/qdomyos-zwift_de.ts
   git commit -m "i18n: Add German translations for main UI"
   ```
4. Create a Pull Request on GitHub

## Automatic Updates

The translation files are automatically updated by GitHub Actions when new strings are added to the source code. This ensures:

- All language files stay synchronized
- New strings appear as `<translation type="unfinished">`
- Contributors can easily see what needs translation

## Machine Draft Translation (IT/FR/DE/ES)

You can generate a first machine-translated draft (to be human-reviewed) from GitHub Actions:

1. Open Actions -> `Auto Translate Language`
2. Click `Run workflow`
3. Choose `target_language`: any available locale code (e.g. `it`, `fr`, `de`, `es`) or `all`
4. Set `max_entries` to `0` to process all unfinished strings

The workflow updates the selected `.ts` file(s) and commits the result to the current branch.

## Common Terms Glossary

| English | Example Translations |
|---------|---------------------|
| Speed | Velocità (IT), Geschwindigkeit (DE), Vitesse (FR), Velocidad (ES) |
| Cadence | Cadenza (IT), Trittfrequenz (DE), Cadence (FR), Cadencia (ES) |
| Resistance | Resistenza (IT), Widerstand (DE), Résistance (FR), Resistencia (ES) |
| Heart rate | Frequenza cardiaca (IT), Herzfrequenz (DE), Fréquence cardiaque (FR) |
| Treadmill | Tapis Roulant (IT), Laufband (DE), Tapis de course (FR), Cinta de correr (ES) |
| Bike | Bici (IT), Fahrrad (DE), Vélo (FR), Bicicleta (ES) |

## File Format

The `.ts` files use Qt's translation format (XML-based):
- Human-readable and editable
- Compatible with Qt Linguist
- Compiled to `.qm` (binary) during build
- Embedded in the application automatically

## Questions?

- Open an issue on GitHub
- Check the main project README
- See `TRADUZIONE-TEMPLATE-IT.md` for Italian translation guide

## Thank You!

Every translation helps make QDomyos-Zwift accessible to more people worldwide! 🌍
