// shared_memory_manager.h
//
// Manages Windows shared memory for multi-window synchronization.
// Creates a named shared memory section accessible across all
// Flutter window processes for instant cross-process state updates.

#ifndef RUNNER_SHARED_MEMORY_MANAGER_H_
#define RUNNER_SHARED_MEMORY_MANAGER_H_

#include <windows.h>

// Shared memory data structure (16 bytes, cache-aligned)
// Used for cross-process communication between Flutter windows
struct SharedMemoryData {
  volatile LONG window_count;  // Atomic counter for active windows
  DWORD reserved[3];           // Reserved for future use (12 bytes)
};

// Manages a Windows shared memory section for cross-process communication.
//
// The first process creates the shared memory, subsequent processes open
// the existing section. All processes map the same physical memory using
// Windows CreateFileMapping API.
//
// Thread-safe: Uses atomic operations (InterlockedIncrement/Decrement)
// for counter updates. Multiple processes can safely access shared state.
//
// Example usage:
//   SharedMemoryManager manager;
//   if (manager.Initialize()) {
//     LONG count = manager.IncrementWindowCount();
//     // ... window logic ...
//     manager.DecrementWindowCount();
//   }
class SharedMemoryManager {
 public:
  // Constructs SharedMemoryManager with uninitialized handles.
  // Call Initialize() before using other methods.
  SharedMemoryManager();

  // Cleans up Windows handles and unmaps shared memory.
  // Uses RAII pattern for automatic resource management.
  ~SharedMemoryManager();

  // Initializes shared memory section.
  //
  // Creates new shared memory section or opens existing one.
  // First process creates, subsequent processes open existing.
  //
  // Returns true on success, false on error.
  // Call GetLastError() for detailed Windows error code on failure.
  bool Initialize();

  // Atomically increments window count.
  //
  // Thread-safe across all processes using InterlockedIncrement.
  // Must call Initialize() successfully before using this method.
  //
  // Returns new window count after incrementing, or -1 on error.
  LONG IncrementWindowCount();

  // Atomically decrements window count.
  //
  // Thread-safe across all processes using InterlockedDecrement.
  // Must call Initialize() successfully before using this method.
  //
  // Returns new window count after decrementing, or -1 on error.
  LONG DecrementWindowCount();

  // Returns current window count (non-atomic read).
  //
  // Reads current value without atomic operation. Value may change
  // immediately after read if other processes modify it.
  //
  // Returns current window count, or 0 if not initialized.
  LONG GetWindowCount() const;

 private:
  // Creates or opens the shared memory section.
  //
  // Uses CreateFileMappingA to create/open named shared memory.
  // Maps memory to process address space with MapViewOfFile.
  // Initializes SharedMemoryData structure for first process.
  //
  // Returns true on success, false on error.
  bool CreateSharedMemory();

  // Cleans up handles and unmaps memory.
  //
  // Unmaps shared memory view and closes Windows handles.
  // Safe to call multiple times or with null handles.
  void Cleanup();

  HANDLE shared_memory_handle_;  // Windows file mapping handle
  SharedMemoryData* shared_data_;  // Pointer to mapped shared memory
  bool is_initialized_;  // Tracks initialization state
  HANDLE update_event_;  // Event signaled when window count changes
};

#endif  // RUNNER_SHARED_MEMORY_MANAGER_H_
