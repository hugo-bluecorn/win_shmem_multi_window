# Feature Notes: Implement DartPortManager

**Created:** 2025-11-28
**Status:** Planning

---

## Overview

### Purpose
Layer 3 of the event-driven multi-window IPC architecture: DartPortManager bridges C++ native code to Dart isolates using Dart FFI and the Dart C API. When WindowCountListener (Layer 2) detects window count changes via Windows Events, DartPortManager broadcasts these changes to all registered Dart isolates using `Dart_PostCObject()`.

This enables zero-latency UI updates in Flutter without polling, completing the event flow:
```
SharedMemoryManager (Layer 1) → WindowCountListener (Layer 2) → DartPortManager (Layer 3) → Dart ReceivePort → setState()
```

### Use Cases
- Synchronize window count across all Flutter windows instantly (<10ms latency)
- Enable real-time multi-window state updates without polling
- Foundation for future shared state synchronization (settings, themes, user data)

### Context
**Prerequisites:**
- ✅ Layer 1 (SharedMemoryManager): Shared memory with atomic operations
- ✅ Layer 2 (WindowCountListener): Event-driven notifications

**Technology Stack:**
- Dart C API (`Dart_PostCObject`, `Dart_Port`, `Dart_CObject`)
- C++ STL thread-safe collections (`std::vector`, `std::mutex`, `std::lock_guard`)
- Dart FFI (`dart:ffi`, `ReceivePort`, `SendPort`)

---

## Requirements Analysis

### Functional Requirements
1. **Port Registration**: Provide FFI-exported function to register Dart SendPort handles
2. **Port Unregistration**: Provide FFI-exported function to unregister Dart SendPort handles
3. **Broadcast Notifications**: Send window count updates to all registered Dart ports
4. **Thread-Safe Storage**: Maintain thread-safe collection of registered ports
5. **Integration with WindowCountListener**: Receive callbacks from Layer 2 event listener
6. **Dart Message Format**: Use Dart_CObject to send LONG values to Dart

### Non-Functional Requirements
- **Thread Safety**: All port operations must be protected with mutex
- **Zero Memory Leaks**: Proper cleanup of registered ports
- **Performance**: Minimal overhead when broadcasting (<1ms per port)
- **Robustness**: Handle invalid/stale ports gracefully
- **Code Quality**: Follow Google C++ Style Guide (`.claude/cpp-style-guide.md`)
- **Documentation**: Comprehensive comments explaining Dart API usage

### Integration Points
- **Layer 2 (WindowCountListener)**: Receives callbacks when events are signaled
- **Dart FFI Layer**: Exports registration functions to Dart
- **Flutter UI**: Dart isolates receive updates via ReceivePort

---

## Implementation Details

### Architectural Approach

**DartPortManager Class:**
```
┌─────────────────────────────────────┐
│ DartPortManager                     │
├─────────────────────────────────────┤
│ - std::vector<Dart_Port> ports_    │
│ - std::mutex ports_mutex_           │
├─────────────────────────────────────┤
│ + RegisterPort(Dart_Port)           │
│ + UnregisterPort(Dart_Port)         │
│ + NotifyWindowCountChanged(LONG)    │
│ - BroadcastToAllPorts(Dart_CObject*)│
└─────────────────────────────────────┘
```

**Event Flow:**
```
Window Created/Destroyed
    ↓
SharedMemoryManager::IncrementWindowCount()
    ↓
SetEvent(update_event_)
    ↓
WindowCountListener::ListenerThreadFunction() [WaitForSingleObject returns]
    ↓
WindowCountListener calls callback_
    ↓
DartPortManager::NotifyWindowCountChanged(new_count)
    ↓
Dart_PostCObject() to all registered ports
    ↓
Dart ReceivePort.listen() receives message
    ↓
setState(() { windowCount = message; })
```

### Design Patterns
- **Singleton Pattern**: DartPortManager as global instance (or owned by FlutterWindow)
- **Observer Pattern**: Dart isolates register as observers via SendPort
- **RAII Pattern**: Mutex guards with std::lock_guard for exception safety

### File Structure
```
windows/runner/
├── dart_port_manager.h         # Class declaration + FFI exports
├── dart_port_manager.cpp       # Implementation with Dart C API
├── window_count_listener.h     # (Modified) Add callback to DartPortManager
├── window_count_listener.cpp   # (Modified) Call DartPortManager on event
├── flutter_window.h            # (Modified) Add DartPortManager member
├── flutter_window.cpp          # (Modified) Initialize DartPortManager
└── CMakeLists.txt              # (Modified) Add dart_port_manager.cpp

lib/
└── window_manager_ffi.dart     # FFI bindings + ReceivePort integration
```

### Naming Conventions
**C++ (Google Style Guide):**
- Class: `DartPortManager` (PascalCase)
- Methods: `RegisterPort()`, `NotifyWindowCountChanged()` (PascalCase public)
- Members: `ports_`, `ports_mutex_` (snake_case_ with trailing underscore)
- Constants: `kDefaultCapacity` (kCamelCase)

**Dart (Flutter Style):**
- Files: `window_manager_ffi.dart` (snake_case)
- Functions: `registerWindowCountPort()` (camelCase)
- Classes: `WindowManagerFFI` (PascalCase)

### Visual Design
```
┌──────────────────────────────────────────────────┐
│ Layer 3: Dart Communication Architecture        │
└──────────────────────────────────────────────────┘

C++ Thread (WindowCountListener)
    │
    │ Event signaled
    ↓
┌─────────────────────────────────┐
│ DartPortManager                 │
│  - ports_: [Port1, Port2, ...]  │ ← Thread-safe with std::mutex
│                                 │
│  NotifyWindowCountChanged(5)    │
└─────────────────────────────────┘
    │
    │ Dart_PostCObject() for each port
    ├───────────┬───────────┬──────────
    ↓           ↓           ↓
┌─────────┐ ┌─────────┐ ┌─────────┐
│ Window1 │ │ Window2 │ │ Window3 │
│ Dart    │ │ Dart    │ │ Dart    │
│ Isolate │ │ Isolate │ │ Isolate │
│         │ │         │ │         │
│ Receive │ │ Receive │ │ Receive │
│ Port    │ │ Port    │ │ Port    │
│ .listen │ │ .listen │ │ .listen │
└─────────┘ └─────────┘ └─────────┘
    │           │           │
    ↓ setState  ↓ setState  ↓ setState
  UI Update   UI Update   UI Update
```

---

## TDD Approach

### Test Strategy
Following TDD principles with Red-Green-Refactor cycle for C++ native code.

**Test Framework:** Manual testing via console output + Flutter integration test
**Build Command:** `flutter build windows` or `flutter run -d windows`
**Verification:** Console output + Dart ReceivePort message verification

### Red Phase - Tests to Write
Since this is C++ native code, tests are verified via:
1. **Build Success**: Code compiles without errors
2. **Registration Test**: FFI exports work from Dart
3. **Notification Test**: Dart ReceivePort receives messages
4. **Thread Safety**: Multiple windows register/unregister without crashes

**Goal:** Structure compiles but methods are stubs (return false/do nothing)

### Green Phase - Implementation
1. Implement `RegisterPort()` with thread-safe vector append
2. Implement `UnregisterPort()` with thread-safe vector removal
3. Implement `NotifyWindowCountChanged()` with Dart_PostCObject loop
4. Integrate callback into WindowCountListener
5. Create FFI exports for Dart binding

**Goal:** Dart receives window count updates via ReceivePort

### Refactor Phase - Code Quality
1. Add comprehensive documentation comments
2. Explain Dart C API usage and threading concerns
3. Add error handling for invalid ports
4. Optimize port lookup (if needed)
5. Ensure proper cleanup on shutdown

**Goal:** Production-ready, maintainable code

---

## Dependencies

### External Packages
- ✅ **Dart C API**: `<dart_api.h>` (included with Flutter SDK)
- ✅ **C++ STL**: `<vector>`, `<mutex>`, `<algorithm>`
- ✅ **Windows API**: Already available (no new dependencies)

### Internal Dependencies
- ✅ **SharedMemoryManager**: Layer 1 (provides window count source)
- ✅ **WindowCountListener**: Layer 2 (triggers notifications)
- 🔲 **Dart FFI Bindings**: Need to create `lib/window_manager_ffi.dart`

### Platform/Framework Requirements
- ✅ Flutter 3.10.1+ (current project version)
- ✅ Windows Desktop platform
- ✅ Dart 3.0+ (for FFI support)

---

## Testing Strategy

### Manual Testing via Console Output
**Test 1: Port Registration**
- Start Flutter app
- Console shows: "Dart port registered: [port-id]"

**Test 2: Notification Broadcast**
- Create window
- Console shows: "Notifying [N] Dart ports of window count: [count]"
- Console shows: "Posted to Dart port: [port-id]"

**Test 3: Dart Reception**
- Dart prints: "Received window count update: [count]"
- UI updates with new count

### Integration Tests
**Multi-Window Test:**
1. Launch window 1 → Count = 1 → All windows show "1"
2. Launch window 2 → Count = 2 → All windows show "2"
3. Close window 2 → Count = 1 → All windows show "1"
4. Close window 1 → Count = 0 → Clean shutdown

### Manual Testing Checklist
- [x] Test on Windows 10/11
- [ ] Test with 1 window (registration works)
- [ ] Test with 3 windows simultaneously (broadcast works)
- [ ] Test rapid window creation/destruction (thread safety)
- [ ] Verify zero memory leaks (port cleanup)
- [ ] Verify performance (<1ms broadcast time)

---

## Known Limitations / Trade-offs

### Limitations
- **Windows Only**: Dart C API integration specific to Windows runner (macOS/Linux need separate implementation)
- **No Automatic Unregistration**: Dart must explicitly unregister ports on window close (handled in Dart `dispose()`)

### Trade-offs Made
- **Global Port Manager**: Using single DartPortManager instance vs per-window managers (simpler, sufficient for use case)
- **Vector vs Set**: Using `std::vector<Dart_Port>` for simplicity; acceptable O(n) removal since N is small (<10 windows)

---

## Implementation Notes

### Key Decisions
- **Mutex Strategy**: Use `std::mutex` with `std::lock_guard` for RAII safety
- **Dart_CObject Type**: Use `kInt64` type for LONG values (cross-platform safe)
- **Error Handling**: Continue on `Dart_PostCObject` failure (don't crash if one port is invalid)
- **Callback Ownership**: WindowCountListener owns callback via `std::function`

### Future Improvements
- [ ] Add port validation (check if port is still valid before posting)
- [ ] Add performance metrics (track broadcast time)
- [ ] Support multiple data types (not just window count)
- [ ] Add structured message format (JSON or custom protocol)

### Potential Refactoring
- Extract port management to separate `PortRegistry` class if complexity grows
- Add typed message wrappers for different notification types
- Consider message queuing for high-frequency updates

---

## References

### Related Code
- **Layer 1**: `windows/runner/shared_memory_manager.{h,cpp}`
- **Layer 2**: `windows/runner/window_count_listener.{h,cpp}`
- **FFI Guide**: `SHARED_MEMORY_COUNTER_GUIDE.md` (Part 2: Dart Implementation)

### Documentation
- [Dart C API Reference](https://api.dart.dev/stable/dart-api/Dart_PostCObject.html)
- [Flutter FFI Guide](https://docs.flutter.dev/platform-integration/c-interop)
- [Google C++ Style Guide](.claude/cpp-style-guide.md)

### Issues/PRs
- Related to multi-window architecture design
- Completes Layer 3 of event-driven IPC system

---

## Acceptance Criteria

- [ ] DartPortManager class compiles successfully
- [ ] FFI exports callable from Dart
- [ ] Dart ReceivePort receives window count updates
- [ ] Multi-window test shows all windows synchronized
- [ ] No crashes with rapid window creation/destruction
- [ ] Code follows Google C++ Style Guide
- [ ] Comprehensive documentation added
- [ ] Console output shows successful notifications

---

## Session Log

### Session 1: Planning and Requirements
**Date:** 2025-11-28
**Summary:** Created feature notes and TDD task specification. Defined Layer 3 architecture integrating Dart C API with event-driven notifications.

---

## Next Steps

1. Create TDD task file: `.claude/tdd-tasks/implement-dartportmanager.md`
2. Review this feature notes document
3. Run `/tdd-implement` command to start TDD workflow
4. Follow Red-Green-Refactor cycle:
   - RED: Create structure (dart_port_manager.h/cpp stubs)
   - GREEN: Implement Dart_PostCObject integration
   - REFACTOR: Add documentation and error handling
5. Create Dart FFI bindings (`lib/window_manager_ffi.dart`)
6. Integration test with multiple windows
7. Submit commits following version-control.md

---

**Created:** 2025-11-28
**Last Updated:** 2025-11-28
**Status:** Planning
