# Flutter Test-Driven Development (TDD) Guide

This guide provides Flutter-specific TDD implementation patterns and best practices. For framework-agnostic TDD principles, see [TDD Guidelines](../../claude/tdd-guidelines.md).

---

## Overview

Test-Driven Development in Flutter follows the same Red-Green-Refactor cycle as other frameworks, but with Flutter-specific testing tools and patterns:

- **Unit Tests:** Using `package:test` for pure Dart functions and classes
- **Widget Tests:** Using `package:flutter_test` for UI components
- **Integration Tests:** Using `package:integration_test` for complete user flows

---

## Flutter Testing Frameworks

### Unit Tests (`package:test`)

Used for testing Dart code without any Flutter dependencies.

**When to use:**
- Business logic (validators, formatters, calculators)
- Service classes
- Repository classes
- Data models

**Example:**
```dart
// test/core/validators/email_validator_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:myapp/core/validators/email_validator.dart';

void main() {
  group('EmailValidator', () {
    test('returns true for valid email', () {
      expect(validateEmail('user@example.com'), isTrue);
    });

    test('returns false for invalid email', () {
      expect(validateEmail('invalid'), isFalse);
    });

    test('returns false for empty string', () {
      expect(validateEmail(''), isFalse);
    });
  });
}
```

**Run unit tests:**
```bash
dart test test/core/validators/email_validator_test.dart
dart test                                            # Run all tests
dart test -v                                         # Verbose output
dart test --coverage                                 # Generate coverage report
```

### Widget Tests (`package:flutter_test`)

Used for testing Flutter widgets and UI behavior.

**When to use:**
- Widget rendering and layout
- Button interactions
- Form validation UI
- State changes
- Navigation

**Example:**
```dart
// test/features/counter/presentation/counter_screen_test.dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:myapp/features/counter/presentation/counter_screen.dart';

void main() {
  group('CounterScreen', () {
    testWidgets('displays initial counter value', (WidgetTester tester) async {
      // Arrange: Build the widget
      await tester.pumpWidget(const MyApp());

      // Assert: Initial state
      expect(find.text('0'), findsOneWidget);
    });

    testWidgets('increments counter when fab is tapped', (WidgetTester tester) async {
      // Arrange
      await tester.pumpWidget(const MyApp());

      // Act: Tap the FAB
      await tester.tap(find.byIcon(Icons.add));
      await tester.pump();

      // Assert: Counter incremented
      expect(find.text('1'), findsOneWidget);
    });
  });
}
```

**Key methods:**
- `pumpWidget()` - Build widget tree
- `pump()` - Trigger rebuild after interaction
- `pumpAndSettle()` - Pump until no more animations
- `tap()` - Simulate tap interaction
- `enterText()` - Enter text into text field
- `find` - Locate widgets in tree

**Run widget tests:**
```bash
flutter test test/features/counter/presentation/counter_screen_test.dart
flutter test -v                                      # Verbose output
```

### Integration Tests (`package:integration_test`)

Used for testing complete user workflows across multiple screens.

**When to use:**
- End-to-end user flows
- Multi-screen navigation
- Data persistence
- API integration
- Performance under real conditions

**Example:**
```dart
// integration_test/app_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:myapp/main.dart' as app;

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('App Integration Tests', () {
    testWidgets('complete user flow test', (WidgetTester tester) async {
      // Build the app
      app.main();
      await tester.pumpAndSettle();

      // Navigate to feature
      await tester.tap(find.byText('Feature Name'));
      await tester.pumpAndSettle();

      // Interact with form
      await tester.enterText(find.byType(TextField), 'input value');
      await tester.tap(find.byType(ElevatedButton));
      await tester.pumpAndSettle();

      // Verify result
      expect(find.text('Success'), findsOneWidget);
    });
  });
}
```

**Run integration tests:**
```bash
flutter test integration_test/app_test.dart
flutter test integration_test/ -v                    # All integration tests
```

---

## Flutter Project Structure for TDD

```
myapp/
├── lib/
│   ├── main.dart
│   ├── core/
│   │   ├── validators/
│   │   │   └── email_validator.dart
│   │   ├── services/
│   │   │   └── auth_service.dart
│   │   └── utils/
│   │       └── formatters.dart
│   └── features/
│       ├── counter/
│       │   ├── presentation/
│       │   │   └── counter_screen.dart
│       │   ├── domain/
│       │   │   └── counter_service.dart
│       │   └── data/
│       │       └── counter_repository.dart
│       └── auth/
│           ├── presentation/
│           │   └── login_screen.dart
│           ├── domain/
│           │   └── auth_service.dart
│           └── data/
│               └── auth_repository.dart
│
├── test/
│   ├── core/
│   │   ├── validators/
│   │   │   └── email_validator_test.dart
│   │   ├── services/
│   │   │   └── auth_service_test.dart
│   │   └── utils/
│   │       └── formatters_test.dart
│   └── features/
│       ├── counter/
│       │   ├── presentation/
│       │   │   └── counter_screen_test.dart
│       │   ├── domain/
│       │   │   └── counter_service_test.dart
│       │   └── data/
│       │       └── counter_repository_test.dart
│       └── auth/
│           └── ...
│
├── integration_test/
│   └── app_test.dart
│
├── pubspec.yaml
└── analysis_options.yaml
```

**Key principles:**
- Test directory mirrors lib/ structure
- Each implementation file has corresponding test file
- Test files end with `_test.dart`
- Integration tests in separate `integration_test/` directory

---

## Flutter Testing Patterns

### Unit Test Pattern

```dart
void main() {
  group('Feature Name', () {
    test('when condition, then expected outcome', () {
      // Arrange
      final validator = EmailValidator();
      const email = 'test@example.com';

      // Act
      final result = validator.validate(email);

      // Assert
      expect(result, isTrue);
    });
  });
}
```

### Widget Test Pattern

```dart
void main() {
  group('FeatureWidget', () {
    testWidgets('displays content', (WidgetTester tester) async {
      // Arrange
      await tester.pumpWidget(
        MaterialApp(
          home: FeatureWidget(),
        ),
      );

      // Act
      // (widget automatically rendered by pumpWidget)

      // Assert
      expect(find.text('Expected Text'), findsOneWidget);
    });

    testWidgets('when button tapped, then action occurs', (WidgetTester tester) async {
      // Arrange
      await tester.pumpWidget(MaterialApp(home: FeatureWidget()));

      // Act
      await tester.tap(find.byType(ElevatedButton));
      await tester.pump();

      // Assert
      expect(find.text('Result'), findsOneWidget);
    });
  });
}
```

### Integration Test Pattern

```dart
void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('Feature Integration Tests', () {
    testWidgets('complete user flow', (WidgetTester tester) async {
      // Arrange
      app.main();
      await tester.pumpAndSettle();

      // Act
      await tester.tap(find.byText('Start'));
      await tester.pumpAndSettle();

      // Assert
      expect(find.byType(FeatureScreen), findsOneWidget);
    });
  });
}
```

---

## Testing Common Flutter Scenarios

### Testing Form Validation

```dart
testWidgets('form validation shows error for invalid email', (WidgetTester tester) async {
  await tester.pumpWidget(const MyApp());

  // Enter invalid email
  await tester.enterText(find.byType(TextField), 'invalid');
  await tester.tap(find.byType(ElevatedButton));
  await tester.pump();

  // Verify error shown
  expect(find.text('Invalid email'), findsOneWidget);
});
```

### Testing State Management

```dart
testWidgets('counter increments when button is tapped', (WidgetTester tester) async {
  await tester.pumpWidget(const MyApp());

  // Initial state
  expect(find.text('0'), findsOneWidget);

  // Tap button
  await tester.tap(find.byType(FloatingActionButton));
  await tester.pump();

  // Verify state changed
  expect(find.text('1'), findsOneWidget);
  expect(find.text('0'), findsNothing);
});
```

### Testing Navigation

```dart
testWidgets('navigates to details screen when item tapped', (WidgetTester tester) async {
  await tester.pumpWidget(const MyApp());

  // Tap item
  await tester.tap(find.text('Item 1'));
  await tester.pumpAndSettle();

  // Verify navigation
  expect(find.byType(DetailsScreen), findsOneWidget);
});
```

### Testing Async Operations

```dart
testWidgets('loads data when screen opens', (WidgetTester tester) async {
  await tester.pumpWidget(const MyApp());
  await tester.pumpAndSettle(); // Wait for all async operations

  // Verify data is loaded
  expect(find.text('Loaded Data'), findsOneWidget);
  expect(find.byType(CircularProgressIndicator), findsNothing);
});
```

### Testing Mocks and Dependencies

```dart
import 'package:mockito/mockito.dart';

class MockAuthService extends Mock implements AuthService {}

void main() {
  group('LoginScreen', () {
    testWidgets('shows error when login fails', (WidgetTester tester) async {
      final mockAuthService = MockAuthService();
      when(mockAuthService.login(any, any))
          .thenThrow(Exception('Login failed'));

      // Build widget with mock
      await tester.pumpWidget(
        MaterialApp(
          home: LoginScreen(authService: mockAuthService),
        ),
      );

      // Interact and verify
      await tester.enterText(find.byType(TextField).first, 'user@example.com');
      await tester.enterText(find.byType(TextField).last, 'password');
      await tester.tap(find.byType(ElevatedButton));
      await tester.pumpAndSettle();

      expect(find.text('Login failed'), findsOneWidget);
    });
  });
}
```

---

## Flutter TDD Commands

### Test Command

```bash
# Run all tests
flutter test

# Run specific test file
flutter test test/features/counter/presentation/counter_screen_test.dart

# Run with coverage
flutter test --coverage

# Verbose output
flutter test -v

# Watch mode (re-run on file changes)
flutter test --watch
```

### Analysis Command

```bash
# Analyze code for issues
flutter analyze

# Fix common issues automatically
dart fix --apply

# Format code
dart format lib/

# Check formatting without changing
dart format --line-length=80 --output=none lib/
```

### Running Tests in Different Phases

**RED Phase (tests should fail):**
```bash
flutter test test/features/[feature]_test.dart
# Expected: ❌ Tests fail (feature not implemented yet)
```

**GREEN Phase (tests should pass):**
```bash
flutter test test/features/[feature]_test.dart
# Expected: ✅ All tests pass (minimal implementation done)
```

**REFACTOR Phase (tests should still pass):**
```bash
flutter test test/features/[feature]_test.dart
dart format lib/features/[feature]/
flutter analyze lib/features/[feature]/
# Expected: ✅ Tests pass, code formatted, no analysis issues
```

---

## Flutter-Specific Best Practices

### 1. Use Proper Widget Testing

```dart
// ❌ DON'T: Test implementation details
expect(find.byWidgetPredicate((w) => w is Scaffold && w.floatingActionButton != null), findsOneWidget);

// ✅ DO: Test behavior and user-visible elements
expect(find.byType(FloatingActionButton), findsOneWidget);
expect(find.text('Add Item'), findsOneWidget);
```

### 2. Keep Widgets Small and Testable

```dart
// ❌ DON'T: Large, complex widgets
class MyWidget extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      // 200 lines of UI code...
    );
  }
}

// ✅ DO: Compose smaller, testable widgets
class MyWidget extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SingleChildScrollView(
        child: Column(
          children: [
            _buildHeader(),
            _buildContent(),
            _buildFooter(),
          ],
        ),
      ),
    );
  }

  Widget _buildHeader() => HeaderWidget();
  Widget _buildContent() => ContentWidget();
  Widget _buildFooter() => FooterWidget();
}

// Each sub-widget can be tested independently
```

### 3. Inject Dependencies for Testability

```dart
// ❌ DON'T: Create dependencies inside widget
class LoginScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    final authService = AuthService(); // Hard to mock
    // ...
  }
}

// ✅ DO: Inject dependencies
class LoginScreen extends StatelessWidget {
  final AuthService authService;

  const LoginScreen({required this.authService});

  @override
  Widget build(BuildContext context) {
    // Uses provided authService, easy to mock in tests
  }
}
```

### 4. Test Edge Cases

```dart
void main() {
  group('EmailValidator', () {
    test('happy path: valid email', () {
      expect(validateEmail('user@example.com'), isTrue);
    });

    test('edge case: email with special characters', () {
      expect(validateEmail('user+tag@example.co.uk'), isTrue);
    });

    test('error case: missing @', () {
      expect(validateEmail('userexample.com'), isFalse);
    });

    test('edge case: empty string', () {
      expect(validateEmail(''), isFalse);
    });

    test('edge case: whitespace', () {
      expect(validateEmail('  '), isFalse);
    });
  });
}
```

### 5. Use Golden Tests for UI

For complex UI widgets, use golden tests to catch visual regressions:

```dart
testWidgets('widget looks correct', (WidgetTester tester) async {
  await tester.binding.window.physicalSizeTestValue = const Size(800, 600);
  addTearDown(tester.binding.window.clearPhysicalSizeTestValue);

  await tester.pumpWidget(const MyApp());

  await expectLater(
    find.byType(MyWidget),
    matchesGoldenFile('golden/my_widget.png'),
  );
});
```

---

## Troubleshooting Flutter Tests

### Tests are slow

**Causes:** Network calls, file I/O, unnecessary waits
**Solutions:**
- Use mocks for external dependencies
- Use `pump()` instead of `pumpAndSettle()` when possible
- Split integration tests from unit tests
- Run unit tests frequently, integration tests less often

### Widget not found in test

**Causes:** Widget not built, incorrect finder, widget hidden
**Solutions:**
```dart
// Debug: Print widget tree
debugPrintBeginFrame = true;

// Or use common issues:
await tester.pumpWidget(...); // Build widget
await tester.pump(); // Rebuild after interaction
await tester.pumpAndSettle(); // Wait for animations

// Verify widget exists before assertion
expect(find.byType(MyWidget), findsWidgets); // Should find at least one
```

### Text field input not working

```dart
// ✅ Correct way
await tester.enterText(find.byType(TextField), 'input');
await tester.pump(); // Rebuild to show text

// ❌ Common mistake
await tester.tap(find.byType(TextField));
// Forgot to rebuild after tap
```

### Async operations timeout

```dart
// Use pumpAndSettle() to wait for all async to complete
await tester.pumpAndSettle();

// Or specify timeout
await tester.pumpAndSettle(const Duration(seconds: 5));

// For specific futures
await tester.pumpAndSettle(const Duration(milliseconds: 500));
```

---

## Flutter TDD Workflow Commands

### Create new Flutter feature with TDD:
```bash
/tdd-new Add user profile feature
```

### Run Flutter tests:
```bash
/tdd-test test/features/user_profile_test.dart
```

### Implement feature with Flutter TDD:
```bash
/tdd-implement .claude/tdd-tasks/add-user-profile.md
```

---

## Additional Resources

- **Flutter Testing Documentation:** https://flutter.dev/docs/testing
- **Flutter Test Package:** https://pub.dev/packages/flutter_test
- **Flutter Integration Test:** https://pub.dev/packages/integration_test
- **Mockito Package:** https://pub.dev/packages/mockito
- **General TDD Guide:** `../../claude/tdd-guidelines.md`

---

## Key Takeaways

✅ Write tests first (Red phase)
✅ Keep implementations minimal (Green phase)
✅ Improve code quality safely (Refactor phase)
✅ Use appropriate test type (unit, widget, integration)
✅ Keep tests independent and fast
✅ Mock external dependencies
✅ Test behavior, not implementation
✅ Clean commits with clear messages

---

**Last Updated:** 2025-11-23
