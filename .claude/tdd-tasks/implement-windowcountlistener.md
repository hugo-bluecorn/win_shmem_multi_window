# TDD Task: Implement WindowCountListener

**Status:** Not Started
**Created:** 2025-11-28
**Last Updated:** 2025-11-28

---

## Feature Description

Implement WindowCountListener class to provide event-driven notifications when the window count changes. This is Layer 2 of the three-layer event-driven multi-window architecture, eliminating polling overhead and achieving zero-latency updates with <1% CPU usage.

**Reference:** `SHARED_MEMORY_COUNTER_GUIDE.md` - Part 1 (WindowCountListener section)

---

## Requirements

### Functional Requirements

- [ ] Create Windows Event "Local\FlutterWindowCountChanged" (auto-reset)
- [ ] Start background thread waiting on event with WaitForSingleObject
- [ ] Wake thread when event is signaled
- [ ] Execute optional callback when window count changes
- [ ] Provide Start() and Stop() lifecycle methods
- [ ] Clean thread shutdown on destruction (RAII pattern)
- [ ] Handle event signaling from SharedMemoryManager

### Non-Functional Requirements

- [ ] Follow `.claude/cpp-style-guide.md` conventions
- [ ] Thread-safe start/stop operations
- [ ] Zero CPU usage when idle (event-driven)
- [ ] <10ms notification latency
- [ ] No thread leaks or race conditions
- [ ] Proper error checking for Windows API calls

---

## Test Specifications

### Test 1: Build Verification

**Description:** Code compiles without errors

**Given:**
- WindowCountListener class header and implementation created
- SharedMemoryManager updated to signal event
- CMakeLists.txt updated

**When:**
- Run `flutter clean && flutter build windows`

**Then:**
- Build completes successfully with no compilation errors

**Test Location:** Manual - command line build

---

### Test 2: Event Creation

**Description:** Windows Event is created successfully

**Given:**
- WindowCountListener instance created

**When:**
- Constructor creates Windows Event

**Then:**
- Event handle is valid (not nullptr)
- Event name: "Local\FlutterWindowCountChanged"
- Event type: Auto-reset (resets automatically after WaitForSingleObject returns)
- Console shows: "Window count listener event created"

**Test Location:** Manual - console output verification

---

### Test 3: Background Thread Start

**Description:** Background thread starts and waits on event

**Given:**
- WindowCountListener created and event initialized

**When:**
- Start() method called

**Then:**
- Background thread spawns
- Thread waits on WaitForSingleObject (blocking, zero CPU)
- Console shows: "WindowCountListener started"
- CPU usage remains <1%

**Test Location:** Manual - console + Task Manager CPU verification

---

### Test 4: Event Signaling and Wake

**Description:** Thread wakes when event is signaled

**Given:**
- WindowCountListener running with background thread waiting
- SharedMemoryManager configured to signal event

**When:**
- Window count incremented (SharedMemoryManager calls SetEvent)

**Then:**
- WaitForSingleObject returns (thread wakes up)
- Console shows: "Window count changed notification"
- Thread loops back to waiting
- Total latency <10ms

**Test Location:** Manual - console output with timestamp verification

---

### Test 5: Multiple Windows Trigger Events

**Description:** Multiple processes can signal same event

**Given:**
- Two windows running with WindowCountListener
- Both listening on same event

**When:**
- First window increments count → SetEvent
- Second window increments count → SetEvent

**Then:**
- Both listeners wake up and receive notification
- Console shows notifications from both processes
- No missed notifications

**Test Location:** Manual - run two instances, observe console

---

### Test 6: Clean Thread Shutdown

**Description:** Background thread stops cleanly

**Given:**
- WindowCountListener running with active background thread

**When:**
- Stop() method called or destructor invoked

**Then:**
- is_running_ flag set to false
- Thread exits WaitForSingleObject loop
- std::thread::join() completes
- Console shows: "WindowCountListener stopped"
- No thread leaks (verify with Task Manager)

**Test Location:** Manual - console + Task Manager thread count

---

### Test 7: RAII Pattern Verification

**Description:** Resources cleaned up automatically

**Given:**
- WindowCountListener instance active

**When:**
- Object goes out of scope or destructor called

**Then:**
- Stop() called automatically
- Thread joined
- Event handle closed with CloseHandle
- No resource leaks

**Test Location:** Manual - verify with Process Explorer handles

---

## Implementation Requirements

### File Structure

**Source Files:**
- `windows/runner/window_count_listener.h` - Class declaration
- `windows/runner/window_count_listener.cpp` - Implementation

**Modified Files:**
- `windows/runner/shared_memory_manager.h` - Add event handle member
- `windows/runner/shared_memory_manager.cpp` - Signal event on count change
- `windows/runner/CMakeLists.txt` - Add window_count_listener.cpp

### Class Signature (window_count_listener.h)

```cpp
#ifndef RUNNER_WINDOW_COUNT_LISTENER_H_
#define RUNNER_WINDOW_COUNT_LISTENER_H_

#include <windows.h>

#include <atomic>
#include <functional>
#include <thread>

// Callback function type for window count change notifications
using WindowCountCallback = std::function<void(LONG new_count)>;

// Listens for window count changes via Windows Event objects.
// Runs background thread waiting on event with zero CPU overhead.
class WindowCountListener {
 public:
  WindowCountListener();
  ~WindowCountListener();

  // Starts background listener thread.
  // Returns true on success, false on error.
  bool Start();

  // Stops background listener thread.
  void Stop();

  // Sets callback function to execute when window count changes.
  void SetCallback(WindowCountCallback callback);

  // Returns true if listener is currently running.
  bool IsRunning() const;

 private:
  // Background thread function that waits on event.
  void ListenerThreadFunction();

  // Creates Windows Event object.
  bool CreateEvent();

  // Cleans up event handle and thread.
  void Cleanup();

  HANDLE update_event_;                  // Event signaled on count change
  std::thread listener_thread_;          // Background listener thread
  std::atomic<bool> is_running_;         // Thread running flag
  WindowCountCallback callback_;         // Optional notification callback
};

#endif  // RUNNER_WINDOW_COUNT_LISTENER_H_
```

### SharedMemoryManager Updates

**Add to shared_memory_manager.h:**
```cpp
// Add member variable
HANDLE update_event_;  // Event signaled when count changes
```

**Update shared_memory_manager.cpp:**
```cpp
// In CreateSharedMemory():
update_event_ = CreateEventA(nullptr, FALSE, FALSE,
                              "Local\\FlutterWindowCountChanged");

// In IncrementWindowCount() and DecrementWindowCount():
SetEvent(update_event_);  // Signal listeners after atomic operation

// In Cleanup():
if (update_event_) {
  CloseHandle(update_event_);
  update_event_ = nullptr;
}
```

### Dependencies Required

- [x] Windows SDK (already available)
- [x] `<thread>` (C++11 threading)
- [x] `<atomic>` (C++11 atomic operations)
- [x] `<functional>` (std::function for callbacks)

### Windows API Functions Used

- `CreateEventA` - Create named auto-reset event
- `SetEvent` - Signal event (wake waiting threads)
- `WaitForSingleObject` - Wait on event (blocking, zero CPU)
- `CloseHandle` - Close event handle
- `std::thread` - Background thread management
- `std::atomic<bool>` - Thread-safe running flag

### Edge Cases to Handle

- [ ] Start() called multiple times (should be idempotent)
- [ ] Stop() called when not running (safe no-op)
- [ ] Event creation fails (nullptr check)
- [ ] Thread fails to start (exception handling)
- [ ] Callback is nullptr (skip callback execution)
- [ ] Rapid event signals (ensure no missed notifications)

---

## Acceptance Criteria

- [ ] `window_count_listener.h` created with class declaration
- [ ] `window_count_listener.cpp` implements all methods
- [ ] `shared_memory_manager.h` updated with event handle
- [ ] `shared_memory_manager.cpp` signals event on count changes
- [ ] CMakeLists.txt updated
- [ ] Code follows `.claude/cpp-style-guide.md`:
  - [ ] PascalCase for methods
  - [ ] snake_case_ for members
  - [ ] Include guards: `RUNNER_WINDOW_COUNT_LISTENER_H_`
  - [ ] RAII pattern for thread management
- [ ] Build succeeds
- [ ] Console shows listener start/stop messages
- [ ] Console shows notifications when count changes
- [ ] CPU usage <1% when idle
- [ ] Thread stops cleanly on exit
- [ ] No resource leaks (handles, threads)

---

## Implementation Notes

### Architectural Decisions

**Decision 1: Auto-reset vs Manual-reset Event**
- *Chosen:* Auto-reset event (`FALSE` parameter)
- *Why:* Automatically resets after WaitForSingleObject, simpler logic
- *Alternative:* Manual-reset (requires ResetEvent call)

**Decision 2: std::thread vs Windows CreateThread**
- *Chosen:* std::thread (C++11)
- *Why:* RAII pattern, exception-safe, cross-platform portable
- *Alternative:* CreateThread (Windows-specific, manual cleanup)

**Decision 3: Callback vs Direct Integration**
- *Chosen:* Optional callback (std::function)
- *Why:* Flexible, allows future integration with DartPortManager
- *Alternative:* Direct coupling (less flexible)

**Decision 4: Polling interval vs Event-driven**
- *Chosen:* Event-driven (WaitForSingleObject blocks)
- *Why:* Zero CPU usage, <10ms latency
- *Alternative:* Polling every 100ms (10-20% CPU, poor latency)

### Thread Safety Patterns

**For listener thread:**
- Use `std::atomic<bool>` for is_running_ flag
- Join thread in destructor before cleanup
- Check is_running_ in loop condition

**For event signaling:**
- SetEvent is thread-safe (Windows API guarantees)
- Multiple processes can signal same event safely

**For callback:**
- Store callback in member variable (not thread-safe write)
- Set callback before Start() (safe)
- Read-only access in thread (safe)

---

## Test Results Tracking

### Iteration 1 (RED Phase)

**Date:** [To be filled]
**Status:** Structure created, implementation pending
**Expected:** Build succeeds with stub implementations

### Iteration 2 (GREEN Phase)

**Date:** [To be filled]
**Tests Passed:** 0/7 → 7/7 ✅
**Status:** Full implementation, all manual tests passing
**Notes:** Console output matches expectations, CPU <1%

### Iteration 3 (REFACTOR Phase)

**Date:** [To be filled]
**Tests Status:** All passing ✅
**Refactoring Done:** Documentation, error handling improvements
**Notes:** Production-ready code

---

## Related Features

- **Depends on:** SharedMemoryManager (Layer 1) ✅
- **Enables:** DartPortManager (Layer 3) - future
- **Integration:** FlutterWindow lifecycle

---

## Checklist Before Implementation

- [x] Layer 1 (SharedMemoryManager) complete and tested
- [x] C++ Style Guide reviewed
- [x] SHARED_MEMORY_COUNTER_GUIDE.md reviewed
- [x] Understanding of Windows Events
- [x] Understanding of std::thread
- [x] Ready to start RED phase

---

## Command References

### Start TDD Implementation
```bash
/tdd-implement .claude/tdd-tasks/implement-windowcountlistener.md
```

### Build and Run
```bash
flutter clean
flutter build windows
flutter run -d windows
```

---

**Created:** 2025-11-28
**Last Modified:** 2025-11-28
