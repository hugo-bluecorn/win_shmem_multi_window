# Feature Notes: Add UI Buttons for Programmatic Window Creation and Destruction

**Created:** 2025-11-29
**Status:** Planning

---

## Overview

### Purpose
Enable users to create and destroy application windows programmatically through UI buttons, completing the main application goal of multi-window management with real-time synchronization.

### Use Cases
- User clicks "Create New Window" button to spawn additional application windows on demand
- User clicks "Close This Window" button to programmatically close the current window
- All windows display real-time window count updates as windows are created/destroyed

### Context
The three-layer event-driven IPC architecture (SharedMemoryManager → WindowCountListener → DartPortManager) is complete and working. The backend properly tracks window count changes across processes. Now we need UI controls to trigger window creation and destruction.

---

## Requirements Analysis

### Functional Requirements
1. Add "Create New Window" button that spawns a new application instance
2. Add "Close This Window" button that closes the current window
3. Window creation must increment the shared window count
4. Window destruction must decrement the shared window count
5. All windows must receive real-time updates when count changes
6. Buttons must be clearly labeled and accessible

### Non-Functional Requirements
- Button clicks must be responsive
- Window destruction must properly clean up resources
- UI must follow Material Design 3 guidelines
- Code must follow Flutter/Dart style guide

### Integration Points
- Integrates with existing WindowCounterScreen
- Uses Process.start() for window creation
- Uses SystemNavigator.pop() for window destruction
- Relies on C++ FlutterWindow lifecycle (OnCreate/OnDestroy)

---

## Implementation Details

### Architectural Approach
**Development mode**: `Process.start('flutter', ['run', '-d', 'windows'])`
**Production mode**: `Process.start(Platform.resolvedExecutable, [])`
**Window close**: `SystemNavigator.pop()` triggers OnDestroy()

### File Structure
```
lib/
└── main.dart  (modify)
    ├── _createNewWindow()    (add)
    ├── _closeCurrentWindow() (add)
    └── build() → add buttons (modify)

test/
└── widget/
    └── window_controls_test.dart (create)
```

---

## TDD Approach

### Red Phase - Tests to Write
1. Verify "Create New Window" button exists
2. Verify "Close This Window" button exists
3. Verify buttons are tappable
4. Verify button labels

### Green Phase - Implementation
1. Add imports (dart:io, flutter/services)
2. Implement _createNewWindow()
3. Implement _closeCurrentWindow()
4. Add buttons to UI

### Refactor Phase
1. Add documentation
2. Ensure Flutter style compliance
3. Add error handling

---

## Future Enhancements
- [ ] Add loading indicators during window creation (2-5 second startup time)
- [ ] Add window positioning controls
- [ ] Add confirmation dialog before closing
- [ ] Add "Close All Windows" button

---

## Acceptance Criteria
- [ ] "Create New Window" button creates new instance
- [ ] Window count increments in all windows
- [ ] "Close This Window" button closes current window
- [ ] Window count decrements in all windows
- [ ] All widget tests pass
- [ ] Manual testing with 5+ windows successful

---

**Created:** 2025-11-29
**Status:** Planning
