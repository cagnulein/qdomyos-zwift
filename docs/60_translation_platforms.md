# Online Translation Platforms (Free OSS)

This project uses Qt `.ts` files in `src/translations/`.
You can manage community translations online with either Weblate or Crowdin.

## Option A: Weblate (recommended for this repo)

Best fit for existing bilingual Qt `.ts` files.

### Why

- Native support for Qt Linguist `.ts`
- Works well with per-language files (`qdomyos-zwift_it.ts`, `qdomyos-zwift_de.ts`, ...)
- Free hosted plan available for libre/open-source projects (upon request)

### Setup steps

1. Create a project on Hosted Weblate and connect this GitHub repository.
2. Add a component with:
   - File mask: `src/translations/qdomyos-zwift_*.ts`
   - File format: `Qt Linguist Translation File`
   - Bilingual mode (monolingual base language file left empty)
3. Enable automatic commits/PRs from Weblate.
4. Keep your existing CI (`lupdate` + `lrelease`) as validation.

### Notes

- Use Weblate translation checks and reviewer workflow before merge.
- Keep placeholders (`%1`, `%2`), tags, and units intact.

## Option B: Crowdin (also valid)

Use if your contributors already work in Crowdin.

### Why

- Free open-source license is available (subject to eligibility).
- Good GitHub integration and PR automation.
- Native Qt TS format support.

### Setup steps

1. Create a Crowdin project and apply for the open-source free license.
2. Connect GitHub repository to Crowdin (or use `crowdin/github-action`).
3. Configure source pattern for `src/translations/qdomyos-zwift_*.ts`.
4. Enable PR-based translation sync back to GitHub.

### Suggested GitHub Secrets (if using Crowdin Action)

- `CROWDIN_PROJECT_ID`
- `CROWDIN_PERSONAL_TOKEN`
- `GH_TOKEN` (token with repo write access for PR creation)

## Recommendation

For this Qt5 repository structure, start with Weblate first (simpler for bilingual `.ts` files),
then evaluate Crowdin only if your community explicitly prefers it.

## References

- Weblate hosted service: https://hosted.weblate.org/
- Weblate Qt TS format docs: https://docs.weblate.org/en/latest/admin/formats/qt.html
- Crowdin open-source program: https://store.crowdin.com/open-source
- Crowdin GitHub integration: https://store.crowdin.com/github
- Crowdin GitHub Action: https://github.com/crowdin/github-action
