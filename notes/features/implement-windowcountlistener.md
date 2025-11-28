# Feature Notes: Implement WindowCountListener

**Created:** 2025-11-28
**Status:** Planning

---

## Overview

### Purpose

Implement **Layer 2 (Event Notification)** of the three-layer event-driven multi-window architecture. WindowCountListener creates a background thread that waits on Windows Event objects to receive instant notifications when the window count changes, eliminating the need for polling and achieving zero-latency updates with <1% CPU usage.

### Use Cases

- **Event creation:** Create Windows Event "Local\FlutterWindowCountChanged"
- **Background thread:** Spawn thread that waits on event with WaitForSingleObject
- **Signal on change:** SharedMemoryManager signals event after increment/decrement
- **Wake and notify:** Listener thread wakes up and can trigger callbacks

### Context

**Three-Layer Architecture:**
```
[Layer 1: SharedMemoryManager] ✅ COMPLETE
         ↓
[Layer 2: WindowCountListener] ← THIS FEATURE
         ↓
[Layer 3: DartPortManager] (Future)
```

**Technologies:**
- Windows Events: `CreateEvent`, `SetEvent`, `WaitForSingleObject`
- C++ Threading: `std::thread`, `std::atomic<bool>`
- Callbacks: Optional callback function for notifications

---

## Requirements

### Functional Requirements

1. Create Windows Event "Local\FlutterWindowCountChanged" (auto-reset)
2. Start background thread that waits on event
3. Handle event signaling from SharedMemoryManager
4. Provide Start/Stop lifecycle management
5. Support optional callback for notifications
6. Clean shutdown when application exits

### Non-Functional Requirements

- Thread-safe start/stop operations
- Zero CPU usage when idle (event-driven, no polling)
- <10ms notification latency
- Proper thread cleanup on destruction
- Follow `.claude/cpp-style-guide.md` conventions

---

## Implementation

### Files to Create

- `windows/runner/window_count_listener.h` - Header
- `windows/runner/window_count_listener.cpp` - Implementation
- Update `windows/runner/CMakeLists.txt` - Add to build
- Update `windows/runner/shared_memory_manager.h` - Add event handle
- Update `windows/runner/shared_memory_manager.cpp` - Signal event on changes

### Event Flow

```
[SharedMemoryManager]
  InterlockedIncrement()
         ↓
  SetEvent(update_event_)
         ↓
[WindowCountListener Thread]
  WaitForSingleObject() wakes up
         ↓
  Execute callback (optional)
         ↓
  Loop back to wait
```

---

## Testing Strategy

**Manual verification** (Windows API layer):

1. **Build test:** Compiles successfully
2. **Start listener:** Console shows "WindowCountListener started"
3. **Change count:** Increment triggers event
4. **Console output:** Shows "Window count changed" notification
5. **Stop listener:** Thread stops cleanly

---

## Acceptance Criteria

- [ ] Files created and compiling
- [ ] Background thread starts and waits on event
- [ ] SetEvent() wakes up waiting thread
- [ ] Console shows notifications when count changes
- [ ] Thread stops cleanly on destruction
- [ ] Code follows C++ style guide
- [ ] No thread leaks or race conditions

---

## References

- [SHARED_MEMORY_COUNTER_GUIDE.md](../../SHARED_MEMORY_COUNTER_GUIDE.md) - Part 1, WindowCountListener section
- [.claude/cpp-style-guide.md](../../.claude/cpp-style-guide.md)
- [Windows Event Objects](https://docs.microsoft.com/en-us/windows/win32/sync/using-event-objects)
