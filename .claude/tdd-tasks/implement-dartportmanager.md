# TDD Task: Implement DartPortManager

**Status:** Not Started
**Created:** 2025-11-28
**Last Updated:** 2025-11-28

---

## Feature Description

Implement Layer 3 of the event-driven multi-window IPC architecture: **DartPortManager** enables C++ native code to communicate with Dart isolates using the Dart C API (`Dart_PostCObject`). When WindowCountListener detects window count changes via Windows Events, DartPortManager broadcasts these updates to all registered Dart isolates, enabling instant UI updates without polling.

**Architecture Position:**
```
Layer 1: SharedMemoryManager ✅ → Layer 2: WindowCountListener ✅ → Layer 3: DartPortManager 🔲
```

---

## Requirements

### Functional Requirements
- [ ] **Port Registration**: Provide FFI-exported function `RegisterWindowCountPort(Dart_Port)` callable from Dart
- [ ] **Port Unregistration**: Provide FFI-exported function `UnregisterWindowCountPort(Dart_Port)` callable from Dart
- [ ] **Thread-Safe Storage**: Maintain `std::vector<Dart_Port>` with `std::mutex` protection
- [ ] **Broadcast Notifications**: Implement `NotifyWindowCountChanged(LONG)` to send updates to all ports
- [ ] **Dart_PostCObject Integration**: Use Dart C API to post `Dart_CObject` messages
- [ ] **WindowCountListener Integration**: Set callback in WindowCountListener to call DartPortManager

### Non-Functional Requirements
- [ ] **Thread Safety**: All port operations protected with `std::lock_guard<std::mutex>`
- [ ] **Code Style**: Follow Google C++ Style Guide (`.claude/cpp-style-guide.md`)
- [ ] **Documentation**: Comprehensive comments explaining Dart C API usage
- [ ] **Build**: Compiles without errors via `flutter build windows`

---

## Test Specifications

### Test 1: DartPortManager Structure Compiles

**Description:** Verify DartPortManager class declaration and implementation files compile successfully with all required methods.

**Given:**
- Empty DartPortManager class with header and implementation files
- CMakeLists.txt updated to include dart_port_manager.cpp
- `#include <dart_api.h>` for Dart C API

**When:**
- Run `flutter build windows`

**Then:**
- Build succeeds without compilation errors
- DartPortManager.h declares: `RegisterPort()`, `UnregisterPort()`, `NotifyWindowCountChanged()`
- DartPortManager.cpp provides stub implementations (return false/do nothing)
- FFI exports declared: `extern "C" __declspec(dllexport)` for registration functions

**Test Code Location:** Manual build verification

---

### Test 2: Port Registration via FFI

**Description:** Verify Dart can call FFI-exported `RegisterWindowCountPort()` and port is stored in DartPortManager.

**Given:**
- DartPortManager initialized
- Empty ports_ vector
- Dart FFI bindings created in `lib/window_manager_ffi.dart`

**When:**
- Dart code calls: `registerWindowCountPort(sendPort.nativePort)`
- FFI invokes: `RegisterWindowCountPort(Dart_Port port)`

**Then:**
- Port is added to `ports_` vector
- Console output shows: `"Dart port registered: [port-id]"`
- Function returns `true`
- Thread-safe operation (mutex acquired during vector modification)

**Test Code Location:** `lib/window_manager_ffi.dart` + console verification

---

### Test 3: Port Unregistration via FFI

**Description:** Verify Dart can call FFI-exported `UnregisterWindowCountPort()` and port is removed from DartPortManager.

**Given:**
- DartPortManager has registered port (from Test 2)
- ports_ vector contains at least one port

**When:**
- Dart code calls: `unregisterWindowCountPort(sendPort.nativePort)`
- FFI invokes: `UnregisterWindowCountPort(Dart_Port port)`

**Then:**
- Port is removed from `ports_` vector using `std::remove_if` + `erase`
- Console output shows: `"Dart port unregistered: [port-id]"`
- Function returns `true`
- Thread-safe operation (mutex acquired during vector modification)

**Test Code Location:** `lib/window_manager_ffi.dart` + console verification

---

### Test 4: Broadcast Notification to All Ports

**Description:** Verify `NotifyWindowCountChanged()` sends Dart_CObject messages to all registered ports via `Dart_PostCObject()`.

**Given:**
- DartPortManager has 2 registered ports (simulating 2 windows)
- WindowCountListener callback is set to DartPortManager::NotifyWindowCountChanged

**When:**
- WindowCountListener detects event (window count changed to 3)
- Callback invokes: `DartPortManager::NotifyWindowCountChanged(3)`

**Then:**
- Console output shows: `"Notifying 2 Dart ports of window count: 3"`
- For each port, console shows: `"Posted to Dart port: [port-id]"`
- Dart_CObject created with type `Dart_CObject_kInt64` and value 3
- `Dart_PostCObject(port, &message)` called for each port
- Thread-safe operation (mutex acquired during iteration)

**Test Code Location:** Console output verification

---

### Test 5: Dart ReceivePort Receives Messages

**Description:** Verify Dart isolates receive window count updates via ReceivePort.listen().

**Given:**
- Dart creates ReceivePort and registers SendPort with FFI
- Port registered in DartPortManager (from Test 2)
- ReceivePort.listen() callback set up to print message

**When:**
- C++ calls `NotifyWindowCountChanged(5)`
- Dart_PostCObject sends message to ReceivePort

**Then:**
- Dart ReceivePort.listen() callback fires
- Dart receives message: `5` (as int)
- Dart console output shows: `"Received window count update: 5"`
- setState() can be called to update UI

**Test Code Location:** `lib/main.dart` with ReceivePort integration

---

### Test 6: WindowCountListener Integration

**Description:** Verify WindowCountListener calls DartPortManager callback when event is signaled.

**Given:**
- WindowCountListener initialized with callback set to DartPortManager::NotifyWindowCountChanged
- SharedMemoryManager initialized
- DartPortManager has 1 registered port

**When:**
- Window created → SharedMemoryManager::IncrementWindowCount() → SetEvent()
- WindowCountListener wakes from WaitForSingleObject
- Callback fires with new count

**Then:**
- DartPortManager::NotifyWindowCountChanged() is called
- Dart receives window count update
- Console shows complete event flow:
  ```
  Window count incremented: 1
  Window count changed notification received  ← Layer 2
  Notifying 1 Dart ports of window count: 1  ← Layer 3
  Posted to Dart port: [port-id]             ← Layer 3
  ```

**Test Code Location:** Integration test with console verification

---

### Test 7: Multi-Window Synchronization

**Description:** Verify all windows receive synchronized updates when any window is created/destroyed.

**Given:**
- 3 Flutter windows running simultaneously
- Each window has registered its ReceivePort SendPort
- DartPortManager has 3 registered ports

**When:**
- Window 4 is created → count = 4
- SharedMemoryManager increments → SetEvent() → WindowCountListener → DartPortManager

**Then:**
- All 3 existing windows receive update simultaneously
- Each window's ReceivePort.listen() fires with value `4`
- All windows update UI to show "Active Windows: 4"
- Latency < 10ms (event-driven, no polling)
- CPU usage < 1% (no polling timers)

**Test Code Location:** Manual multi-window test with UI verification

---

### Test 8: Error Handling - Invalid Port

**Description:** Handle `Dart_PostCObject` failures gracefully without crashing.

**Given:**
- DartPortManager has 2 registered ports
- Port 1 is valid, Port 2 is invalid/stale

**When:**
- `NotifyWindowCountChanged(7)` is called
- `Dart_PostCObject` fails for Port 2 (returns false)

**Then:**
- Error logged: `"Failed to post to Dart port: [port-id]"`
- Broadcast continues to remaining ports
- Port 1 still receives message successfully
- No crash or exception
- Consider removing invalid port from registry (future enhancement)

**Test Code Location:** Simulated error condition with console verification

---

## Implementation Requirements

### File Structure
**C++ Native:**
- `windows/runner/dart_port_manager.h` - Class declaration + FFI exports
- `windows/runner/dart_port_manager.cpp` - Implementation with Dart_PostCObject
- `windows/runner/window_count_listener.h` - Modified to add DartPortManager callback
- `windows/runner/window_count_listener.cpp` - Modified to call callback with count
- `windows/runner/flutter_window.h` - Modified to add DartPortManager member
- `windows/runner/flutter_window.cpp` - Modified to initialize DartPortManager
- `windows/runner/CMakeLists.txt` - Add dart_port_manager.cpp to build

**Dart FFI:**
- `lib/window_manager_ffi.dart` - FFI bindings and ReceivePort integration
- `lib/main.dart` - Modified to create ReceivePort and display window count

### Framework-Specific Information

**Test Framework:** Manual verification via console output + Flutter integration test
**Build Command:** `flutter build windows` or `flutter run -d windows`
**Verification Method:** Console output + Dart print statements + UI updates

### Class Signature

```cpp
// dart_port_manager.h
#ifndef RUNNER_DART_PORT_MANAGER_H_
#define RUNNER_DART_PORT_MANAGER_H_

#include <dart_api.h>
#include <windows.h>

#include <mutex>
#include <vector>

/// Manages communication from C++ to Dart isolates via Dart C API.
///
/// DartPortManager maintains a registry of Dart SendPort handles and
/// broadcasts window count updates to all registered Dart isolates using
/// Dart_PostCObject(). This enables zero-latency UI updates without polling.
///
/// Thread Safety: All public methods are thread-safe using std::mutex.
class DartPortManager {
 public:
  DartPortManager();
  ~DartPortManager();

  /// Registers a Dart SendPort for receiving window count notifications.
  ///
  /// Thread-safe: Can be called from FFI thread.
  ///
  /// @param port Dart_Port obtained from SendPort.nativePort in Dart
  /// @return true if registration successful
  bool RegisterPort(Dart_Port port);

  /// Unregisters a previously registered Dart SendPort.
  ///
  /// Thread-safe: Can be called from FFI thread.
  ///
  /// @param port Dart_Port to remove from registry
  /// @return true if port was found and removed
  bool UnregisterPort(Dart_Port port);

  /// Broadcasts window count update to all registered Dart ports.
  ///
  /// Creates Dart_CObject message and calls Dart_PostCObject() for each
  /// registered port. This is called from WindowCountListener callback
  /// when the event is signaled.
  ///
  /// Thread-safe: Can be called from background thread.
  ///
  /// @param new_count Current window count from SharedMemoryManager
  void NotifyWindowCountChanged(LONG new_count);

 private:
  std::vector<Dart_Port> ports_;  // Registered Dart SendPort handles
  std::mutex ports_mutex_;        // Protects ports_ vector
};

// FFI Exports for Dart binding
extern "C" {

/// FFI export: Register Dart SendPort for window count notifications.
///
/// Called from Dart via FFI:
///   registerWindowCountPort(sendPort.nativePort);
///
/// @param port Dart_Port from SendPort.nativePort
/// @return true if registration successful
__declspec(dllexport) bool RegisterWindowCountPort(Dart_Port port);

/// FFI export: Unregister Dart SendPort.
///
/// Called from Dart via FFI when window is destroyed:
///   unregisterWindowCountPort(sendPort.nativePort);
///
/// @param port Dart_Port to unregister
/// @return true if port was found and removed
__declspec(dllexport) bool UnregisterWindowCountPort(Dart_Port port);

}  // extern "C"

#endif  // RUNNER_DART_PORT_MANAGER_H_
```

### Dependencies Required
- [x] **Dart C API**: `#include <dart_api.h>` (provided by Flutter SDK)
- [x] **C++ STL**: `#include <vector>`, `#include <mutex>`, `#include <algorithm>`
- [x] **Windows API**: `#include <windows.h>` (for LONG type)
- [ ] **DartPortManager Instance**: Create global or FlutterWindow member

### Edge Cases to Handle
- [ ] **Empty Port Registry**: NotifyWindowCountChanged with no registered ports (no-op)
- [ ] **Invalid Dart_Port**: Dart_PostCObject returns false (log error, continue)
- [ ] **Duplicate Registration**: Same port registered twice (prevent or allow?)
- [ ] **Unregister Non-Existent Port**: Returns false gracefully
- [ ] **Concurrent Registration**: Multiple threads registering simultaneously (mutex protection)
- [ ] **Shutdown Cleanup**: Ports unregistered before DartPortManager destroyed

---

## Acceptance Criteria

- [ ] All test specifications pass (Tests 1-8)
- [ ] Code follows Google C++ Style Guide
- [ ] Build succeeds without warnings: `flutter build windows`
- [ ] Console output shows successful port registration and notifications
- [ ] Dart ReceivePort receives window count updates
- [ ] Multi-window test shows synchronized updates across all windows
- [ ] Thread-safe operations verified (no crashes with rapid operations)
- [ ] Comprehensive documentation added to all methods
- [ ] FFI exports callable from Dart

---

## Implementation Notes

### Architectural Decisions

**Singleton vs Member Variable:**
- Use DartPortManager as member of FlutterWindow (one per window)
- Alternative: Global singleton DartPortManager
- Decision: Member variable for cleaner ownership and lifecycle

**Mutex Strategy:**
- Use `std::lock_guard<std::mutex>` for RAII safety
- Ensures mutex released even if exception thrown
- Minimal lock duration (only during vector modification/iteration)

**Dart_CObject Type:**
- Use `Dart_CObject_kInt64` for LONG values
- Cross-platform safe (32-bit and 64-bit Windows)
- Dart receives as `int` type

**Error Handling:**
- Log errors but continue broadcasting to other ports
- Don't crash entire application if one port is invalid
- Future: Consider removing invalid ports from registry

### Potential Refactoring Opportunities

**After GREEN Phase:**
- Extract port validation logic to separate method
- Add performance metrics (track broadcast time)
- Create typed message wrapper class for different notification types
- Add structured message format (e.g., JSON or custom protocol)

**Future Enhancements:**
- Support multiple data types beyond window count
- Add message queuing for high-frequency updates
- Implement port health checking (remove stale ports automatically)

---

## Test Results Tracking

### Iteration 1 (RED Phase)

**Date:** 2025-11-28
**Status:** Structure created, stubs implemented
**Tests:**
- Test 1: DartPortManager compiles ✅
- Tests 2-8: Not yet implemented (stubs return false/do nothing)

**Expected Console Output:**
```
(No output - stubs do nothing)
```

### Iteration 2 (GREEN Phase)

**Date:** [TBD]
**Status:** Implementation complete, all tests pass
**Tests:**
- Test 1: Build succeeds ✅
- Test 2: Port registration works ✅
- Test 3: Port unregistration works ✅
- Test 4: Broadcast notifications work ✅
- Test 5: Dart ReceivePort receives messages ✅
- Test 6: WindowCountListener integration works ✅
- Test 7: Multi-window synchronization works ✅
- Test 8: Error handling graceful ✅

**Expected Console Output:**
```
Dart port registered: 12345
Window count incremented: 1
Window count changed notification received
Notifying 1 Dart ports of window count: 1
Posted to Dart port: 12345
```

**Expected Dart Output:**
```
Received window count update: 1
```

### Iteration 3 (REFACTOR Phase)

**Date:** [TBD]
**Status:** Code quality improvements, documentation complete
**Refactoring Done:**
- Added comprehensive method documentation
- Explained Dart C API usage patterns
- Added error handling for invalid ports
- Optimized mutex lock duration
- Added debug logging

**Tests Status:** All passing ✅

---

## Related Issues/Features

**Depends On:**
- ✅ Layer 1: SharedMemoryManager (provides window count source)
- ✅ Layer 2: WindowCountListener (triggers notifications via callback)

**Blocks:**
- Full multi-window UI synchronization
- Real-time state sharing across windows
- Future shared state features (themes, settings, etc.)

**Related Files:**
- `SHARED_MEMORY_COUNTER_GUIDE.md` - Part 2: Dart Implementation section
- `.claude/cpp-style-guide.md` - C++ coding standards
- `.claude/flutter-dart-rules.md` - Dart FFI best practices

---

## Checklist Before Implementation

- [x] I understand the requirements completely
- [x] I've reviewed Layer 1 and Layer 2 implementations
- [x] I've identified all test cases needed (Tests 1-8)
- [x] I have a plan for the implementation approach (Dart C API + std::vector)
- [x] I'm ready to start with the Red phase (creating structure)
- [x] I'm on the correct branch (main)

---

## Command References

### Start TDD Implementation
```bash
/tdd-implement .claude/tdd-tasks/implement-dartportmanager.md
```

### Build During Development
```bash
flutter build windows
flutter run -d windows
```

### Manual Test with Multiple Windows
```bash
# Terminal 1
flutter run -d windows

# Terminal 2 (after first window opens)
flutter run -d windows

# Observe synchronized window count updates
```

---

**Created:** 2025-11-28
**Last Modified:** 2025-11-28
