# TDD Task: Refactor Flutter Integration for Testability

**Status:** In Progress
**Created:** 2025-11-29
**Priority:** Critical - Root cause diagnosis and fix

---

## Problem Statement

Cross-process shared memory works correctly in isolation (proven by unit tests and standalone tests), but the Flutter app doesn't synchronize window counts across processes. We need to:

1. **Diagnose** - Find the exact point of failure in the integration
2. **Refactor** - Break monolithic code into testable components
3. **Test** - Write comprehensive tests for each component
4. **Fix** - Implement solutions with test coverage

---

## Proven Facts

| Component | Status | Evidence |
|-----------|--------|----------|
| SharedMemoryManager C++ | ✅ WORKS | 20/20 unit tests pass |
| Cross-process sharing | ✅ WORKS | Standalone test shows magic marker shared |
| WindowCountListener | ⚠️ Untested | Build failed (CreateEventA naming conflict) |
| DartPortManager | ⚠️ Untested | Build failed (dart_api_dl.h missing) |
| Dart FFI bindings | ❓ Unknown | No tests exist |
| End-to-end integration | ❌ BROKEN | Windows show count=1 instead of 2,3,4... |

---

## Current Architecture Analysis

### Layer 1: SharedMemoryManager (C++)
```
Status: ✅ Working
Location: windows/runner/shared_memory_manager.{h,cpp}
Tests: windows/test/shared_memory_manager_test.cpp (20 tests)
```

### Layer 2: WindowCountListener (C++)
```
Status: ⚠️ Has naming conflict with Windows API
Location: windows/runner/window_count_listener.{h,cpp}
Issue: CreateEvent method conflicts with Windows CreateEventA macro
Tests: windows/test/window_count_listener_test.cpp (won't build)
```

### Layer 3: DartPortManager (C++)
```
Status: ⚠️ Depends on Dart API headers
Location: windows/runner/dart_port_manager.{h,cpp}
Issue: dart_api_dl.h not available in standalone test builds
Tests: windows/test/dart_port_manager_test.cpp (won't build)
```

### Layer 4: FFI Bindings (Dart)
```
Status: ❓ No tests
Location: lib/window_manager_ffi.dart
Tests: None exist
```

### Layer 5: UI Integration (Dart)
```
Status: ❓ Partial widget tests
Location: lib/main.dart
Tests: test/widget/window_controls_test.dart (partial)
```

---

## Refactoring Plan

### Phase 1: Fix C++ Test Build Issues

#### Task 1.1: Fix WindowCountListener naming conflict

**Problem:** `CreateEvent` method name conflicts with Windows `CreateEventA` macro.

**Solution:** Rename method to `CreateUpdateEvent()` or use explicit `::CreateEventA`.

**Test:** Build and run `window_count_listener_test.cpp`.

#### Task 1.2: Mock Dart API for test builds

**Problem:** `dart_api_dl.h` not available outside Flutter build.

**Solution:** Create mock header for tests that provides stub types.

**Test:** Build and run `dart_port_manager_test.cpp`.

---

### Phase 2: Add Dart FFI Tests

#### Task 2.1: Create FFI unit tests

**File:** `test/unit/window_manager_ffi_test.dart`

**Tests:**
- [ ] FFI library loads successfully
- [ ] InitDartApiDL returns 0 (success)
- [ ] RegisterWindowCountPort returns true
- [ ] UnregisterWindowCountPort returns true
- [ ] SendPort.nativePort returns valid value

#### Task 2.2: Create FFI integration tests

**File:** `test/integration/ffi_integration_test.dart`

**Tests:**
- [ ] ReceivePort receives messages from C++
- [ ] Multiple ports can be registered
- [ ] Ports receive broadcast notifications
- [ ] Port unregistration stops messages

---

### Phase 3: Refactor main.dart for Testability

#### Current Structure (Monolithic)

```dart
// lib/main.dart - 252 lines, hard to test
class _WindowCounterScreenState {
  // FFI initialization
  // ReceivePort management
  // UI state management
  // Window creation/destruction
  // Error handling
  // All mixed together
}
```

#### Target Structure (Separated Concerns)

```
lib/
├── main.dart                    # App entry point only
├── features/
│   └── window_counter/
│       ├── presentation/
│       │   ├── window_counter_screen.dart
│       │   └── widgets/
│       │       ├── window_count_display.dart
│       │       └── window_control_buttons.dart
│       ├── domain/
│       │   ├── window_count_service.dart
│       │   └── window_manager_service.dart
│       └── data/
│           └── native_bridge.dart  # FFI wrapper
```

#### Task 3.1: Extract NativeBridge class

**File:** `lib/features/window_counter/data/native_bridge.dart`

```dart
/// Abstracts FFI communication with C++ layer.
///
/// Testable via dependency injection with mock implementation.
abstract class NativeBridge {
  Future<bool> initialize();
  Stream<int> get windowCountStream;
  Future<void> dispose();
}

class WindowManagerNativeBridge implements NativeBridge {
  // Real FFI implementation
}

class MockNativeBridge implements NativeBridge {
  // Test implementation
}
```

**Tests:**
- [ ] initialize() returns true on success
- [ ] windowCountStream emits values
- [ ] dispose() cleans up resources
- [ ] Handles FFI errors gracefully

#### Task 3.2: Extract WindowCountService

**File:** `lib/features/window_counter/domain/window_count_service.dart`

```dart
/// Business logic for window count management.
///
/// Decoupled from UI and FFI for testability.
class WindowCountService {
  final NativeBridge _bridge;

  WindowCountService(this._bridge);

  Stream<int> get countStream => _bridge.windowCountStream;
  int get currentCount => _currentCount;
}
```

**Tests:**
- [ ] Exposes count stream from bridge
- [ ] Tracks current count value
- [ ] Handles stream errors

#### Task 3.3: Extract WindowManagerService

**File:** `lib/features/window_counter/domain/window_manager_service.dart`

```dart
/// Handles window creation and destruction.
///
/// Abstracts Process.start for testability.
abstract class ProcessRunner {
  Future<void> startDetached(String executable, List<String> args);
}

class WindowManagerService {
  final ProcessRunner _processRunner;

  Future<void> createWindow();
  void closeCurrentWindow();
}
```

**Tests:**
- [ ] createWindow() calls ProcessRunner.startDetached
- [ ] closeCurrentWindow() calls exit(0)
- [ ] Handles process errors

#### Task 3.4: Refactor WindowCounterScreen

**File:** `lib/features/window_counter/presentation/window_counter_screen.dart`

```dart
/// UI only - receives services via constructor/provider.
class WindowCounterScreen extends StatelessWidget {
  final WindowCountService countService;
  final WindowManagerService windowManager;

  // Pure UI, no FFI or Process logic
}
```

**Tests:**
- [ ] Displays count from service stream
- [ ] Create button calls windowManager.createWindow()
- [ ] Close button calls windowManager.closeCurrentWindow()
- [ ] Shows error state on service errors

---

### Phase 4: Add Integration Tests

#### Task 4.1: Create end-to-end integration test

**File:** `integration_test/multi_window_test.dart`

**Tests:**
- [ ] First window shows count = 1
- [ ] Creating second window updates count to 2
- [ ] Closing window decrements count
- [ ] All windows show synchronized count

---

## Test Specifications

### Test Suite 1: C++ Layer (Fix Build Issues)

#### Test 1.1: WindowCountListener builds and passes

**Given:** Fixed CreateEvent naming conflict
**When:** `cmake --build build --config Release`
**Then:** window_count_listener_test.exe builds and runs

#### Test 1.2: DartPortManager builds with mocked Dart API

**Given:** Mock dart_api_dl.h header created
**When:** `cmake --build build --config Release`
**Then:** dart_port_manager_test.exe builds and runs

---

### Test Suite 2: Dart FFI Layer

#### Test 2.1: FFI initialization succeeds

```dart
test('InitDartApiDL returns 0 on success', () {
  final ffi = WindowManagerFFI();
  expect(ffi.initializeDartApi(), equals(0));
});
```

#### Test 2.2: Port registration works

```dart
test('registerWindowCountPort returns true', () {
  final ffi = WindowManagerFFI();
  ffi.initializeDartApi();

  final port = ReceivePort();
  expect(ffi.registerWindowCountPort(port.sendPort), isTrue);

  port.close();
});
```

#### Test 2.3: ReceivePort receives messages

```dart
test('ReceivePort receives window count updates', () async {
  final ffi = WindowManagerFFI();
  ffi.initializeDartApi();

  final port = ReceivePort();
  ffi.registerWindowCountPort(port.sendPort);

  // Trigger count change somehow
  // ...

  final message = await port.first.timeout(Duration(seconds: 5));
  expect(message, isA<int>());
});
```

---

### Test Suite 3: Refactored Components

#### Test 3.1: NativeBridge integration

```dart
group('WindowManagerNativeBridge', () {
  test('initialize returns true', () async {
    final bridge = WindowManagerNativeBridge();
    expect(await bridge.initialize(), isTrue);
    await bridge.dispose();
  });

  test('windowCountStream emits initial value', () async {
    final bridge = WindowManagerNativeBridge();
    await bridge.initialize();

    final count = await bridge.windowCountStream.first;
    expect(count, greaterThanOrEqualTo(1));

    await bridge.dispose();
  });
});
```

#### Test 3.2: WindowCountService unit tests

```dart
group('WindowCountService', () {
  late MockNativeBridge mockBridge;
  late WindowCountService service;

  setUp(() {
    mockBridge = MockNativeBridge();
    service = WindowCountService(mockBridge);
  });

  test('exposes count stream from bridge', () {
    when(mockBridge.windowCountStream).thenAnswer(
      (_) => Stream.value(5)
    );

    expect(service.countStream, emits(5));
  });
});
```

#### Test 3.3: WindowCounterScreen widget tests

```dart
testWidgets('displays count from service', (tester) async {
  final mockService = MockWindowCountService();
  when(mockService.countStream).thenAnswer(
    (_) => Stream.value(3)
  );

  await tester.pumpWidget(
    MaterialApp(
      home: WindowCounterScreen(
        countService: mockService,
        windowManager: MockWindowManagerService(),
      ),
    ),
  );

  await tester.pumpAndSettle();

  expect(find.text('3'), findsOneWidget);
});
```

---

## Implementation Order

### Immediate (Today)

1. **Fix WindowCountListener build** - Rename CreateEvent method
2. **Create Dart API mock** - Enable DartPortManager tests
3. **Run all C++ tests** - Verify layers 1-3 work

### Short-term

4. **Add FFI unit tests** - Test Dart-C++ bridge
5. **Extract NativeBridge** - First refactoring step
6. **Add NativeBridge tests** - Cover the bridge layer

### Medium-term

7. **Extract services** - WindowCountService, WindowManagerService
8. **Add service tests** - Unit test business logic
9. **Refactor UI** - Clean separation of concerns

### Final

10. **Integration tests** - End-to-end multi-window test
11. **Fix root cause** - Whatever the tests reveal
12. **Documentation** - Update guides and README

---

## Success Criteria

- [ ] All C++ tests build and pass (4 test executables)
- [ ] All Dart unit tests pass
- [ ] All widget tests pass
- [ ] Integration test shows synchronized window count
- [ ] Code coverage > 80% for new code
- [ ] No monolithic files > 200 lines
- [ ] Clear separation between layers

---

## Commands

### Run C++ Tests
```bash
cd windows/test
cmake -B build -S .
cmake --build build --config Release
cd build && ctest --output-on-failure -C Release
```

### Run Dart Tests
```bash
flutter test
flutter test test/unit/
flutter test test/widget/
flutter test integration_test/
```

### Run All Tests
```bash
# Use TDD command
/tdd-test
```

---

**Created:** 2025-11-29
**Last Modified:** 2025-11-29
