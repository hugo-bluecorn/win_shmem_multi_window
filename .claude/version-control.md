# Version Control & Git Workflow

This document describes how Claude Code handles version control for this project.

## Overview

We use an **approval-gated commit workflow** where Claude Code stages changes and proposes commits, but requires your explicit approval before executing them. This ensures quality control while automating the mechanical aspects of version management.

## Commit Workflow

### Step 1: Claude Code Proposes Changes

When Claude Code completes a task:
1. It stages all modified files
2. It generates a clear, descriptive commit message
3. It proposes the commit to you with the message and file list

### Step 2: Review and Approve

Before Claude Code commits, you should:
1. Review the staged files and changes
2. Review the proposed commit message
3. Approve or request modifications

### Step 3: Documentation Updates

Before proposing the commit, Claude Code will:
1. Identify which documentation files are affected by the changes
2. Update relevant files according to the guide below
3. Include documentation updates in the staged changes

### Step 4: Execute Commit

Once approved, Claude Code commits the changes with the approved message.

## Documentation Update Strategy

The following documentation files should be updated based on the type of changes:

### README.md

Update when:
- Adding or modifying the project's functionality
- Changing how to run or build the project
- Updating installation or usage instructions
- Adding new features or capabilities

Include in README:
- Description of the new/modified feature
- Updated setup or usage instructions
- Updated examples (if applicable)

### CHANGELOG.md

Update **always** before every commit. Use the following format:

```
## [Unreleased]

### Added
- Description of new features

### Changed
- Description of modified existing features

### Fixed
- Description of bug fixes

### Removed
- Description of removed features
```

### Project Documentation

Update when:
- Modifying how developers use this project
- Changing the project process
- Updating project structure documentation

### Code Files

Update when:
- Adding new functionality
- Modifying existing features
- Fixing bugs
- Refactoring code

When modifying code:
- Ensure changes align with project goals
- Test changes thoroughly
- Document any assumptions about project structure

## Documentation Update Guidelines

When updating documentation:

1. **Be concise and clear** — Use language a developer new to the project can understand
2. **Include examples** — Provide code snippets or command examples where helpful
3. **Update cross-references** — If you add a new section, update related documentation
4. **Keep CHANGELOG up-to-date** — This is the historical record of all changes
5. **Test instructions** — If you document a process, verify it actually works

## Identifying Affected Documentation

Use this decision tree to determine which files to update:

- **Is this adding a new feature?** → Update CHANGELOG and README
- **Is this modifying existing functionality?** → Update CHANGELOG and relevant documentation
- **Is this fixing a bug?** → Update CHANGELOG
- **Is this refactoring with no user-facing changes?** → Update CHANGELOG only
- **Is this documentation-only?** → Update only the relevant documentation file

## Example Commit Workflow

1. You ask Claude Code: "Add new feature X"
2. Claude Code makes the changes and proposes:
   - Files modified: `src/feature-x.dart`
   - Documentation updated: `CHANGELOG.md` (new feature added)
   - Proposed message: "feat: add feature X with Y capability"
3. You review and approve (or request changes)
4. Claude Code executes the commit

## Commit Message Format

Claude Code will use conventional commit format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Where:
- **type**: feat, fix, docs, style, refactor, perf, test, chore
- **scope**: optional, the area affected
- **subject**: concise description (imperative mood, lowercase)
- **body**: optional, more detailed explanation
- **footer**: optional, references to issues (e.g., "Closes #123")

### Example Commit Messages

```
feat: add user authentication support

Implements login and registration with JWT tokens.

fix: resolve null pointer exception in data processing

Added proper null checks before accessing object properties.

docs: update setup instructions

Clarified Flutter version requirements and added troubleshooting section.

refactor: simplify widget composition

Extracted complex nested widgets into smaller, reusable components.
```

## Special Cases

### Emergency Fixes

If you need to bypass the approval process for critical fixes, explicitly instruct Claude Code to do so. The CHANGELOG should still be updated.

### Large Changes

For significant changes:
1. Break into multiple smaller commits where logically possible
2. Each commit should be independently buildable
3. Update documentation incrementally with each commit

### Reverting Changes

If you need to revert a commit:
1. Provide the commit hash or message to Claude Code
2. Claude Code will propose the revert with an explanation
3. Follow the normal approval workflow
4. Update CHANGELOG to document the revert

## Reviewing Documentation Updates

When reviewing Claude Code's documentation updates, check:

- [ ] Changes accurately reflect the code changes
- [ ] Examples are correct and runnable
- [ ] Language is clear and consistent with existing docs
- [ ] No dead links or broken references
- [ ] CHANGELOG entry is descriptive and follows the format

## References

- **Keep a Changelog:** https://keepachangelog.com/ — Standard format for maintaining changelog files
- **Conventional Commits:** https://www.conventionalcommits.org/ — Specification for commit message format
- **Semantic Versioning:** https://semver.org/ — Specification for version numbering
