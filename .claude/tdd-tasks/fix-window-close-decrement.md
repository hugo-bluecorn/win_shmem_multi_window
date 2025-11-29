# TDD Task: Fix Window Close to Properly Decrement Counter

## Problem Statement

When closing a window using the "Close This Window" button, the window count does **not** decrement. The counter only increments when new windows are created.

**Root Cause**: The Dart code uses `exit(0)` which terminates the process immediately, bypassing the Win32 message loop. This means `OnDestroy()` is never called, so `DecrementWindowCount()` never runs.

## Current Behavior

1. Window 1 opens → count = 1 ✅
2. Window 2 opens → count = 2 ✅
3. Window 2 closes → count stays 2 ❌ (should be 1)

## Expected Behavior

1. Window 1 opens → count = 1
2. Window 2 opens → count = 2
3. Window 2 closes → count = 1
4. All remaining windows receive notification and update their display

## Technical Approach

### Option A: FFI Function (Recommended)
Add a native FFI function `RequestWindowClose()` that:
1. Sends `WM_CLOSE` message to the window
2. This triggers proper Win32 cleanup path
3. `OnDestroy()` is called → `DecrementWindowCount()` runs
4. Event is signaled → other windows receive notification

### Option B: Platform Channel
Use Flutter's MethodChannel to communicate with native code.

## TDD Implementation Plan

### Phase 1: C++ Unit Tests (RED → GREEN)

#### Test 1.1: RequestWindowClose FFI function exists
```cpp
// Test that the function can be called without crashing
TEST(WindowCloseTest, RequestWindowClose_DoesNotCrash) {
  // Function should exist and be callable
  RequestWindowClose();
  // No crash = pass
}
```

#### Test 1.2: Window count decrements on proper close
```cpp
TEST(WindowCloseTest, ProperClose_DecrementsCount) {
  SharedMemoryManager manager;
  manager.Initialize();

  LONG initial = manager.IncrementWindowCount();  // count = 1
  manager.IncrementWindowCount();  // count = 2

  // Simulate proper window close
  manager.DecrementWindowCount();

  EXPECT_EQ(1, manager.GetWindowCount());
}
```

### Phase 2: Dart FFI Binding Tests (RED → GREEN)

#### Test 2.1: WindowManagerFFI has closeWindow method
```dart
test('WindowManagerFFI has closeWindow method', () {
  final ffi = WindowManagerFFI();
  // Method should exist (compile-time check)
  expect(ffi.closeWindow, isA<Function>());
});
```

### Phase 3: Integration Tests (RED → GREEN)

#### Test 3.1: Window close triggers count decrement
```dart
test('closing window decrements count', () async {
  final service = FFIWindowCountService();
  await service.initialize();

  final initialCount = await service.windowCountStream.first;

  // Launch a window
  await Process.start(exePath, [], mode: ProcessStartMode.detached);

  // Wait for count to increase
  final afterCreate = await service.windowCountStream
      .firstWhere((c) => c > initialCount);

  // Close the spawned window (via some mechanism)
  // ...

  // Wait for count to decrease
  final afterClose = await service.windowCountStream
      .firstWhere((c) => c < afterCreate)
      .timeout(Duration(seconds: 5));

  expect(afterClose, equals(initialCount));
});
```

## Files to Modify

### C++ Layer
1. `windows/runner/dart_port_manager.cpp` - Add `RequestWindowClose()` FFI export
2. `windows/runner/dart_port_manager.h` - Add declaration (optional)

### Dart Layer
1. `lib/window_manager_ffi.dart` - Add `closeWindow()` binding
2. `lib/main.dart` - Replace `exit(0)` with FFI call

### Tests
1. `windows/test/window_close_test.cpp` - C++ unit tests (new file)
2. `test/window_manager_ffi_test.dart` - Dart binding tests
3. `integration_test/window_close_integration_test.dart` - End-to-end tests

## Implementation Notes

### C++ Implementation
```cpp
extern "C" {
__declspec(dllexport) void RequestWindowClose() {
  HWND hwnd = GetActiveWindow();
  if (hwnd == nullptr) {
    hwnd = FindWindowA("FLUTTER_RUNNER_WIN32_WINDOW", nullptr);
  }
  if (hwnd != nullptr) {
    PostMessageA(hwnd, WM_CLOSE, 0, 0);
  } else {
    PostQuitMessage(0);
  }
}
}
```

### Dart Implementation
```dart
// In window_manager_ffi.dart
typedef RequestWindowCloseNative = Void Function();
typedef RequestWindowCloseDart = void Function();

class WindowManagerFFI {
  late final RequestWindowCloseDart closeWindow;

  WindowManagerFFI() {
    final dylib = DynamicLibrary.process();
    closeWindow = dylib.lookupFunction<
        RequestWindowCloseNative,
        RequestWindowCloseDart>('RequestWindowClose');
  }
}

// In main.dart
void _closeCurrentWindow() {
  debugPrint('Closing current window...');
  final ffi = WindowManagerFFI();
  ffi.closeWindow();  // Instead of exit(0)
}
```

## Success Criteria

1. All C++ unit tests pass
2. All Dart unit tests pass
3. Integration test verifies:
   - Creating window increments count
   - Closing window decrements count
   - Other windows receive notification and update display
4. Manual test confirms proper behavior

## Cycle Limit

Maximum 5 TDD cycles before reassessing approach.

## Related Files

- `windows/runner/flutter_window.cpp` - Contains `OnDestroy()` with `DecrementWindowCount()`
- `windows/runner/shared_memory_manager.cpp` - Contains `DecrementWindowCount()` implementation
- `lib/services/ffi_window_count_service.dart` - Receives count updates
