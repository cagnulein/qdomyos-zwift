# Translation Workflow - Dual Mode Operation

## Overview

The translation workflow operates in two modes:

1. **🧪 Testing Mode** - During Pull Requests
2. **🚀 Production Mode** - Scheduled or Manual runs

## Modes Explained

### 🧪 Testing Mode

**When it runs**:
- On every commit to a Pull Request
- On every push to branches matching:
  - `claude/**` (Claude Code branches)
  - `translations/**` (Translation-specific branches)

**What it does**:
- ✅ Extracts translatable strings using `lupdate`
- ✅ Updates `src/qdomyos-zwift.ts` if changes detected
- ✅ Commits changes **directly to the current PR branch**
- ❌ Does NOT create a new PR

**Why?**
- Allows testing the workflow during development
- Validates that `lupdate` works correctly
- Ensures translation files stay in sync during PR development
- Provides immediate feedback on translation changes

**Example Flow**:
```
Developer pushes code to PR branch
         ↓
Workflow detects .cpp/.qml/.h changes
         ↓
Runs lupdate to extract strings
         ↓
Commits updated .ts file to same branch
         ↓
Developer sees changes in their PR
```

### 🚀 Production Mode

**When it runs**:
- **Scheduled**: Every Sunday at midnight (UTC)
- **Manual**: Via GitHub Actions UI (workflow_dispatch)

**What it does**:
- ✅ Extracts translatable strings using `lupdate`
- ✅ Updates `src/qdomyos-zwift.ts` if changes detected
- ✅ Creates a **new branch**: `translations/auto-update-YYYYMMDD-HHMMSS`
- ✅ Creates a **new Pull Request** with:
  - Title: "🌍 Update translation strings"
  - Labels: `translations`, `automation`
  - Detailed description

**Why?**
- Automated weekly translation updates
- Keeps translation files synchronized with codebase
- Creates isolated PRs for translation review
- Separates translation work from feature development

**Example Flow**:
```
Sunday midnight (or manual trigger)
         ↓
Workflow extracts all translatable strings
         ↓
Creates new branch: translations/auto-update-20231210-120000
         ↓
Commits updated .ts file
         ↓
Creates PR for review
         ↓
Translator adds Italian translations
         ↓
Merge PR when complete
```

## Triggers Summary

| Trigger | Mode | Creates PR | Branch |
|---------|------|------------|--------|
| Pull Request commit | Testing | ❌ No | Current PR branch |
| Push to `claude/**` | Testing | ❌ No | Current branch |
| Push to `translations/**` | Testing | ❌ No | Current branch |
| Weekly schedule (Sunday) | Production | ✅ Yes | New branch |
| Manual trigger | Production | ✅ Yes | New branch |

## File Paths That Trigger the Workflow

The workflow runs when changes are detected in:
- `src/**/*.cpp` - C++ source files
- `src/**/*.h` - C++ header files
- `src/**/*.qml` - QML UI files
- `src/**/*.ui` - Qt UI files
- `src/**/*.ts` - Translation files

**Note**: Only applies to Testing Mode. Production mode runs on schedule/manual trigger regardless of changes.

## Workflow Output

### Testing Mode Output
```
🧪 Running in TESTING mode (PR/push to branch)
Extracting translatable strings from source code...
Checking for changes...
📝 Translation file has been updated with new/modified strings

Changes summary:
 src/qdomyos-zwift.ts | 15 +++++++++------

✅ Changes committed to current branch

============================================
Translation Workflow Summary
============================================
Mode: testing
Event: push
Has changes: true
============================================
✅ Translation file updated in current branch
   Review changes in: src/qdomyos-zwift.ts
```

### Production Mode Output
```
🚀 Running in PRODUCTION mode (scheduled/manual)
Extracting translatable strings from source code...
Checking for changes...
📝 Translation file has been updated with new/modified strings

Changes summary:
 src/qdomyos-zwift.ts | 15 +++++++++------

✅ Changes pushed to new branch: translations/auto-update-20231210-120000

============================================
Translation Workflow Summary
============================================
Mode: production
Event: schedule
Has changes: true
============================================
✅ Translation file updated and PR created
   Branch: translations/auto-update-20231210-120000
```

## How to Use

### For Developers (During PR Development)

1. **Create your PR** with code changes
2. **Workflow runs automatically** on every commit
3. **Review the .ts changes** in your PR
4. **Continue developing** - translations stay in sync

**Example**:
```bash
# You're working on feature branch
git checkout -b feature/add-new-screen
git commit -m "Add new workout screen"
git push origin feature/add-new-screen

# Workflow runs automatically
# Commits translation updates to your branch
# You see them in your PR
```

### For Translators (Weekly Updates)

1. **Wait for automated PR** (created every Sunday)
2. **Review new strings** in `src/qdomyos-zwift.ts`
3. **Add Italian translations**:
   - Via Weblate (recommended)
   - Via Qt Linguist
   - Via text editor
4. **Merge PR** when translations complete

### For Maintainers (Manual Trigger)

**When to use**:
- Before a release to ensure translations are current
- After merging multiple PRs with UI changes
- When requested by translators

**How to trigger**:
1. Go to **Actions** tab on GitHub
2. Select **"Update Translations"** workflow
3. Click **"Run workflow"**
4. Select branch (usually `master` or `main`)
5. Click **"Run workflow"** button

## Benefits of Dual Mode

### Testing Mode Benefits
- ✅ **Immediate feedback** during development
- ✅ **No extra PRs** cluttering the repository
- ✅ **Validates workflow** before merge
- ✅ **Keeps translations in sync** with code

### Production Mode Benefits
- ✅ **Isolated translation work** in separate PRs
- ✅ **Weekly automation** ensures nothing is missed
- ✅ **Clean separation** between code and translations
- ✅ **Easy review process** for translators

## Advanced Configuration

### Adjusting Schedule

To change the weekly schedule, edit `.github/workflows/update-translations.yml`:

```yaml
schedule:
  - cron: '0 0 * * 0'  # Current: Sunday at midnight UTC
  # Examples:
  # - cron: '0 0 * * 1'  # Monday at midnight
  # - cron: '0 12 * * 5'  # Friday at noon
  # - cron: '0 0 1 * *'  # First day of month
```

### Adding More Branch Patterns

To trigger Testing Mode on additional branches:

```yaml
push:
  branches:
    - 'claude/**'
    - 'translations/**'
    - 'feature/**'  # Add this
    - 'i18n/**'      # And this
```

### Excluding Certain Paths

To prevent workflow from running on certain changes:

```yaml
pull_request:
  paths:
    - 'src/**/*.cpp'
    - 'src/**/*.h'
    - '!src/test/**'  # Exclude test files
```

## Troubleshooting

### Workflow doesn't run on my PR

**Check**:
1. Is your branch named `claude/**` or `translations/**`?
2. Did you modify any `.cpp`, `.h`, `.qml`, `.ui`, or `.ts` files?
3. Check GitHub Actions tab for errors

### Translation file not updating

**Possible causes**:
1. No translatable strings in changed files
2. Strings not wrapped in `tr()` or `qsTr()`
3. `lupdate` not finding the files

**Solution**:
- Check workflow logs in GitHub Actions
- Ensure strings use `tr()` in C++ or `qsTr()` in QML

### Too many commits in my PR

If the workflow commits on every push:

**Solution**:
- Squash commits before merging
- Or temporarily disable workflow for your branch

### Want to disable Testing Mode temporarily

**Option 1 - Disable for specific PR**:
Add `[skip ci]` to commit message:
```bash
git commit -m "WIP: testing changes [skip ci]"
```

**Option 2 - Disable workflow file**:
Temporarily rename the workflow:
```bash
mv .github/workflows/update-translations.yml \
   .github/workflows/update-translations.yml.disabled
```

## Monitoring

### GitHub Actions Dashboard

View workflow runs at:
```
https://github.com/YOUR-ORG/qdomyos-zwift/actions/workflows/update-translations.yml
```

### Notifications

You'll receive notifications for:
- ✅ Workflow success
- ❌ Workflow failure
- 📝 PR created (Production mode)

### Status Checks

The workflow appears as a status check on PRs:
- 🟢 Green: No changes or successfully updated
- 🔴 Red: Workflow failed (check logs)
- 🟡 Yellow: Workflow running

## Best Practices

### For Developers

1. **Review translation changes** in your PRs
2. **Use meaningful string contexts**: `tr("Open", "verb")`
3. **Don't ignore workflow failures**
4. **Add comments for complex strings**

### For Reviewers

1. **Check that new strings are translatable**
2. **Verify placeholder usage** (`%1`, `%2`)
3. **Ensure strings aren't too technical**
4. **Consider UI space constraints**

### For Translators

1. **Wait for Production PRs** for main translation work
2. **Use Testing Mode PRs** for quick feedback during development
3. **Test translations** in the application
4. **Ask for context** via PR comments if unclear

## Integration with Weblate

When using Weblate:

1. **Testing Mode**: Weblate syncs automatically with PR changes
2. **Production Mode**: Weblate creates its own commits/PRs
3. **Avoid conflicts**: Let Weblate handle Production PRs
4. **Manual intervention**: Only needed for Testing Mode PRs

## Summary

| Aspect | Testing Mode | Production Mode |
|--------|-------------|-----------------|
| **Trigger** | PR commits, branch pushes | Weekly schedule, manual |
| **Frequency** | Every commit | Weekly or on-demand |
| **Target** | Current PR branch | New branch |
| **Creates PR** | No | Yes |
| **Purpose** | Validation & sync | Automated updates |
| **Audience** | Developers | Translators |
| **Commit message** | `[automated]` tag | Clean message |

## Questions?

- Check GitHub Actions logs for detailed output
- Review this document for configuration options
- Open an issue for workflow problems
- See `TRANSLATIONS.md` for general translation info
