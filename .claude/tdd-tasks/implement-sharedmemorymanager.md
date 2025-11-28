# TDD Task: Implement SharedMemoryManager

**Status:** Not Started
**Created:** 2025-11-28
**Last Updated:** 2025-11-28

---

## Feature Description

Implement the SharedMemoryManager class to create and manage Windows shared memory for cross-process window count synchronization. This is Layer 1 of the three-layer event-driven multi-window architecture.

**Reference:** `SHARED_MEMORY_COUNTER_GUIDE.md` - Part 1 (Native Implementation)

---

## Requirements

### Functional Requirements

- [ ] Create shared memory section "Local\FlutterMultiWindowCounter" (16 bytes)
- [ ] Initialize SharedMemoryData structure (window_count + reserved fields)
- [ ] Handle ERROR_ALREADY_EXISTS (multiple processes opening same memory)
- [ ] Provide `IncrementWindowCount()` using `InterlockedIncrement`
- [ ] Provide `DecrementWindowCount()` using `InterlockedDecrement`
- [ ] Provide `GetWindowCount()` for reading current count
- [ ] Clean up resources in destructor (RAII pattern)

### Non-Functional Requirements

- [ ] Follow `.claude/cpp-style-guide.md` conventions
- [ ] Thread-safe atomic operations
- [ ] Proper Windows API error checking
- [ ] Console debug logging for verification
- [ ] No memory leaks

---

## Test Specifications

### Test 1: Build Verification

**Description:** Code compiles without errors

**Given:**
- SharedMemoryManager class header and implementation created
- CMakeLists.txt updated to include new files

**When:**
- Run `flutter clean && flutter build windows`

**Then:**
- Build completes successfully with no compilation errors
- No linker errors

**Test Location:** Manual - command line build

---

### Test 2: First Window Creates Shared Memory

**Description:** First process creates new shared memory section

**Given:**
- No existing shared memory section exists
- Application starts for the first time

**When:**
- SharedMemoryManager constructor is called
- Initialize() method creates shared memory
- IncrementWindowCount() is called

**Then:**
- Console output shows: "Shared memory created"
- ERROR_ALREADY_EXISTS is NOT returned from CreateFileMapping
- window_count initialized to 0
- After increment, window_count = 1
- Console output shows: "Window count incremented: 1"

**Test Location:** Manual - console output verification

---

### Test 3: Second Window Opens Existing Shared Memory

**Description:** Additional processes open existing shared memory

**Given:**
- First window is running with shared memory created
- Shared memory contains window_count = 1

**When:**
- Second window's SharedMemoryManager constructor is called
- Initialize() opens existing shared memory
- IncrementWindowCount() is called

**Then:**
- Console output shows: "Shared memory opened (already exists)"
- ERROR_ALREADY_EXISTS is returned from CreateFileMapping
- Existing data is preserved (window_count still accessible)
- After increment, window_count = 2
- Console output shows: "Window count incremented: 2"

**Test Location:** Manual - console output from second window

---

### Test 4: Window Count Decrement

**Description:** Window count decrements when window closes

**Given:**
- Two windows running, window_count = 2

**When:**
- One window closes
- DecrementWindowCount() is called before destruction

**Then:**
- window_count atomically decrements to 1
- Console output shows: "Window count decremented: 1"
- Remaining window can still access shared memory

**Test Location:** Manual - close one window, observe console

---

### Test 5: Resource Cleanup (RAII)

**Description:** Resources properly cleaned up in destructor

**Given:**
- SharedMemoryManager instance with initialized handles

**When:**
- SharedMemoryManager destructor is called (window closes)

**Then:**
- UnmapViewOfFile() is called on shared_data_
- CloseHandle() is called on shared_memory_handle_
- No memory leaks (verify with Process Explorer)
- shared_data_ and shared_memory_handle_ set to nullptr

**Test Location:** Manual - Process Explorer handle inspection

---

### Test 6: Process Explorer Handle Verification

**Description:** Shared memory handle visible in Process Explorer

**Given:**
- Application running with SharedMemoryManager initialized

**When:**
- Open Process Explorer (Sysinternals)
- View → Lower Pane View → Handles
- Search for "FlutterMultiWindowCounter"

**Then:**
- Find handle of type: `Section`
- Handle name: `\BaseNamedObjects\Local\FlutterMultiWindowCounter`
- Handle is present for all running window processes

**Test Location:** Process Explorer (Sysinternals tool)

---

### Test 7: Error Handling - Invalid Initialization

**Description:** Handle errors gracefully

**Given:**
- Attempt to increment before initialization

**When:**
- IncrementWindowCount() called before Initialize()

**Then:**
- Method returns -1 (error code)
- Console output shows: "SharedMemoryManager not initialized"
- Application doesn't crash

**Test Location:** Manual - code path testing

---

## Implementation Requirements

### File Structure

**Source Files:**
- `windows/runner/shared_memory_manager.h` - Class declaration
- `windows/runner/shared_memory_manager.cpp` - Implementation

**Build Configuration:**
- `windows/runner/CMakeLists.txt` - Add source files to build

### CMakeLists.txt Update

```cmake
# In windows/runner/CMakeLists.txt
add_executable(${BINARY_NAME} WIN32
  "flutter_window.cpp"
  "main.cpp"
  "shared_memory_manager.cpp"  # ADD THIS LINE
  "utils.cpp"
  "win32_window.cpp"
  # ... other files ...
)
```

### Class Signature (shared_memory_manager.h)

```cpp
#ifndef RUNNER_SHARED_MEMORY_MANAGER_H_
#define RUNNER_SHARED_MEMORY_MANAGER_H_

#include <windows.h>

struct SharedMemoryData {
  volatile LONG window_count;
  DWORD reserved[3];
};

class SharedMemoryManager {
 public:
  SharedMemoryManager();
  ~SharedMemoryManager();

  bool Initialize();
  LONG IncrementWindowCount();
  LONG DecrementWindowCount();
  LONG GetWindowCount() const;

 private:
  bool CreateSharedMemory();
  void Cleanup();

  HANDLE shared_memory_handle_;
  SharedMemoryData* shared_data_;
  bool is_initialized_;
};

#endif  // RUNNER_SHARED_MEMORY_MANAGER_H_
```

### Dependencies Required

- [x] Windows SDK (already available in Flutter Windows environment)
- [x] No external packages required

### Windows API Functions Used

- `CreateFileMappingA` - Create/open shared memory section
- `MapViewOfFile` - Map shared memory to process address space
- `UnmapViewOfFile` - Unmap shared memory
- `CloseHandle` - Close handle
- `InterlockedIncrement` - Atomic increment
- `InterlockedDecrement` - Atomic decrement
- `GetLastError` - Retrieve error codes

### Edge Cases to Handle

- [ ] Shared memory already exists (ERROR_ALREADY_EXISTS)
- [ ] CreateFileMapping fails (returns nullptr)
- [ ] MapViewOfFile fails (returns nullptr)
- [ ] Increment/decrement called before initialization
- [ ] Multiple rapid increments/decrements (thread safety)
- [ ] Cleanup with null handles (defensive programming)

---

## Acceptance Criteria

- [ ] `shared_memory_manager.h` created with correct class declaration
- [ ] `shared_memory_manager.cpp` implements all methods
- [ ] CMakeLists.txt updated to include new source file
- [ ] Code follows `.claude/cpp-style-guide.md` conventions:
  - [ ] PascalCase for methods
  - [ ] snake_case_ for member variables
  - [ ] Include guards: `RUNNER_SHARED_MEMORY_MANAGER_H_`
  - [ ] 2-space indentation, 80-character lines
  - [ ] RAII pattern for resource management
- [ ] Build completes without errors: `flutter build windows`
- [ ] Console output shows correct messages:
  - [ ] "Shared memory created" for first window
  - [ ] "Shared memory opened (already exists)" for additional windows
  - [ ] "Window count incremented: N" for each increment
  - [ ] "Window count decremented: N" for each decrement
- [ ] Process Explorer shows shared memory handle
- [ ] No memory leaks (handles properly closed)
- [ ] Code includes error checking for all Windows API calls
- [ ] Inline comments explain non-obvious Windows API usage

---

## Implementation Notes

### Architectural Decisions

**Decision 1: Use "Local\" namespace**
- *Why:* Scoped to current login session, not global to machine
- *Alternative:* "Global\" namespace (requires admin rights, overkill)

**Decision 2: 16-byte shared memory size**
- *Why:* Cache-aligned, room for future expansion (reserved fields)
- *Alternative:* 4 bytes (only counter) - less flexible

**Decision 3: RAII pattern for resource management**
- *Why:* Automatic cleanup, exception-safe
- *Alternative:* Manual Cleanup() method - error-prone

**Decision 4: Atomic operations instead of mutexes**
- *Why:* Lock-free, zero overhead for simple counter
- *Alternative:* Mutex protection - unnecessary for atomic operations

### Style Guide Compliance

Follow `.claude/cpp-style-guide.md`:
- **Naming:** PascalCase methods, snake_case_ members, kPascalCase constants
- **Formatting:** 2 spaces, 80-character lines, braces on same line
- **Comments:** Document public interface, explain Windows API usage
- **Error Handling:** Check all return values, use GetLastError()

### Debug Logging Pattern

```cpp
#include <iostream>

// Success messages
std::cout << "Shared memory created" << std::endl;
std::cout << "Window count incremented: " << count << std::endl;

// Error messages
std::cerr << "CreateFileMapping failed: " << GetLastError() << std::endl;
```

---

## Test Results Tracking

### Iteration 1 (RED Phase)

**Date:** [To be filled during implementation]
**Status:** Tests defined, implementation not started
**Expected:** Build fails (files don't exist yet)

### Iteration 2 (GREEN Phase)

**Date:** [To be filled]
**Tests Passed:** 0/7 → 7/7 ✅
**Status:** Implementation complete, all manual tests passing
**Notes:** Console output matches expected, Process Explorer shows handles

### Iteration 3 (REFACTOR Phase)

**Date:** [To be filled]
**Tests Status:** All passing ✅
**Refactoring Done:**
- Extract magic constants
- Add comprehensive error messages
- Improve inline documentation
**Notes:** Code ready for production use

---

## Related Features

- **Depends on:** None (first component)
- **Enables:** WindowCountListener (Layer 2) - will read from SharedMemoryManager
- **Enables:** DartPortManager (Layer 3) - will notify Dart about changes

---

## Checklist Before Implementation

- [x] C++ Style Guide reviewed (`.claude/cpp-style-guide.md`)
- [x] SHARED_MEMORY_COUNTER_GUIDE.md Part 1 reviewed
- [x] Understanding of Windows shared memory API
- [x] Understanding of RAII pattern
- [x] Understanding of atomic operations
- [x] Ready to start with creating header file

---

## Command References

### Start TDD Implementation
```bash
/tdd-implement .claude/tdd-tasks/implement-sharedmemorymanager.md
```

### Build and Run
```bash
flutter clean
flutter pub get
flutter build windows
flutter run -d windows
```

### Verify
```powershell
# Run Process Explorer and search for "FlutterMultiWindowCounter"
```

---

**Created:** 2025-11-28
**Last Modified:** 2025-11-28
