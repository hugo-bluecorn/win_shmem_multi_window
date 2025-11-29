# TDD Task: Diagnose Shared Memory Cross-Process Issue

**Status:** In Progress
**Created:** 2025-11-29
**Priority:** Critical - Blocking all multi-window functionality

---

## Problem Statement

Shared memory created via `CreateFileMappingA` with "Local\\" namespace is NOT being shared across processes launched via Dart's `Process.start()` with `ProcessStartMode.detached`. Each process creates its own separate memory section instead of opening the existing one.

**Evidence:**
- Every process logs "Shared memory created" (should be "opened (already exists)" for 2nd+)
- Every process increments window_count to 1 (should increment shared global counter to 2, 3, 4...)
- Events ARE shared correctly (proves "Local\\" namespace works for events)
- This indicates issue is specific to `CreateFileMappingA` usage, not Windows named object system

---

## Investigation Goals

1. Verify `CreateFileMappingA` behavior with different namespace prefixes
2. Verify `GetLastError()` returns correct ERROR_ALREADY_EXISTS code
3. Verify process launch method doesn't affect named object sharing
4. Identify root cause of separate memory sections
5. Test potential fixes (Global\\, security descriptors, etc.)

---

## Test Specifications

### Test Suite 1: CreateFileMappingA Error Code Verification

#### Test 1.1: Verify GetLastError() Called Immediately After CreateFileMappingA

**Purpose:** Ensure GetLastError() is called before any other Windows API that might reset it

**Current Code Issue:**
```cpp
shared_memory_handle_ = CreateFileMappingA(...);

if (shared_memory_handle_ == nullptr) {
    // This GetLastError() call might interfere
    DWORD error = GetLastError();
    ...
}

// GetLastError() called here - might be too late!
DWORD last_error = GetLastError();
bool already_exists = (last_error == ERROR_ALREADY_EXISTS);
```

**Expected Behavior:**
- GetLastError() must be called IMMEDIATELY after CreateFileMappingA
- No other Windows API calls between CreateFileMappingA and GetLastError()

**Test Procedure:**
1. Add logging IMMEDIATELY after CreateFileMappingA:
   ```cpp
   shared_memory_handle_ = CreateFileMappingA(...);
   DWORD last_error = GetLastError();  // IMMEDIATELY

   std::cout << "CreateFileMappingA returned handle: " << shared_memory_handle_
             << ", GetLastError() = " << last_error
             << ", ERROR_ALREADY_EXISTS = " << ERROR_ALREADY_EXISTS << std::endl;
   ```

2. Run two instances
3. Verify console output

**Expected Output:**
- First instance: `GetLastError() = 0` (or ERROR_SUCCESS)
- Second instance: `GetLastError() = 183` (ERROR_ALREADY_EXISTS)

**Acceptance Criteria:**
- [ ] First process: last_error != ERROR_ALREADY_EXISTS
- [ ] Second process: last_error == ERROR_ALREADY_EXISTS (183)
- [ ] Both processes: shared_memory_handle_ != nullptr

---

#### Test 1.2: Verify Shared Memory Data Is Actually Shared

**Purpose:** Confirm both processes map to the SAME physical memory

**Test Procedure:**
1. First process: Write magic value to shared memory after creation:
   ```cpp
   if (!already_exists) {
       shared_data_->window_count = 0;
       shared_data_->reserved[0] = 0xDEADBEEF;  // Magic marker
       std::cout << "Set magic marker: 0xDEADBEEF" << std::endl;
   } else {
       std::cout << "Opened shared memory, magic marker = 0x"
                 << std::hex << shared_data_->reserved[0] << std::dec << std::endl;
   }
   ```

2. Run first instance, verify magic marker set
3. Run second instance, check if it sees the magic marker

**Expected Output:**
- First instance: "Set magic marker: 0xDEADBEEF"
- Second instance: "Opened shared memory, magic marker = 0xDEADBEEF"

**Acceptance Criteria:**
- [ ] Second process reads 0xDEADBEEF (proves shared memory)
- [ ] If reads 0x00000000, memory is NOT shared (separate sections)

---

### Test Suite 2: Namespace Prefix Testing

#### Test 2.1: Test with "Global\\" Namespace

**Purpose:** Verify if Global\\ namespace shares memory correctly

**Hypothesis:** "Local\\" may not work with detached processes in different sessions

**Test Procedure:**
1. Change `kSharedMemoryName` to `"Global\\FlutterMultiWindowCounter"`
2. Change `kEventName` to `"Global\\FlutterWindowCountChanged"`
3. Run two instances
4. Verify console output

**Expected Behavior:**
- Both processes should share memory with Global\\ namespace
- Windows 10+ allows Global\\ without admin rights for default security

**Acceptance Criteria:**
- [ ] Second process logs "Shared memory opened (already exists)"
- [ ] Second process sees magic marker from first process
- [ ] Window count increments to 2, 3, 4... (not 1, 1, 1...)

---

#### Test 2.2: Test with No Namespace Prefix

**Purpose:** Test unnamed namespace (default to BaseNamedObjects)

**Test Procedure:**
1. Change `kSharedMemoryName` to `"FlutterMultiWindowCounter"` (no Local\\ or Global\\)
2. Run two instances
3. Verify sharing behavior

**Acceptance Criteria:**
- [ ] Document if sharing works without namespace prefix
- [ ] Compare with Local\\ and Global\\ results

---

### Test Suite 3: Security Descriptor Testing

#### Test 3.1: Verify Current Security Descriptor

**Purpose:** Check if default security descriptor (nullptr) is causing isolation

**Test Procedure:**
1. Add logging to show current process security context:
   ```cpp
   HANDLE hToken;
   if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
       DWORD dwLength = 0;
       GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
       // Log user SID
       CloseHandle(hToken);
   }
   ```

2. Run instances and compare SIDs

**Acceptance Criteria:**
- [ ] Both processes have same user SID
- [ ] Security context is identical

---

#### Test 3.2: Test with Explicit SECURITY_DESCRIPTOR

**Purpose:** Use explicit "allow all" security descriptor

**Test Procedure:**
1. Create security descriptor:
   ```cpp
   SECURITY_ATTRIBUTES sa;
   SECURITY_DESCRIPTOR sd;
   InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
   SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE); // Allow all
   sa.nLength = sizeof(SECURITY_ATTRIBUTES);
   sa.lpSecurityDescriptor = &sd;
   sa.bInheritHandle = FALSE;

   shared_memory_handle_ = CreateFileMappingA(
       INVALID_HANDLE_VALUE,
       &sa,  // Explicit security attributes
       PAGE_READWRITE,
       0,
       kSharedMemorySize,
       kSharedMemoryName);
   ```

2. Run two instances
3. Verify sharing

**Acceptance Criteria:**
- [ ] Explicit security descriptor allows sharing
- [ ] Compare with nullptr security descriptor behavior

---

### Test Suite 4: Process Launch Method Testing

#### Test 4.1: Compare ProcessStartMode.detached vs ProcessStartMode.normal

**Purpose:** Verify if detached mode affects named object sharing

**Test Procedure:**
1. Modify `_createNewWindow()` to use `ProcessStartMode.normal`:
   ```dart
   await Process.start(
     Platform.resolvedExecutable,
     [],
     mode: ProcessStartMode.normal,  // Changed from detached
   );
   ```

2. Run and observe if sharing works

**Expected Behavior:**
- ProcessStartMode should not affect named object sharing (same session)

**Acceptance Criteria:**
- [ ] Document if normal mode shares memory
- [ ] Document if detached mode shares memory
- [ ] Identify any differences

---

#### Test 4.2: Test Direct EXE Launch vs Flutter Run

**Purpose:** Isolate Flutter tooling from native Windows behavior

**Test Procedure:**
1. Build release executable: `flutter build windows --release`
2. Manually launch multiple instances by double-clicking EXE
3. Verify console output (redirect to file)

**Acceptance Criteria:**
- [ ] Multiple EXE instances share memory correctly
- [ ] Isolates issue to debug mode vs release mode
- [ ] Or isolates to Flutter run vs standalone EXE

---

### Test Suite 5: Handle Inspection with Process Explorer

#### Test 5.1: Verify Named Object Handles

**Purpose:** Use Sysinternals Process Explorer to inspect kernel handles

**Test Procedure:**
1. Download Process Explorer (https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer)
2. Run two application instances
3. View → Lower Pane View → Handles
4. Search for "FlutterMultiWindowCounter"

**Expected Findings:**
- Both processes should have handle to `Section\BaseNamedObjects\Local\FlutterMultiWindowCounter`
- Same Section object (same kernel address)
- If different Section objects → separate memory (BUG CONFIRMED)

**Acceptance Criteria:**
- [ ] Document handle names for both processes
- [ ] Document kernel object addresses
- [ ] Verify if same object or different objects

---

#### Test 5.2: Event Handle Verification

**Purpose:** Verify event IS shared (as control/comparison)

**Test Procedure:**
1. In Process Explorer, search for "FlutterWindowCountChanged"
2. Verify both processes have handles to SAME event object

**Expected Findings:**
- Both processes: `Event\BaseNamedObjects\Local\FlutterWindowCountChanged`
- Same kernel object address
- Proves Local\\ namespace works for Events

**Acceptance Criteria:**
- [ ] Confirm event is shared (same kernel object)
- [ ] Compare with Section object (shared memory) behavior
- [ ] Identify why Event works but Section doesn't

---

### Test Suite 6: Minimal Reproduction Test

#### Test 6.1: Create Standalone Test Executable

**Purpose:** Isolate issue from Flutter framework

**Test Procedure:**
1. Create minimal C++ test program:
   ```cpp
   // test_shared_memory.cpp
   #include <windows.h>
   #include <iostream>

   int main() {
       const char* name = "Local\\TestSharedMemory";

       HANDLE hMap = CreateFileMappingA(
           INVALID_HANDLE_VALUE,
           nullptr,
           PAGE_READWRITE,
           0,
           16,
           name);

       DWORD err = GetLastError();

       int* data = (int*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 16);

       if (err == ERROR_ALREADY_EXISTS) {
           std::cout << "Opened existing: value = " << *data << std::endl;
       } else {
           *data = 42;
           std::cout << "Created new: set value = 42" << std::endl;
       }

       system("pause");
       UnmapViewOfFile(data);
       CloseHandle(hMap);
       return 0;
   }
   ```

2. Compile: `cl test_shared_memory.cpp`
3. Run two instances manually
4. Verify sharing works in standalone program

**Expected Behavior:**
- First instance: "Created new: set value = 42"
- Second instance: "Opened existing: value = 42"

**Acceptance Criteria:**
- [ ] Standalone C++ program shares memory correctly
- [ ] Proves CreateFileMappingA works as expected
- [ ] Issue is Flutter-specific or process-launch-specific

---

### Test Suite 7: Code Review Verification

#### Test 7.1: Verify No Memory Reinitialization

**Purpose:** Ensure second process doesn't overwrite shared data

**Current Code:**
```cpp
if (!already_exists) {
    shared_data_->window_count = 0;  // Only for first process
    for (int i = 0; i < 3; i++) {
        shared_data_->reserved[i] = 0;
    }
}
```

**Potential Bug:**
- If `already_exists` is incorrectly false (due to GetLastError issue)
- Second process will reset window_count to 0
- Overwrites first process's data

**Acceptance Criteria:**
- [ ] Verify already_exists logic is correct
- [ ] Verify memory not reinitialized by second process
- [ ] Add logging before/after initialization

---

#### Test 7.2: Verify MapViewOfFile Parameters

**Purpose:** Ensure mapping parameters are correct

**Current Code:**
```cpp
shared_data_ = static_cast<SharedMemoryData*>(
    MapViewOfFile(shared_memory_handle_,
                  FILE_MAP_ALL_ACCESS,
                  0,  // High offset
                  0,  // Low offset
                  kSharedMemorySize));
```

**Verification:**
- All parameters correct for mapping entire section
- FILE_MAP_ALL_ACCESS grants full permissions
- Offset 0, 0 starts at beginning
- Size matches CreateFileMappingA size

**Acceptance Criteria:**
- [ ] Parameters are correct
- [ ] MapViewOfFile succeeds (non-null return)
- [ ] Both processes map same size

---

## Test Execution Order

**Phase 1: Quick Diagnostics (15 minutes)**
1. Test 1.1: GetLastError timing
2. Test 1.2: Magic marker verification
3. Test 5.1: Process Explorer handle inspection

**Phase 2: Namespace Testing (10 minutes)**
4. Test 2.1: Global\\ namespace
5. Test 2.2: No namespace prefix

**Phase 3: Deep Investigation (20 minutes)**
6. Test 6.1: Standalone test program
7. Test 4.2: Direct EXE launch
8. Test 3.2: Explicit security descriptor

**Phase 4: Code Review (10 minutes)**
9. Test 7.1: Reinitialization check
10. Test 7.2: MapViewOfFile verification

---

## Expected Outcomes

**If Test 1.2 fails (magic marker = 0x00000000):**
- Confirms memory is NOT shared
- Proceed to Test 2.1 (Global\\)

**If Test 2.1 succeeds:**
- Root cause: Local\\ namespace issue with detached processes
- Solution: Use Global\\ namespace

**If Test 6.1 succeeds but application fails:**
- Root cause: Flutter Process.start or debug mode specific
- Solution: Test with release build (Test 4.2)

**If all tests fail:**
- Deep investigation into security descriptors (Test 3.x)
- Check Windows version/session isolation

---

## Success Criteria

- [ ] Root cause identified and documented
- [ ] Proposed fix verified with test
- [ ] All three layers working end-to-end
- [ ] Window count synchronizes: 1 → 2 → 3 → 4...
- [ ] Cross-window notifications trigger callbacks
- [ ] Dart UI updates in all windows

---

## Notes

- Keep diagnostic logging in place until issue resolved
- Document all GetLastError() values
- Use Process Explorer for kernel object inspection
- Test on Windows 10 and Windows 11 if available

---

**Created:** 2025-11-29
**Last Modified:** 2025-11-29
