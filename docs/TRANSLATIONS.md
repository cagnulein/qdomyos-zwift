# Translation System Documentation

This document explains how the translation system works in QDomyos-Zwift and how to work with translations.

## Overview

QDomyos-Zwift uses Qt's built-in translation system:
- **Source language**: English (en)
- **Translation files**: `.ts` files (XML format, human-readable)
- **Compiled files**: `.qm` files (binary format, loaded at runtime)
- **Supported languages**: Italian (it_IT) - more can be added

## File Structure

```
src/
├── qdomyos-zwift.ts         # Translation source file (Italian)
├── qdomyos-zwift_it.qm      # Compiled translation (auto-generated)
├── translations.qrc         # Qt resource file for translations
└── main.cpp                 # Loads translations at startup
```

## How Translations Work

### 1. Extraction (lupdate)
The `lupdate` tool scans source code for translatable strings:
- C++: Strings wrapped in `tr()` or `QCoreApplication::translate()`
- QML: Strings wrapped in `qsTr()` or `qsTranslate()`

Example:
```cpp
// C++
QString text = tr("Speed");

// QML
Text { text: qsTr("Speed") }
```

### 2. Translation (.ts file)
Translators edit the `.ts` file to add translations:

```xml
<message>
    <source>Speed</source>
    <translation>Velocità</translation>
</message>
```

### 3. Compilation (lrelease)
The `lrelease` tool compiles `.ts` → `.qm` files:
```bash
lrelease src/qdomyos-zwift.ts
```

### 4. Loading (QTranslator)
At application startup, `main.cpp` loads the appropriate `.qm` file based on system locale.

## Automated GitHub Actions Workflow

The translation workflow runs:
- **Trigger**: Every Sunday at midnight (UTC) or manually
- **Action**: Extracts new strings and creates a PR

### What it does:
1. Runs `lupdate` to extract translatable strings from source code
2. Updates `src/qdomyos-zwift.ts` with new/modified strings
3. Creates a Pull Request with the changes
4. Labels the PR with "translations" and "automation"

### After PR is created:
1. Review the new strings in the `.ts` file
2. Add Italian translations for strings marked as `<translation type="unfinished">`
3. Merge the PR when translations are complete

## Manual Translation Workflow

### 1. Extract new strings
```bash
# From project root
lupdate src/qdomyos-zwift.pri -ts src/qdomyos-zwift.ts
```

### 2. Translate strings
Open `src/qdomyos-zwift.ts` in:
- **Qt Linguist** (GUI tool - recommended)
- Any text editor (for simple changes)

Find entries like:
```xml
<message>
    <source>New feature text</source>
    <translation type="unfinished"></translation>
</message>
```

Add the translation:
```xml
<message>
    <source>New feature text</source>
    <translation>Testo nuova funzionalità</translation>
</message>
```

### 3. Compile translations
```bash
# Manual compilation
lrelease src/qdomyos-zwift.ts -qm src/qdomyos-zwift_it.qm

# Or use qmake/make (recommended - automatic during build)
qmake
make
```

### 4. Test translations
Build and run the application. The translation will be loaded automatically based on your system locale.

To test with a specific locale:
```bash
# Linux/macOS
LC_ALL=it_IT.UTF-8 ./qdomyos-zwift

# Windows (PowerShell)
$env:LANG="it_IT"
.\qdomyos-zwift.exe
```

## Platform-Specific Behavior

### iOS
- Translations are embedded in the app bundle
- System locale is detected automatically
- Falls back to English if translation not available

### Android
- Translations are embedded in the APK
- System locale is detected automatically
- Falls back to English if translation not available

### Windows
- Translations are embedded in the executable
- System locale is detected automatically
- Falls back to English if translation not available

### Linux/macOS
- Translations are embedded in the executable
- System locale is detected automatically
- Falls back to English if translation not available

## Adding New Languages

To add support for a new language (e.g., German):

### 1. Update `.pri` file
Edit `src/qdomyos-zwift.pri`:
```qmake
TRANSLATIONS += \
    $$PWD/qdomyos-zwift.ts \
    $$PWD/qdomyos-zwift_de.ts
```

### 2. Create translation file
```bash
lupdate src/qdomyos-zwift.pri -ts src/qdomyos-zwift_de.ts
```

### 3. Update resource file
Edit `src/translations.qrc`:
```xml
<RCC>
    <qresource prefix="/translations">
        <file>qdomyos-zwift_it.qm</file>
        <file>qdomyos-zwift_de.qm</file>
    </qresource>
</RCC>
```

### 4. Translate strings
Use Qt Linguist or text editor to translate the new `.ts` file.

### 5. Rebuild
```bash
qmake
make
```

## Translation Best Practices

### For Developers

1. **Always use translation functions**:
   ```cpp
   // Good
   QString text = tr("Speed: %1 km/h").arg(speed);

   // Bad
   QString text = "Speed: " + QString::number(speed) + " km/h";
   ```

2. **Provide context**:
   ```cpp
   // Good - context helps translators
   tr("Open", "verb - open a file");
   tr("Open", "adjective - door is open");
   ```

3. **Use placeholders for dynamic content**:
   ```cpp
   // Good
   tr("Distance: %1 %2").arg(distance).arg(unit);

   // Bad - hard to translate word order
   tr("Distance: ") + QString::number(distance) + " " + unit;
   ```

4. **Avoid breaking strings**:
   ```cpp
   // Good
   tr("This is a long string that spans multiple lines in the source code");

   // Bad - creates multiple translation entries
   tr("This is a long string") + tr(" that spans multiple lines");
   ```

### For Translators

1. **Keep formatting**:
   - Preserve `%1`, `%2` placeholders
   - Keep HTML tags if present
   - Maintain special characters like `\n`

2. **Consider context**:
   - UI space constraints (buttons, labels)
   - Technical terms vs. user-friendly terms
   - Cultural differences

3. **Test in application**:
   - Always test translations in the running app
   - Check for truncation or layout issues

## Troubleshooting

### Translations not loading
1. Check locale: `qDebug() << QLocale::system().name();`
2. Verify `.qm` file exists in `translations.qrc`
3. Check console output for translation loading messages

### Strings not updating
1. Run `lupdate` to extract new strings
2. Rebuild application with `make clean && make`
3. Check that strings use `tr()` or `qsTr()`

### Build errors
1. Ensure Qt Linguist tools are installed
2. Check that `.ts` files have correct XML syntax
3. Verify `translations.qrc` lists all `.qm` files

## Tools

### Qt Linguist
GUI tool for editing translations:
```bash
linguist src/qdomyos-zwift.ts
```

### lupdate
Extract translatable strings:
```bash
lupdate src/qdomyos-zwift.pri -ts src/qdomyos-zwift.ts
```

### lrelease
Compile translations:
```bash
lrelease src/qdomyos-zwift.ts
```

## Resources

- [Qt Linguist Manual](https://doc.qt.io/qt-5/qtlinguist-index.html)
- [Qt Internationalization](https://doc.qt.io/qt-5/internationalization.html)
- [QTranslator Class](https://doc.qt.io/qt-5/qtranslator.html)
