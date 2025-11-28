// shared_memory_manager.cpp
//
// Implementation of SharedMemoryManager for Windows shared memory management.

#include "shared_memory_manager.h"

#include <iostream>

// Shared memory configuration constants
namespace {
// Use "Local\" namespace to scope shared memory to current login session.
// Alternative "Global\" would require administrator privileges and share
// memory across all sessions, which is unnecessary for this use case.
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
constexpr size_t kSharedMemorySize = sizeof(SharedMemoryData);
const char* kEventName = "Local\\FlutterWindowCountChanged";
}  // anonymous namespace

SharedMemoryManager::SharedMemoryManager()
    : shared_memory_handle_(nullptr),
      shared_data_(nullptr),
      is_initialized_(false),
      update_event_(nullptr) {
  // Constructor initializes all members to safe defaults
  // Actual initialization happens in Initialize()
  // TODO: In GREEN phase, create update_event_ in CreateSharedMemory()
}

SharedMemoryManager::~SharedMemoryManager() {
  Cleanup();
}

bool SharedMemoryManager::Initialize() {
  if (is_initialized_) {
    std::cout << "SharedMemoryManager already initialized" << std::endl;
    return true;
  }

  if (!CreateSharedMemory()) {
    std::cerr << "Failed to create/open shared memory" << std::endl;
    return false;
  }

  is_initialized_ = true;
  return true;
}

LONG SharedMemoryManager::IncrementWindowCount() {
  if (!is_initialized_ || !shared_data_) {
    std::cerr << "SharedMemoryManager not initialized" << std::endl;
    return -1;
  }

  LONG new_count = InterlockedIncrement(&shared_data_->window_count);
  std::cout << "Window count incremented: " << new_count << std::endl;

  // Signal event to notify listeners of count change
  if (update_event_) {
    SetEvent(update_event_);
  }

  return new_count;
}

LONG SharedMemoryManager::DecrementWindowCount() {
  if (!is_initialized_ || !shared_data_) {
    std::cerr << "SharedMemoryManager not initialized" << std::endl;
    return -1;
  }

  LONG new_count = InterlockedDecrement(&shared_data_->window_count);
  std::cout << "Window count decremented: " << new_count << std::endl;

  // Signal event to notify listeners of count change
  if (update_event_) {
    SetEvent(update_event_);
  }

  return new_count;
}

LONG SharedMemoryManager::GetWindowCount() const {
  if (!shared_data_) {
    return 0;
  }
  return shared_data_->window_count;
}

bool SharedMemoryManager::CreateSharedMemory() {
  // Create or open shared memory section using Windows file mapping.
  // INVALID_HANDLE_VALUE tells Windows to use the paging file rather than
  // a physical file on disk, which is appropriate for shared memory.
  shared_memory_handle_ = CreateFileMappingA(
      INVALID_HANDLE_VALUE,  // Use system paging file (not a real file)
      nullptr,               // Default security descriptor
      PAGE_READWRITE,        // Read/write access for all processes
      0,                     // High-order DWORD of maximum size
      kSharedMemorySize,     // Low-order DWORD of maximum size (16 bytes)
      kSharedMemoryName);    // Name of mapping object

  if (shared_memory_handle_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "CreateFileMappingA failed for '" << kSharedMemoryName
              << "': Error code " << error << std::endl;
    return false;
  }

  // Check if shared memory already existed (opened by another process).
  // GetLastError() returns ERROR_ALREADY_EXISTS even when CreateFileMapping
  // succeeds, to distinguish between creating new vs opening existing.
  bool already_exists = (GetLastError() == ERROR_ALREADY_EXISTS);

  // Map the shared memory section into this process's address space.
  // This allows us to access the shared memory as a regular pointer.
  // All processes that map this section will see the same physical memory.
  shared_data_ = static_cast<SharedMemoryData*>(
      MapViewOfFile(shared_memory_handle_,  // Handle to mapping object
                    FILE_MAP_ALL_ACCESS,     // Read/write permission
                    0,                       // High-order offset (0 = start)
                    0,                       // Low-order offset (0 = start)
                    kSharedMemorySize));     // Number of bytes to map

  if (shared_data_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "MapViewOfFile failed for '" << kSharedMemoryName
              << "': Error code " << error << std::endl;
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
    return false;
  }

  // Only the first process (creator) initializes the shared memory data.
  // Subsequent processes (openers) preserve existing data.
  if (!already_exists) {
    shared_data_->window_count = 0;
    for (int i = 0; i < 3; i++) {
      shared_data_->reserved[i] = 0;
    }
    std::cout << "Shared memory created: " << kSharedMemoryName << std::endl;
  } else {
    std::cout << "Shared memory opened (already exists): "
              << kSharedMemoryName << std::endl;
  }

  // Create event for notifying listeners of window count changes
  update_event_ = CreateEventA(nullptr, FALSE, FALSE, kEventName);
  if (update_event_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "CreateEventA failed: " << error << std::endl;
    // Continue anyway - event is not critical for basic functionality
  }

  return true;
}

void SharedMemoryManager::Cleanup() {
  // RAII cleanup: Release Windows resources in reverse order of acquisition.
  // Safe to call multiple times or with null handles.

  // First, unmap the memory view (if mapped)
  if (shared_data_) {
    UnmapViewOfFile(shared_data_);
    shared_data_ = nullptr;
  }

  // Then close the file mapping handle (if created/opened)
  if (shared_memory_handle_) {
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
  }

  // Finally, close the event handle (if created)
  if (update_event_) {
    CloseHandle(update_event_);
    update_event_ = nullptr;
  }

  is_initialized_ = false;
}
