# Your Comprehensive Git Workflow Cheatsheet

A complete reference for common Git operations and scenarios you'll encounter in development.

---

## **1. Getting Started**

- **Clone the repository:**
  ```bash
  git clone https://github.com/Bassai-Sho/qdomyos-zwift.git
  ```
- **Add the upstream remote (original repository):**
  ```bash
  git remote add upstream https://github.com/cagnulein/qdomyos-zwift.git
  ```
- **Check your remote repositories:**
  ```bash
  git remote -v
  ```
- **Set up your Git identity:** *Required for commits to have proper authorship*
  ```bash
  git config --global user.name "Your Name"
  git config --global user.email "your.email@example.com"
  ```

---

## **2. Branch Management & Development**

- **Create a new feature branch and switch to it:**
  ```bash
  git checkout -b feat/ant_footpod
  ```
- **List all branches (local and remote):** *See what branches exist and which one you're on (marked with *)*
  ```bash
  git branch -a
  ```
- **Switch between branches:** *Move to a different branch to work on different features*
  ```bash
  git checkout branch_name
  git switch branch_name  # newer alternative
  ```
- **Stage and commit your changes on the feature branch:**
  ```bash
  git add .
  git commit -m "feat: adding ant footpod support"
  ```
- **Push your feature branch to your GitHub repository:**
  ```bash
  git push origin feat/ant_footpod
  ```
- **Set upstream tracking for easier pushes:** *Links your local branch to remote, so you can just use `git push` later*
  ```bash
  git push -u origin feat/ant_footpod
  # Now you can just use: git push
  ```
- **Delete a local feature branch:** *⚠️ SAFE: Only deletes if branch is merged. Use -D to force delete*
  ```bash
  git checkout master # First, switch to master
  git branch -d feat/ant_footpod
  git branch -D feat/ant_footpod  # ⚠️ DESTRUCTIVE: Force delete even if not merged
  ```
- **Delete a remote branch on GitHub:** *⚠️ DESTRUCTIVE: Permanently removes branch from GitHub*
  ```bash
  git push origin --delete feat/ant_footpod
  ```

---

## **3. Keeping Branches Updated**

- **Update your local `master` with the latest from `cagnulein`:**
  ```bash
  git checkout master
  git pull upstream master
  git push origin master
  ```
- **Update your feature branch with the latest `master` changes:**
  ```bash
  git checkout feat/ant_footpod
  git merge master
  ```
- **Alternative: Rebase instead of merge (cleaner history):** *⚠️ CAUTION: Rewrites commit history - don't use on shared branches*
  ```bash
  git checkout feat/ant_footpod
  git rebase master
  ```
- **Fetch all remote changes without merging:** *Safe way to see what's new without changing your code*
  ```bash
  git fetch --all
  ```

---

## **4. Commit Management**

- **View commit history:**
  ```bash
  git log --oneline
  git log --graph --oneline --all  # Visual branch history
  ```
- **Amend the last commit (change message or add files):** *⚠️ CAUTION: Changes commit hash - don't use if already pushed to shared branch*
  ```bash
  git add forgotten_file.txt
  git commit --amend -m "Updated commit message"
  ```
- **Interactive rebase to edit/squash commits:** *⚠️ CAUTION: Rewrites history - use only on private branches*
  ```bash
  git rebase -i HEAD~3  # Edit last 3 commits
  ```
- **Cherry-pick a specific commit to current branch:** *Copies a commit from another branch - safe operation*
  ```bash
  git cherry-pick <commit-hash>
  ```
- **Create a commit with detailed message:**
  ```bash
  git commit -m "feat: add new feature

  - Added functionality X
  - Fixed issue Y
  - Updated documentation"
  ```

---

## **5. Fixing Mistakes & Restoring Files**

- **Stash uncommitted changes:** Temporarily save changes on the current branch.
  ```bash
  git stash
  git stash push -m "Work in progress on feature X"
  ```
- **List and manage stashes:**
  ```bash
  git stash list
  git stash show stash@{0}
  git stash drop stash@{0}  # Delete specific stash
  ```
- **Apply stashed changes to a different branch:**
  ```bash
  git checkout <other_branch>
  git stash pop
  git stash apply stash@{0}  # Apply specific stash without removing it
  ```
- **Restore a file to the state of another branch (`master`):**
  ```bash
  git checkout master -- src/devices/proformtreadmill/proformtreadmill.cpp
  ```
- **Completely restore a local branch to the GitHub version:** *⚠️ DESTRUCTIVE: Permanently loses all local changes*
  ```bash
  git checkout feat/ant_footpod
  git reset --hard origin/feat/ant_footpod
  git pull
  ```
- **Undo last commit but keep changes:** *Safe - moves changes back to staging area*
  ```bash
  git reset --soft HEAD~1
  ```
- **Undo last commit and discard changes:** *⚠️ DESTRUCTIVE: Permanently loses the commit and all changes*
  ```bash
  git reset --hard HEAD~1
  ```
- **Discard all uncommitted changes:** *⚠️ DESTRUCTIVE: Loses all work since last commit*
  ```bash
  git checkout .
  git restore .  # newer alternative
  ```

---

## **6. File and Project Management**

- **Check status of your working directory:**
  ```bash
  git status
  git status -s  # Short format
  ```
- **See what changed in files:**
  ```bash
  git diff  # Unstaged changes
  git diff --staged  # Staged changes
  git diff HEAD  # All changes since last commit
  git diff --name-only master feat/ant_footpod  # Shows only filenames that differ between branches
  ```
- **Add build artifacts to `.gitignore`:**
  - Create/edit `.gitignore` at the project root.
  - Add lines for files to ignore: `src/qdomyos-zwift-arm64`
  - Commit the `.gitignore` file: `git add .gitignore` then `git commit -m "chore: ignore build files"`
- **Remove a file from Git tracking but keep it locally:** *Useful for files you want to keep but not track (like configs)*
  ```bash
  git rm --cached <file_path>
  git add .gitignore
  git commit -m "chore: remove file from tracking"
  ```
- **Stage specific parts of a file:** *Interactive mode lets you choose which changes to commit*
  ```bash
  git add -p <file_name>  # Interactive staging
  ```

---

## **7. Collaboration & Pull Requests**

- **Create a pull request workflow:**
  1. Push your feature branch: `git push origin feat/new-feature`
  2. Go to GitHub and create a pull request
  3. After review and merge, clean up:
     ```bash
     git checkout master
     git pull upstream master
     git branch -d feat/new-feature
     git push origin --delete feat/new-feature
     ```
- **Review someone else's branch locally:** *Download and test someone's changes before they're merged*
  ```bash
  git fetch origin
  git checkout -b review-branch origin/their-feature-branch
  ```

---

## **8. Advanced Operations**

- **Merge vs Rebase strategies:**
  - **Merge**: Preserves branch history, creates merge commits
    ```bash
    git merge feature-branch
    ```
  - **Rebase**: Linear history, cleaner but rewrites commits
    ```bash
    git rebase master
    ```
- **Interactive rebase for cleaning up commits:** *⚠️ CAUTION: Rewrites history - only use on private branches*
  ```bash
  git rebase -i HEAD~3
  # Use 'squash' to combine commits, 'edit' to modify
  ```
- **Find when a bug was introduced:** *Binary search through commits to find when bug appeared*
  ```bash
  git bisect start
  git bisect bad HEAD
  git bisect good <known-good-commit>
  ```
- **Blame/annotate lines in a file:**
  ```bash
  git blame <file_name>
  git log -p <file_name>  # See file history
  ```

---

## **9. Using Visual Studio Code**

- **Open your WSL2 project in VS Code:** From your WSL2 terminal, navigate to the project directory and run `code .`
- **Search for a string:**
  - In the current file: `Ctrl + F`
  - In the entire project: `Ctrl + Shift + F`
- **Compare branches:**
  1. Open **Source Control** view.
  2. Click the branch name in the status bar.
  3. Select **"Compare with..."** and choose `master`.
     *(Note: This is often provided by the GitLens extension.)*
- **VS Code Git shortcuts:**
  - `Ctrl + Shift + G`: Open Source Control
  - `Ctrl + Shift + P`: Command palette → "Git: " commands
  - Stage files by clicking the "+" next to them
  - View file diff by clicking on modified files

---

## **10. Troubleshooting Common Issues**

- **Merge conflicts:**
  ```bash
  # After conflict occurs
  git status  # See conflicted files
  # Edit files to resolve conflicts
  git add <resolved_files>
  git commit -m "resolve merge conflicts"
  ```
- **Accidentally committed to wrong branch:** *Move your commit to the correct branch*
  ```bash
  git reset --soft HEAD~1  # Undo commit, keep changes
  git checkout correct-branch
  git commit -m "commit message"
  ```
- **Recover deleted branch:** *Git keeps commits for ~30 days even after branch deletion*
  ```bash
  git reflog  # Find the commit hash
  git checkout -b recovered-branch <commit-hash>
  ```
- **Clean up untracked files:** *⚠️ DESTRUCTIVE: Permanently deletes files not tracked by Git*
  ```bash
  git clean -n  # SAFE: Preview what would be deleted
  git clean -f  # ⚠️ DESTRUCTIVE: Delete untracked files
  git clean -fd  # ⚠️ DESTRUCTIVE: Delete untracked files and directories
  ```

---

## **11. Configuration & Aliases**

- **Useful Git aliases:**
  ```bash
  git config --global alias.st status
  git config --global alias.co checkout
  git config --global alias.br branch
  git config --global alias.ci commit
  git config --global alias.unstage 'reset HEAD --'
  git config --global alias.last 'log -1 HEAD'
  git config --global alias.visual '!gitk'
  ```
- **Pretty log format:**
  ```bash
  git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"
  ```

---

## **12. Emergency Commands**

- **"Oh no, I messed up everything!":** *Git saves everything - you can usually recover*
  ```bash
  git reflog  # See all your recent actions
  git reset --hard HEAD@{n}  # ⚠️ DESTRUCTIVE: Go back to a previous state
  ```
- **Completely start over with clean repository:** *⚠️ DESTRUCTIVE: Loses ALL local changes*
  ```bash
  git fetch origin
  git reset --hard origin/master
  git clean -fd
  ```
- **Save work before panic:** *Always save your work before trying risky operations*
  ```bash
  git add .
  git commit -m "WIP: saving current work before fix"
  ```