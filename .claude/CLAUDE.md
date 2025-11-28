# Claude Code Configuration

This Flutter/Dart project is configured with Claude Code.

## Key Documentation

Read these documents to understand the project:

- [flutter-dart-rules.md](./flutter-dart-rules.md) - Flutter/Dart best practices
- [flutter-tdd-guide.md](./flutter-tdd-guide.md) - TDD patterns for Flutter
- [project-structure.md](./project-structure.md) - Architecture guidance
- [version-control.md](./version-control.md) - Git workflow and commits

## Commands Available

- `/tdd-new` - Create new TDD task
- `/tdd-test` - Run tests with framework detection
- `/tdd-implement` - Full Red-Green-Refactor workflow

## Templates

- `tdd-template.md` - TDD task template
- `templates/feature-notes.md` - Feature planning template

---

## Adding Additional Documentation

To add more files to Claude Code's context:

1. Place your documentation file in `.claude/` directory
2. Add a link to this file under "## Key Documentation" section above
3. Use relative paths: `[filename.md](./filename.md)`

**Example:**
```markdown
## Key Documentation

- [flutter-dart-rules.md](./flutter-dart-rules.md) - Flutter/Dart best practices
- [your-new-doc.md](./your-new-doc.md) - Your documentation description
```

Claude Code will automatically load these linked files into context on startup, making them available for reference during development.

For more information, see the main [CLAUDE.md](../CLAUDE.md) in the project root.
