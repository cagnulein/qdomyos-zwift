# Weblate Setup Guide for QDomyos-Zwift

## Overview

Weblate is a web-based translation tool that allows non-programmers to contribute translations easily. This guide explains how to set it up for QDomyos-Zwift.

## Why Weblate?

- ✅ **Free for open source projects**
- ✅ **Native Qt `.ts` file support**
- ✅ **Direct GitHub integration**
- ✅ **User-friendly interface for translators**
- ✅ **Automatic quality checks**
- ✅ **Translation memory and suggestions**
- ✅ **Multiple translators can work simultaneously**
- ✅ **Mobile-friendly interface**

## Setup Options

### Option 1: Weblate Cloud (Recommended)

**Pros**: No maintenance, instant setup, free for open source
**Cons**: Data hosted by Weblate

**Steps**:

1. **Sign up at https://hosted.weblate.org**
   - Use your GitHub account to sign up
   - It's free for open source projects

2. **Create New Project**
   - Click "Add new translation project"
   - Name: "QDomyos-Zwift"
   - URL: https://github.com/cagnulein/qdomyos-zwift
   - License: Select your project license
   - Mark as "Public" and "Open source"

3. **Add Component**
   - Click "Add new translation component"
   - Name: "Application"
   - Repository: https://github.com/cagnulein/qdomyos-zwift.git
   - Branch: master (or main)
   - File mask: `src/qdomyos-zwift*.ts`
   - File format: "Qt Linguist Translation File"
   - Template file: Leave empty (Qt uses source strings)

4. **Configure GitHub Integration**
   - Go to Settings → Integrations
   - Enable "GitHub" integration
   - Add Weblate as a GitHub App to your repository
   - Configure webhook for automatic sync

5. **Set Translation Languages**
   - Add Italian (it_IT) as primary translation language
   - Can add more languages later (German, Spanish, French, etc.)

6. **Configure Workflow**
   - Settings → Workflow
   - Enable "Push changes from Weblate to repository"
   - Set "Commit message" template:
     ```
     i18n: Update {{ language_name }} translation

     Translated by {{ author }} via Weblate
     ```

7. **Invite Translators**
   - Share Weblate project URL with translators
   - They can sign up and start translating immediately
   - No programming knowledge required!

### Option 2: Self-Hosted Weblate

**Pros**: Full control, data on your servers
**Cons**: Requires server maintenance

**Requirements**:
- Docker or Linux server
- PostgreSQL or MySQL database
- Redis for caching

**Quick Setup with Docker**:
```bash
# Clone Weblate Docker setup
git clone https://github.com/WeblateOrg/docker-compose.git weblate-docker
cd weblate-docker

# Configure
cp .env.example .env
nano .env  # Edit configuration

# Set these variables:
# WEBLATE_SITE_DOMAIN=your-domain.com
# WEBLATE_ADMIN_PASSWORD=your-password
# POSTGRES_PASSWORD=postgres-password

# Start Weblate
docker-compose up -d

# Access at http://your-domain.com
```

Then follow steps 2-7 from Option 1.

## Translator Workflow

Once Weblate is set up, translators can:

1. **Login to Weblate**
   - Visit your Weblate project URL
   - Sign up or login (GitHub, Google, email)

2. **Select Language**
   - Choose Italian (or other language)
   - See translation progress (e.g., "234 / 500 strings translated")

3. **Translate Strings**
   - See source string (English)
   - Type translation (Italian)
   - View context (file location, nearby strings)
   - Use suggestions from translation memory

4. **Quality Checks**
   - Weblate automatically checks for:
     - Missing placeholders (%1, %2)
     - Punctuation consistency
     - Translation length (for UI elements)
     - HTML tag consistency

5. **Submit Translation**
   - Click "Save"
   - Translation is automatically committed to GitHub
   - Or batched and committed periodically

## Integration with GitHub Actions

Update the existing GitHub Actions workflow to pull translations from Weblate:

```yaml
name: Update Translations

on:
  workflow_dispatch:
  schedule:
    - cron: '0 0 * * 0'  # Weekly

jobs:
  update-translations:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      # Pull latest translations from Weblate
      - name: Pull from Weblate
        run: |
          # Weblate will automatically push changes
          # This job just extracts new source strings
          lupdate src/qdomyos-zwift.pri -ts src/qdomyos-zwift.ts

      - name: Push to Weblate
        run: |
          git config --local user.email "github-actions[bot]@users.noreply.github.com"
          git config --local user.name "github-actions[bot]"
          git add src/qdomyos-zwift.ts
          git commit -m "i18n: Update source strings" || echo "No changes"
          git push
          # Weblate will detect changes and update automatically
```

## Weblate Features for Translators

### 1. Translation Interface
```
┌─────────────────────────────────────────────────┐
│ Source: Speed: %1 km/h                          │
│                                                 │
│ Translation: Velocità: %1 km/h                  │
│                                                 │
│ [Save]  [Skip]  [Need editing]                 │
│                                                 │
│ Suggestions:                                    │
│ • Velocità: %1 km/h (100% match)                │
│ • Rapidità: %1 km/h (85% match)                 │
│                                                 │
│ Context:                                        │
│ File: src/HomeForm.ui.qml                       │
│ Nearby: Distance, Cadence, Power                │
└─────────────────────────────────────────────────┘
```

### 2. Translation Memory
- Automatically saves all translations
- Suggests translations for similar strings
- Learns from your translation style

### 3. Glossary
- Define consistent terminology
- Example: "bike" → "bici" (not "bicicletta")
- Ensures consistency across translations

### 4. Quality Checks
- ✅ Placeholders present: %1, %2, etc.
- ✅ HTML tags match: `<b>`, `</b>`
- ✅ Punctuation consistent
- ⚠️ Translation too long (might not fit in UI)
- ⚠️ Translation contains URL (should match source)

### 5. Comments & Discussion
- Translators can leave notes
- Ask for context or clarification
- Developers can respond

## For Developers: Making Strings Translator-Friendly

### 1. Add Context
```cpp
// Good - context helps translators
tr("Open", "verb - open a file");
tr("Open", "adjective - the door is open");

// Without context, translators see "Open" twice
// and don't know which meaning to use
```

### 2. Use Placeholders
```cpp
// Good - word order can vary between languages
tr("Downloaded %1 of %2 files").arg(current).arg(total);

// Bad - fixed word order
tr("Downloaded ") + QString::number(current) +
tr(" of ") + QString::number(total) + tr(" files");
```

### 3. Avoid String Concatenation
```cpp
// Good
tr("Welcome, %1!").arg(username);

// Bad - can't translate properly
tr("Welcome, ") + username + "!";
```

### 4. Add Comments for Translators
```cpp
//: Maximum speed the user can achieve
//~ This appears in the settings dialog
tr("Max Speed");
```

Comments appear in Weblate to help translators understand context.

## Translation Statistics & Monitoring

Weblate provides:
- **Progress Dashboard**: See completion percentage
- **Activity Feed**: Recent translations and changes
- **Contributor List**: Top translators
- **Quality Metrics**: String with issues
- **Export Options**: Download .ts files manually if needed

## Mobile App

Weblate has mobile apps for iOS and Android:
- Translate on the go
- Push notifications for new strings
- Offline mode

## Alternative: POEditor

If Weblate is too complex, consider POEditor (https://poeditor.com):
- Simpler interface
- Free for open source (up to 1000 strings)
- Good Qt .ts support
- Less features than Weblate

## Cost Comparison

| Service | Open Source | Private | Self-Hosted |
|---------|-------------|---------|-------------|
| **Weblate** | Free | €19/month | Free |
| **Crowdin** | Free | $40/month | N/A |
| **Transifex** | Free | $99/month | N/A |
| **POEditor** | Free (1000 strings) | $19/month | N/A |

## Recommended Setup for QDomyos-Zwift

1. **Start with Weblate Cloud** (free, easy)
2. **Set up automatic sync** with GitHub
3. **Invite Italian translators** to Weblate
4. **Keep GitHub Actions** for extracting new strings
5. **Weblate pushes** completed translations back to GitHub
6. **Compile .qm files** during build (already configured)

## Getting Started

1. Go to https://hosted.weblate.org
2. Sign up with GitHub
3. Click "Add project"
4. Follow setup wizard
5. Share Weblate URL with translators

**Time to setup**: ~15 minutes
**Time for translators to start**: 2 minutes (just sign up)

## Support

- Weblate Documentation: https://docs.weblate.org
- Weblate Community: https://github.com/WeblateOrg/weblate/discussions
- Qt Translation Guide: https://doc.qt.io/qt-5/qtlinguist-index.html

## Example Weblate Projects

Check out these open source projects using Weblate:
- https://hosted.weblate.org/projects/f-droid/
- https://hosted.weblate.org/projects/godot-engine/
- https://hosted.weblate.org/projects/opensuse/

## Summary

✅ **Best Choice**: Weblate Cloud (hosted)
✅ **Setup Time**: 15 minutes
✅ **Cost**: Free for open source
✅ **Translator Experience**: Excellent
✅ **Integration**: Seamless with GitHub
✅ **Maintenance**: Zero (cloud) or minimal (self-hosted)

Would you like me to set up Weblate for your project?
