# TDD Task: Orphan Window Detection and Cleanup

## Problem Statement

When processes terminate abnormally (crash, Task Manager kill, etc.), the window count in shared memory is not decremented. This leads to "orphan" counts that persist and cause the displayed count to be incorrect.

**Note:** This task is separate from the `fix-window-close-decrement.md` task, which addresses the common case of users clicking the close button. This task addresses edge cases where processes die without proper cleanup.

## Current Behavior

1. Window 1 opens → count = 1
2. Window 2 opens → count = 2
3. Window 2 process crashes (or killed via Task Manager) → count stays 2 ❌
4. New windows see incorrect count of 2 instead of 1

## Expected Behavior

1. Window 1 opens → count = 1
2. Window 2 opens → count = 2
3. Window 2 process crashes → count = 1 (detected and cleaned up)
4. New windows see correct count of 1

## Root Cause Analysis

The shared memory only tracks `window_count` (a single LONG). There is no way to:
- Identify which processes contributed to the count
- Detect if a process is still alive
- Clean up counts from dead processes

## Technical Approaches

### Option A: Process ID Registry (Recommended)

Store process IDs in shared memory alongside the count:

```cpp
struct SharedMemoryData {
  volatile LONG window_count;        // Current count
  volatile DWORD process_ids[16];    // Array of PIDs (support up to 16 windows)
  volatile LONG pid_count;           // Number of valid PIDs
  DWORD reserved[8];                 // Future expansion
};
```

**On window create:**
1. Add current PID to `process_ids` array
2. Increment `window_count`

**On window close:**
1. Remove current PID from `process_ids` array
2. Decrement `window_count`

**On initialize (any window):**
1. Scan `process_ids` array
2. For each PID, call `OpenProcess()` to check if alive
3. Remove dead PIDs and decrement count accordingly

### Option B: Named Mutex Per Window

Create a named mutex for each window that auto-releases on process termination:

```cpp
// On window create:
HANDLE mutex = CreateMutex(nullptr, TRUE, "Local\\FlutterWindow_<PID>");
// Mutex held while process is alive

// Other processes can detect orphans by trying to acquire mutexes
```

### Option C: Heartbeat Mechanism

Periodic "I'm alive" updates to shared memory:

```cpp
struct SharedMemoryData {
  volatile LONG window_count;
  volatile DWORD last_heartbeat[16];  // Timestamps per window
  // ...
};
```

- Each window updates its heartbeat every N seconds
- Other windows can detect stale heartbeats and clean up

## Recommended Approach: Option A (Process ID Registry)

**Pros:**
- Simple implementation
- Low overhead (only PID tracking)
- Cleanup happens on next window open (no background thread needed)
- Works across process boundaries

**Cons:**
- Limited to 16 windows (can be increased)
- Requires scanning on every initialize

## TDD Implementation Plan

### Phase 1: Expand SharedMemoryData Structure

#### Test 1.1: New structure has process_ids field
```cpp
TEST(OrphanDetection, SharedMemoryData_HasProcessIds) {
  SharedMemoryData data;
  // Verify structure has process_ids array
  EXPECT_EQ(sizeof(data.process_ids), 16 * sizeof(DWORD));
}
```

#### Test 1.2: RegisterProcessId adds PID
```cpp
TEST(OrphanDetection, RegisterProcessId_AddsPID) {
  SharedMemoryManager manager;
  manager.Initialize();

  DWORD pid = GetCurrentProcessId();
  bool result = manager.RegisterProcessId(pid);

  EXPECT_TRUE(result);
  EXPECT_TRUE(manager.IsProcessIdRegistered(pid));
}
```

### Phase 2: Dead Process Detection

#### Test 2.1: DetectDeadProcesses finds terminated PIDs
```cpp
TEST(OrphanDetection, DetectDeadProcesses_FindsDeadPIDs) {
  SharedMemoryManager manager;
  manager.Initialize();

  // Register a fake dead PID (use invalid PID that doesn't exist)
  DWORD fake_pid = 99999;  // Very unlikely to be a real process
  manager.ForceRegisterPid(fake_pid);  // Test helper

  std::vector<DWORD> dead_pids = manager.DetectDeadProcesses();

  EXPECT_TRUE(std::find(dead_pids.begin(), dead_pids.end(), fake_pid) != dead_pids.end());
}
```

### Phase 3: Automatic Cleanup on Initialize

#### Test 3.1: Initialize cleans up orphan counts
```cpp
TEST(OrphanDetection, Initialize_CleansUpOrphans) {
  // First manager: set up orphan scenario
  {
    SharedMemoryManager manager1;
    manager1.Initialize();
    manager1.ForceRegisterPid(99999);  // Fake dead PID
    manager1.ForceSetWindowCount(5);   // Artificial high count
  }

  // Second manager: should detect and clean up
  SharedMemoryManager manager2;
  manager2.Initialize();

  // Count should be corrected (99999 is dead, so -1)
  EXPECT_LT(manager2.GetWindowCount(), 5);
}
```

## Files to Modify

### C++ Layer
1. `windows/runner/shared_memory_manager.h` - Expand SharedMemoryData struct
2. `windows/runner/shared_memory_manager.cpp` - Add PID tracking and cleanup logic

### Tests
1. `windows/test/orphan_detection_test.cpp` - New test file

## Success Criteria

1. All orphan detection tests pass
2. Manual test: Kill process via Task Manager, verify count corrects on next window open
3. No regression in existing functionality
4. Performance: Cleanup adds <50ms to initialization

## Dependencies

- Requires `fix-window-close-decrement.md` to be completed first
- That fix handles the common case; this handles edge cases

## Cycle Limit

Maximum 5 TDD cycles before reassessing approach.

## Notes

- This is a **future task** - complete after `fix-window-close-decrement.md`
- Consider backwards compatibility with existing shared memory structure
- May need version field in SharedMemoryData for migration
