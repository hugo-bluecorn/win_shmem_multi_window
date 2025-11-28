# Feature Notes: Implement SharedMemoryManager

**Created:** 2025-11-28
**Status:** Planning

---

## Overview

### Purpose

Implement **Layer 1 (Windows Shared Memory)** of the three-layer event-driven multi-window architecture. This component creates and manages named shared memory accessible across all Flutter window processes, enabling zero-latency cross-process state synchronization.

### Use Cases

- **Create shared memory:** First window creates "Local\FlutterMultiWindowCounter" section
- **Open existing memory:** Additional windows open the same shared memory
- **Atomic operations:** Thread-safe increment/decrement of window counter
- **Read current state:** All processes can read window count instantly

### Context

**Three-Layer Architecture:**
```
[Layer 1: SharedMemoryManager] ← THIS FEATURE
         ↓
[Layer 2: WindowCountListener] (Future)
         ↓
[Layer 3: DartPortManager] (Future)
```

**Technologies:**
- Windows API: `CreateFileMapping`, `MapViewOfFile`
- C++ Atomic: `InterlockedIncrement`/`Decrement`
- RAII pattern for resource management

---

## Requirements

### Functional Requirements

1. Create/open shared memory section "Local\FlutterMultiWindowCounter"
2. Handle ERROR_ALREADY_EXISTS gracefully
3. Provide atomic increment/decrement operations
4. Provide read-only window count access
5. Clean up resources in destructor

### Non-Functional Requirements

- Thread-safe atomic operations
- Zero-copy direct memory access
- Proper Windows API error handling
- Follow `.claude/cpp-style-guide.md` conventions

---

## Implementation

### Files to Create

- `windows/runner/shared_memory_manager.h` - Header
- `windows/runner/shared_memory_manager.cpp` - Implementation
- Update `windows/runner/CMakeLists.txt` - Add to build

### Memory Layout

```cpp
struct SharedMemoryData {
  volatile LONG window_count;  // 4 bytes
  DWORD reserved[3];           // 12 bytes
  // Total: 16 bytes (cache-aligned)
};
```

---

## Testing Strategy

**Manual verification** (no automated tests for Windows API layer):

1. **Build test:** `flutter build windows` - no errors
2. **First window:** Console shows "Shared memory created", count=1
3. **Second window:** Console shows "Shared memory opened", count=2
4. **Process Explorer:** Verify handle exists

---

## Acceptance Criteria

- [ ] Files created and compiling
- [ ] Console shows correct creation/opening messages
- [ ] Window count increments/decrements correctly
- [ ] Process Explorer shows shared memory handle
- [ ] Code follows C++ style guide
- [ ] No memory leaks (handles closed properly)

---

## References

- [SHARED_MEMORY_COUNTER_GUIDE.md](../../SHARED_MEMORY_COUNTER_GUIDE.md) - Part 1
- [.claude/cpp-style-guide.md](../../.claude/cpp-style-guide.md)
- [Windows Shared Memory API](https://docs.microsoft.com/en-us/windows/win32/memory/creating-named-shared-memory)
