# Flutter & Dart Project Structure Guide

## Overview

This guide provides architecture and organization guidance for Flutter and Dart projects. It covers directory organization, naming conventions, and structural patterns that scale from small apps to large enterprise applications.

---

## Standard Flutter Project Structure

### Root Directory

```
my_flutter_app/
├── lib/                    # Application source code
├── test/                   # Unit and widget tests
├── integration_test/       # Integration tests
├── android/                # Android-specific code
├── ios/                    # iOS-specific code
├── web/                    # Web-specific code
├── windows/                # Windows desktop code
├── macos/                  # macOS desktop code
├── linux/                  # Linux desktop code
├── assets/                 # Images, fonts, data files
├── docs/                   # Project documentation
├── pubspec.yaml            # Dependencies and configuration
├── analysis_options.yaml   # Linter rules
├── README.md               # Project overview
├── CHANGELOG.md            # Version history
└── CLAUDE.md               # Claude Code configuration
```

### Key Files

- **`pubspec.yaml`** - Package dependencies, assets, Flutter SDK constraints
- **`analysis_options.yaml`** - Dart linter and analyzer configuration
- **`README.md`** - Project documentation and setup instructions
- **`CLAUDE.md`** - Claude Code configuration (installed by this template)

---

## lib/ Directory Organization

### Basic Structure

```
lib/
├── main.dart              # Application entry point
├── app.dart               # Root app widget (MaterialApp/CupertinoApp)
├── core/                  # Shared core functionality
│   ├── constants/         # App-wide constants
│   ├── themes/            # Theme data
│   ├── utils/             # Utility functions
│   └── extensions/        # Dart extension methods
├── features/              # Feature-based modules
│   └── feature_name/
│       ├── data/          # Data sources, repositories
│       ├── domain/        # Business logic, entities
│       └── presentation/  # UI, state management
├── shared/                # Shared widgets and models
│   ├── widgets/           # Reusable UI components
│   └── models/            # Shared data models
└── config/                # Configuration files
    ├── routes/            # Routing configuration
    └── di/                # Dependency injection setup
```

### Entry Point (main.dart)

```dart
// lib/main.dart
import 'package:flutter/material.dart';
import 'app.dart';

void main() {
  runApp(const MyApp());
}
```

### Root App Widget (app.dart)

```dart
// lib/app.dart
import 'package:flutter/material.dart';

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'My Flutter App',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const HomeScreen(),
    );
  }
}
```

---

## test/ Directory Organization

Tests should mirror the `lib/` directory structure for easy navigation:

```
test/
├── unit/                  # Unit tests (pure Dart logic)
│   ├── core/
│   │   └── utils/
│   │       └── validators_test.dart
│   └── features/
│       └── auth/
│           └── domain/
│               └── usecases/
│                   └── login_user_test.dart
├── widget/                # Widget tests (UI components)
│   └── features/
│       └── auth/
│           └── presentation/
│               └── pages/
│                   └── login_page_test.dart
└── helpers/               # Test helpers and mocks
    ├── test_helpers.dart
    └── mock_data.dart
```

### Integration Tests

```
integration_test/
├── app_test.dart
└── flows/
    ├── auth_flow_test.dart
    └── checkout_flow_test.dart
```

---

## Architecture Patterns

### 1. Simple Structure (Small Apps <500 lines)

Best for: MVPs, prototypes, learning projects

```
lib/
├── main.dart
├── screens/               # Full page widgets
│   ├── home_screen.dart
│   └── details_screen.dart
├── widgets/               # Reusable components
│   └── custom_button.dart
├── models/                # Data models
│   └── user.dart
├── services/              # Business logic
│   └── api_service.dart
└── utils/                 # Helper functions
    └── validators.dart
```

**Pros:**
- Simple to understand
- Fast to implement
- Low cognitive overhead

**Cons:**
- Doesn't scale well
- Hard to test in isolation
- Business logic mixed with UI

---

### 2. Feature-Based Organization (Medium Apps 500-5000 lines)

**Recommended for most projects**

Best for: Medium to large apps with distinct features

```
lib/
├── main.dart
├── app.dart
├── core/
│   ├── constants/
│   │   ├── app_constants.dart
│   │   └── api_constants.dart
│   ├── themes/
│   │   └── app_theme.dart
│   └── utils/
│       └── validators.dart
├── features/
│   ├── authentication/
│   │   ├── data/
│   │   │   └── auth_repository.dart
│   │   └── presentation/
│   │       ├── login_screen.dart
│   │       └── signup_screen.dart
│   ├── home/
│   │   └── presentation/
│   │       └── home_screen.dart
│   └── profile/
│       └── presentation/
│           └── profile_screen.dart
└── shared/
    ├── widgets/
    │   ├── loading_indicator.dart
    │   └── error_widget.dart
    └── models/
        └── user.dart
```

**Benefits:**
- Clear feature boundaries
- Easy to navigate
- Team-friendly (features can be owned)
- Scalable structure
- Good separation of concerns

**When to migrate from Simple:**
- App exceeds 500 lines
- Multiple developers working
- Features start overlapping
- Testing becomes difficult

---

### 3. Clean Architecture (Large Apps 5000+ lines)

Best for: Large applications requiring strict layer separation

```
lib/
├── main.dart
├── app.dart
├── core/
│   ├── error/
│   │   ├── exceptions.dart
│   │   └── failures.dart
│   ├── network/
│   │   └── network_info.dart
│   ├── usecases/
│   │   └── usecase.dart
│   └── utils/
│       └── input_converter.dart
└── features/
    └── authentication/
        ├── data/
        │   ├── datasources/
        │   │   ├── auth_remote_datasource.dart
        │   │   └── auth_local_datasource.dart
        │   ├── models/
        │   │   └── user_model.dart
        │   └── repositories/
        │       └── auth_repository_impl.dart
        ├── domain/
        │   ├── entities/
        │   │   └── user.dart
        │   ├── repositories/
        │   │   └── auth_repository.dart
        │   └── usecases/
        │       ├── login_user.dart
        │       └── logout_user.dart
        └── presentation/
            ├── bloc/
            │   ├── auth_bloc.dart
            │   ├── auth_event.dart
            │   └── auth_state.dart
            ├── pages/
            │   ├── login_page.dart
            │   └── signup_page.dart
            └── widgets/
                └── login_form.dart
```

**Layer Responsibilities:**

1. **Presentation Layer** (`presentation/`)
   - UI components (pages, widgets)
   - State management (BLoC, Provider, etc.)
   - User interaction handling

2. **Domain Layer** (`domain/`)
   - Business logic (use cases)
   - Entity definitions
   - Repository interfaces (contracts)

3. **Data Layer** (`data/`)
   - Repository implementations
   - Data sources (remote/local)
   - Data models (DTOs)

**Benefits:**
- Strict separation of concerns
- Highly testable
- Independent layers
- Easy to swap implementations

**Drawbacks:**
- More boilerplate
- Steeper learning curve
- Overkill for small apps

---

## State Management Organization

### Provider / Riverpod

```
lib/
├── features/
│   └── auth/
│       ├── providers/
│       │   └── auth_provider.dart
│       └── presentation/
│           └── login_screen.dart
└── shared/
    └── providers/
        └── theme_provider.dart
```

### BLoC Pattern

```
lib/
└── features/
    └── auth/
        └── presentation/
            ├── bloc/
            │   ├── auth_bloc.dart
            │   ├── auth_event.dart
            │   └── auth_state.dart
            └── pages/
                └── login_page.dart
```

### GetX

```
lib/
├── controllers/
│   └── auth_controller.dart
├── bindings/
│   └── auth_binding.dart
└── views/
    └── login_view.dart
```

---

## Best Practices

### File Naming Conventions

- Use `snake_case` for file names: `user_profile_screen.dart`
- Match class names to file names: `UserProfileScreen` in `user_profile_screen.dart`
- Test files: `user_profile_screen_test.dart`
- One class per file (with exceptions for tightly coupled classes)

### Directory Naming

- **Descriptive names**: `authentication/` not `auth/`
- **Plural for collections**: `widgets/`, `models/`, `screens/`
- **Singular for single-purpose**: `domain/`, `data/`, `presentation/`
- **snake_case** for directories: `feature_name/`, not `featureName/`

### Code Organization

- Keep files under 300-400 lines
- Extract widgets to separate files when they exceed 50-100 lines
- Group related functionality in directories
- Separate UI from business logic

### Import Organization

Follow this order for imports:

```dart
// 1. Dart SDK imports
import 'dart:async';
import 'dart:io';

// 2. Package imports (alphabetical)
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

// 3. Local project imports (alphabetical)
import 'package:myapp/core/constants/app_constants.dart';
import 'package:myapp/features/auth/presentation/login_screen.dart';
import 'package:myapp/shared/widgets/custom_button.dart';
```

---

## Common Project Examples

### Example 1: Small Flutter App (Todo List)

```
lib/
├── main.dart
├── screens/
│   ├── home_screen.dart
│   └── add_todo_screen.dart
├── widgets/
│   ├── todo_item.dart
│   └── custom_fab.dart
├── models/
│   └── todo.dart
└── utils/
    └── database_helper.dart
```

### Example 2: Medium E-Commerce App

```
lib/
├── main.dart
├── app.dart
├── core/
│   ├── constants/
│   ├── themes/
│   └── utils/
├── features/
│   ├── auth/
│   │   ├── data/
│   │   │   └── auth_repository.dart
│   │   └── presentation/
│   │       ├── login_screen.dart
│   │       └── signup_screen.dart
│   ├── products/
│   │   ├── data/
│   │   │   └── products_repository.dart
│   │   └── presentation/
│   │       ├── product_list_screen.dart
│   │       └── product_detail_screen.dart
│   ├── cart/
│   │   ├── data/
│   │   │   └── cart_repository.dart
│   │   └── presentation/
│   │       └── cart_screen.dart
│   └── checkout/
│       └── presentation/
│           └── checkout_screen.dart
└── shared/
    ├── widgets/
    │   ├── product_card.dart
    │   └── loading_indicator.dart
    └── models/
        ├── product.dart
        └── cart_item.dart
```

### Example 3: Large Social Media App (Clean Architecture)

```
lib/
├── main.dart
├── app.dart
├── core/
│   ├── error/
│   ├── network/
│   ├── usecases/
│   └── utils/
└── features/
    ├── feed/
    │   ├── data/
    │   │   ├── datasources/
    │   │   ├── models/
    │   │   └── repositories/
    │   ├── domain/
    │   │   ├── entities/
    │   │   ├── repositories/
    │   │   └── usecases/
    │   └── presentation/
    │       ├── bloc/
    │       ├── pages/
    │       └── widgets/
    ├── profile/
    │   ├── data/
    │   ├── domain/
    │   └── presentation/
    ├── chat/
    │   ├── data/
    │   ├── domain/
    │   └── presentation/
    └── notifications/
        ├── data/
        ├── domain/
        └── presentation/
```

---

## Assets Organization

### Structure

```
assets/
├── images/
│   ├── icons/
│   │   └── app_icon.png
│   ├── logos/
│   │   └── company_logo.svg
│   └── backgrounds/
│       └── splash_bg.jpg
├── fonts/
│   ├── Roboto-Regular.ttf
│   └── Roboto-Bold.ttf
└── data/
    └── countries.json
```

### pubspec.yaml Configuration

```yaml
flutter:
  assets:
    - assets/images/
    - assets/images/icons/
    - assets/images/logos/
    - assets/data/

  fonts:
    - family: Roboto
      fonts:
        - asset: assets/fonts/Roboto-Regular.ttf
        - asset: assets/fonts/Roboto-Bold.ttf
          weight: 700
```

---

## Platform-Specific Code

```
android/          # Android native code
  └── app/
      └── src/
          └── main/
              ├── AndroidManifest.xml
              └── kotlin/

ios/              # iOS native code
  └── Runner/
      ├── Info.plist
      └── AppDelegate.swift

web/              # Web specific files
  └── index.html

windows/          # Windows desktop
macos/            # macOS desktop
linux/            # Linux desktop
```

Keep platform-specific code in these directories. Use platform channels for native integration.

---

## Configuration Files

### pubspec.yaml

```yaml
name: my_flutter_app
description: A Flutter application
version: 1.0.0+1

environment:
  sdk: '>=3.0.0 <4.0.0'

dependencies:
  flutter:
    sdk: flutter
  cupertino_icons: ^1.0.2
  provider: ^6.0.0

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^2.0.0

flutter:
  uses-material-design: true
```

### analysis_options.yaml

```yaml
include: package:flutter_lints/flutter.yaml

linter:
  rules:
    - prefer_const_constructors
    - avoid_print
    - prefer_single_quotes
```

### .gitignore

```
# Flutter/Dart generated files
.dart_tool/
.packages
build/
.flutter-plugins
.flutter-plugins-dependencies

# IDE files
.idea/
.vscode/
*.iml

# Platform-specific
android/.gradle/
ios/Pods/
ios/.symlinks/

# Secrets
.env
*.key
firebase_options.dart
```

---

## Migration Strategies

### From Simple to Feature-Based

1. Create `features/` directory
2. Identify distinct features in your app
3. Create feature directories: `features/feature_name/`
4. Move related screens/widgets to feature directories
5. Create `data/` and `presentation/` subdirectories
6. Update imports throughout the project
7. Test after each feature migration

**Example Migration:**

```
# Before
lib/screens/login_screen.dart
lib/screens/signup_screen.dart

# After
lib/features/authentication/presentation/login_screen.dart
lib/features/authentication/presentation/signup_screen.dart
```

### From Feature-Based to Clean Architecture

1. Within each feature, create layer directories: `data/`, `domain/`, `presentation/`
2. Identify pure business objects → Move to `domain/entities/`
3. Extract business logic → Create use cases in `domain/usecases/`
4. Create repository interfaces in `domain/repositories/`
5. Implement repositories in `data/repositories/`
6. Separate data sources: `data/datasources/`
7. Convert models to DTOs in `data/models/`
8. Move UI to `presentation/pages/` and `presentation/widgets/`
9. Add state management in `presentation/bloc/` or `presentation/providers/`
10. Update imports and test thoroughly

---

## Development Workflow with This Structure

### Adding a New Feature

1. **Plan**: Create feature directory: `lib/features/feature_name/`
2. **TDD**: Use `/tdd-new` command to create task specification
3. **Structure**: Create subdirectories based on chosen architecture
4. **Test Structure**: Mirror in `test/features/feature_name/`
5. **Implement**: Use `/tdd-implement` for Red-Green-Refactor workflow
6. **Document**: Update README or docs if significant feature

### Refactoring

1. Use feature branches for structural changes
2. Update tests first to ensure they still pass with new structure
3. Refactor incrementally, one feature or layer at a time
4. Commit after each logical refactoring step
5. Run full test suite before merging

### Code Review Checklist

- [ ] Files follow naming conventions (snake_case)
- [ ] Imports organized correctly (Dart, packages, local)
- [ ] No files exceed 400 lines
- [ ] Tests mirror lib/ structure
- [ ] Business logic separated from UI
- [ ] No hardcoded values (use constants)
- [ ] Proper error handling

---

## References

- [Flutter Documentation](https://flutter.dev/docs)
- [Effective Dart](https://dart.dev/effective-dart)
- [Flutter Project Structure Guide](https://docs.flutter.dev/development/tools/sdk/overview)
- [Clean Architecture by Robert Martin](https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html)
- [Flutter Best Practices](https://docs.flutter.dev/cookbook)

## Related Documentation

- `flutter-dart-rules.md` - Code style and development rules
- `flutter-tdd-guide.md` - TDD implementation patterns
- `version-control.md` - Git workflow and commits
- `CLAUDE.md` - Claude Code configuration

---

*Last Updated: 2025-11-25*
