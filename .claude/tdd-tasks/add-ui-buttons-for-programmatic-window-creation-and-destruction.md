# TDD Task: Add UI Buttons for Programmatic Window Creation and Destruction

**Status:** Not Started
**Created:** 2025-11-29
**Last Updated:** 2025-11-29

---

## Feature Description

Add two UI buttons to the WindowCounterScreen that enable users to:
1. Create new application windows programmatically via "Create New Window" button
2. Close the current window programmatically via "Close This Window" button

This completes the main application goal of programmatic multi-window management with real-time synchronization.

---

## Requirements

### Functional Requirements
- [ ] "Create New Window" button spawns a new Flutter application instance
- [ ] "Close This Window" button closes the current window
- [ ] Window creation increments window count (via existing SharedMemoryManager)
- [ ] Window destruction decrements window count (via existing OnDestroy)
- [ ] All windows receive real-time count updates (via existing DartPortManager)
- [ ] Buttons are clearly labeled and accessible

### Non-Functional Requirements
- [ ] Buttons follow Material Design 3 guidelines
- [ ] Code follows Flutter/Dart style guide (.claude/flutter-dart-rules.md)
- [ ] Widget tests verify button existence and behavior
- [ ] Error handling with user feedback (SnackBar)

---

## Test Specifications

### Test 1: Create New Window Button Exists

**Description:** Verify that the "Create New Window" button is rendered in the UI

**Given:**
- WindowCounterScreen is built and rendered
- Widget tree is initialized

**When:**
- Test searches for button with text "Create New Window"

**Then:**
- Button widget is found in the widget tree
- Button is of type ElevatedButton or FilledButton

**Test Code Location:** `test/widget/window_controls_test.dart`

---

### Test 2: Close This Window Button Exists

**Description:** Verify that the "Close This Window" button is rendered in the UI

**Given:**
- WindowCounterScreen is built and rendered
- Widget tree is initialized

**When:**
- Test searches for button with text "Close This Window"

**Then:**
- Button widget is found in the widget tree
- Button is of type ElevatedButton or OutlinedButton

**Test Code Location:** `test/widget/window_controls_test.dart`

---

### Test 3: Create New Window Button Is Tappable

**Description:** Verify that the "Create New Window" button can be tapped

**Given:**
- WindowCounterScreen is rendered
- "Create New Window" button exists

**When:**
- Test taps the button using WidgetTester

**Then:**
- Button onPressed callback is triggered
- No exceptions are thrown

**Test Code Location:** `test/widget/window_controls_test.dart`

---

### Test 4: Close This Window Button Is Tappable

**Description:** Verify that the "Close This Window" button can be tapped

**Given:**
- WindowCounterScreen is rendered
- "Close This Window" button exists

**When:**
- Test taps the button using WidgetTester

**Then:**
- Button onPressed callback is triggered
- SystemNavigator.pop() is called (can be mocked)

**Test Code Location:** `test/widget/window_controls_test.dart`

---

### Test 5: Button Layout and Spacing

**Description:** Verify buttons are properly laid out with appropriate spacing

**Given:**
- WindowCounterScreen is rendered
- Both buttons exist in widget tree

**When:**
- Test examines widget tree structure

**Then:**
- Buttons are in a Column layout
- SizedBox spacing exists between buttons
- Buttons are centered in the layout

**Test Code Location:** `test/widget/window_controls_test.dart`

---

### Test 6: Error Handling for Failed Window Creation

**Description:** Verify error handling when Process.start() fails

**Given:**
- WindowCounterScreen is rendered
- Process.start() will throw an exception (mocked)

**When:**
- User taps "Create New Window" button

**Then:**
- Exception is caught
- SnackBar is displayed with error message
- Application does not crash

**Test Code Location:** `test/widget/window_controls_test.dart`

---

## Implementation Requirements

### File Structure
- **Source:** `lib/main.dart` (modify existing)
- **Tests:** `test/widget/window_controls_test.dart` (create new)

### Framework-Specific Information

**Test Framework:** `flutter_test`
**Test Command:** `flutter test test/widget/window_controls_test.dart`
**Analysis Command:** `flutter analyze`

### Function/Class Signatures

```dart
// lib/main.dart additions

import 'dart:io';
import 'package:flutter/services.dart';

class _WindowCounterScreenState extends State<WindowCounterScreen> {
  // ... existing code ...

  /// Creates a new application window by launching a new Flutter instance.
  ///
  /// In development mode, runs `flutter run -d windows`.
  /// In production mode, launches the built executable.
  ///
  /// Throws [ProcessException] if process creation fails.
  Future<void> _createNewWindow() async {
    // Implementation
  }

  /// Closes the current window.
  ///
  /// Calls SystemNavigator.pop() which triggers FlutterWindow::OnDestroy()
  /// for proper cleanup and window count decrement.
  void _closeCurrentWindow() {
    // Implementation
  }

  @override
  Widget build(BuildContext context) {
    // ... existing code ...
    // Add buttons in Column after status indicator
  }
}
```

### Dependencies Required
- [x] `dart:io` (for Process.start)
- [x] `package:flutter/services.dart` (for SystemNavigator)
- [x] No new pubspec.yaml dependencies needed

### Edge Cases to Handle
- [ ] Flutter command not found in PATH (development mode)
- [ ] Executable not found (production mode - unlikely)
- [ ] Process.start() throws exception
- [ ] User taps button multiple times rapidly
- [ ] Window destruction while receiving count updates

---

## Acceptance Criteria

- [ ] All widget tests pass (`flutter test test/widget/window_controls_test.dart`)
- [ ] Code follows Flutter/Dart style guidelines
- [ ] No linting errors (`flutter analyze`)
- [ ] "Create New Window" button creates new application instance
- [ ] "Close This Window" button closes current window
- [ ] Window count synchronization works with new UI controls
- [ ] Manual testing with 5+ windows successful
- [ ] Error handling tested (disconnect flutter command)
- [ ] Documentation updated (inline comments)

---

## Implementation Notes

### Architectural Decisions

**Why Process.start() instead of FFI to Win32 CreateWindow?**
- Simpler implementation that works with Flutter's architecture
- Separate engines per window (our current approach)
- No need for complex Win32 FFI bindings
- Cross-platform compatible approach

**Why SystemNavigator.pop() instead of exit(0)?**
- Proper Flutter lifecycle management
- Triggers OnDestroy() for cleanup
- Decrements window count via shared memory
- Graceful shutdown

**Development vs Production Detection:**
```dart
final bool isDebugMode = !const bool.fromEnvironment('dart.vm.product');
```
This compile-time constant distinguishes debug/release builds.

### Potential Refactoring Opportunities

After tests pass:
- Extract button widgets to separate components if adding more controls
- Create WindowControlService if window management becomes more complex
- Add ButtonStyle theming for consistent appearance

---

## Test Results Tracking

### Iteration 1 (RED Phase)

**Date:** [To be filled]
**Status:** Tests written and confirmed failing
**Failing Tests:** 6/6 expected to fail
**Notes:**
- Button widgets don't exist yet
- Methods _createNewWindow() and _closeCurrentWindow() not implemented

### Iteration 2 (GREEN Phase)

**Date:** [To be filled]
**Tests Passed:** 0/6 → 6/6 ✅
**Status:** Implementation complete and tests passing
**Notes:**
- Added imports
- Implemented window creation/destruction methods
- Added buttons to UI layout

### Iteration 3 (REFACTOR Phase)

**Date:** [To be filled]
**Tests Status:** All passing ✅
**Refactoring Done:**
- Added comprehensive documentation
- Improved error messages
- Verified style compliance
**Notes:** Final code quality improvements

---

## Related Issues/Features

- Depends on: SharedMemoryManager (Layer 1) ✅ Complete
- Depends on: WindowCountListener (Layer 2) ✅ Complete
- Depends on: DartPortManager (Layer 3) ✅ Complete
- Depends on: Dart FFI integration ✅ Complete
- Blocks: UI feedback enhancements (progress indicators) - Future task
- Blocks: Window positioning controls - Future task

---

## Checklist Before Implementation

- [x] I understand the requirements completely
- [x] I've reviewed existing WindowCounterScreen code
- [x] I've identified all test cases needed
- [x] I have a plan for the implementation approach
- [x] I'm ready to start with the Red phase (writing tests)
- [x] I'm on the correct branch (main)

---

## Command References

### Start TDD Implementation
```bash
/tdd-implement .claude/tdd-tasks/add-ui-buttons-for-programmatic-window-creation-and-destruction.md
```

### Run Tests During Development
```bash
/tdd-test test/widget/window_controls_test.dart
```

### Manual Testing
```bash
# Clean and rebuild
flutter clean && flutter pub get

# Run first instance
flutter run -d windows

# In the app UI:
# 1. Click "Create New Window" - verify new window appears
# 2. Verify window count increments in both windows
# 3. Click "Close This Window" in one window
# 4. Verify window count decrements in remaining window
```

---

**Created:** 2025-11-29
**Last Modified:** 2025-11-29
