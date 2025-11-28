# Claude Code Configuration

This Flutter/Dart project is configured to work with Claude Code terminal.

## Quick Start

This project includes Claude Code configuration for streamlined TDD development workflows.

### Available Commands

- **`/tdd-new`** - Create a new TDD task with pre-development checklist
- **`/tdd-test`** - Run tests with framework detection (dart test, flutter test)
- **`/tdd-implement`** - Execute full Red-Green-Refactor TDD workflow

### Documentation

- **`.claude/version-control.md`** - Git workflow and commit guidelines
- **`.claude/flutter-dart-rules.md`** - Flutter/Dart development best practices (888 lines)
- **`.claude/flutter-tdd-guide.md`** - TDD patterns for Flutter (695 lines)
- **`.claude/project-structure.md`** - Architecture and organization guidance

---

## Development Workflow

### Using TDD Commands

#### 1. Create a New Feature

```bash
/tdd-new Add user authentication
```

This will:
- Create `notes/features/add-user-authentication.md` for planning
- Create `.claude/tdd-tasks/add-user-authentication.md` for TDD specification
- Guide you through pre-development checklist
- Prompt for feature requirements and test specifications

#### 2. Run Tests

```bash
/tdd-test test/auth/user_auth_test.dart
```

Or run all tests:
```bash
/tdd-test
```

This will:
- Auto-detect test framework (flutter test vs dart test)
- Run specified tests or all tests
- Show TDD phase context (Red/Green/Refactor)
- Run code analysis (flutter analyze or dart analyze)

#### 3. Implement with TDD

```bash
/tdd-implement .claude/tdd-tasks/add-user-authentication.md
```

This will:
- Guide through Red-Green-Refactor cycle
- Create clean commits for each phase
- Ensure tests pass before completion
- Follow approval-gated commit workflow

---

## Project Guidelines

### Flutter/Dart Best Practices

See `.claude/flutter-dart-rules.md` for comprehensive guidelines including:

- **Code Style**: PascalCase for classes, camelCase for members, snake_case for files
- **Widget Composition**: Prefer composition over inheritance
- **State Management**: Separate ephemeral and app state
- **Null Safety**: Sound null safety, avoid `!` unless guaranteed non-null
- **Async/Await**: Proper use of Future and Stream
- **Error Handling**: Try-catch blocks with appropriate exceptions
- **Testing**: Write testable code with dependency injection

### Testing Approach

See `.claude/flutter-tdd-guide.md` for TDD patterns:

- **Unit Tests**: Pure Dart functions with `package:test`
- **Widget Tests**: UI components with `package:flutter_test`
- **Integration Tests**: Complete flows with `package:integration_test`
- **Mocking**: Use `mockito` or `mocktail` for dependencies
- **Test Organization**: Mirror lib/ structure in test/

### Version Control

See `.claude/version-control.md` for commit workflow:

- **Approval-Gated Commits**: Claude proposes, you approve
- **Documentation Updates**: CHANGELOG always, README when needed
- **Conventional Commits**: feat, fix, refactor, docs, test, chore
- **Version Numbering**: Semantic versioning (MAJOR.MINOR.PATCH)

### Project Structure

See `.claude/project-structure.md` for architecture guidance:

- **Feature-Based Organization**: Recommended for medium/large apps
- **Clean Architecture**: For complex apps with strict layer separation
- **Directory Naming**: snake_case, descriptive names
- **File Organization**: Under 300-400 lines per file
- **Import Organization**: Dart, packages, local (alphabetical)

---

## Pre-Commit Checklist

Before proposing commits, Claude Code should:

- [ ] Run `flutter pub get` if dependencies changed
- [ ] Run `flutter analyze` and fix any issues
- [ ] Run `flutter test` and ensure all tests pass
- [ ] Update `CHANGELOG.md` with changes (always required)
- [ ] Update `README.md` if user-facing changes
- [ ] Format code with `flutter format` or `dart format`
- [ ] Verify no secrets or credentials in staged files

---

## Configuration Files

### .claude/ Directory Structure

```
.claude/
  commands/           # TDD workflow commands
    tdd-new.md
    tdd-test.md
    tdd-implement.md
  tdd-tasks/          # Generated TDD task specifications
  templates/          # Templates used by commands
    feature-notes.md
  tdd-template.md     # TDD task template
  .template-version   # Installed template version
```

### Generated During Development

```
notes/
  features/           # Feature planning notes (created by /tdd-new)
```

---

## Getting Help

### Command Documentation

Each command has detailed documentation in `.claude/commands/`:

- Open `.claude/commands/tdd-new.md` for `/tdd-new` usage examples
- Open `.claude/commands/tdd-test.md` for `/tdd-test` usage examples
- Open `.claude/commands/tdd-implement.md` for `/tdd-implement` workflow details

### Best Practices Documentation

- **`.claude/flutter-dart-rules.md`** - 888 lines of Flutter/Dart development guidelines
- **`.claude/flutter-tdd-guide.md`** - 695 lines of TDD patterns and examples
- **`.claude/project-structure.md`** - Architecture patterns and organization
- **`.claude/version-control.md`** - Git workflow and commit format

---

## Customization

This configuration was installed by [flutter_claude_code_template](https://github.com/your-org/flutter_claude_code_template).

**Installed Version**: Check `.claude/.template-version`

### To Customize:

1. **Modify commands**: Edit `.claude/commands/*.md` to adjust command behavior
2. **Update guidelines**: Edit `.claude/*.md` files to reflect project-specific practices
3. **Extend this file**: Add project-specific Claude Code instructions below
4. **Add documentation**: Place new docs in `.claude/` and link them in `.claude/CLAUDE.md`

---

## Project-Specific Guidelines

[Add your project-specific guidelines here]

### Team Conventions
- [Your team's coding conventions]
- [Specific state management approach used]
- [API integration patterns]

### Development Environment
- Flutter version: [e.g., 3.24.0]
- Dart version: [e.g., 3.5.0]
- Target platforms: [iOS, Android, Web, etc.]

### Additional Commands
- [Any custom Claude Code commands you create]

---

## References

- **Template Repository**: https://github.com/your-org/flutter_claude_code_template
- **Flutter Documentation**: https://flutter.dev/docs
- **Dart Documentation**: https://dart.dev/guides
- **Effective Dart**: https://dart.dev/effective-dart
- **Flutter TDD**: https://flutter.dev/docs/cookbook/testing

## Questions or Issues?

If you encounter problems with the Claude Code configuration:

1. Check `.claude/commands/*.md` for command documentation
2. Review `.claude/CLAUDE.md` and linked documentation files
3. Verify `.claude/.template-version` matches your expected version
4. Consult your project maintainers or team lead

---

*This file is the entry point for Claude Code when working in this project. Claude Code reads this file automatically to understand project structure, available commands, and development workflows.*
