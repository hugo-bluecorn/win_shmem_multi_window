// shared_memory_manager.cpp
//
// Implementation of SharedMemoryManager for Windows shared memory management.

#include "shared_memory_manager.h"

#include <iostream>

// Shared memory configuration constants
namespace {
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
constexpr size_t kSharedMemorySize = sizeof(SharedMemoryData);
}  // anonymous namespace

SharedMemoryManager::SharedMemoryManager()
    : shared_memory_handle_(nullptr),
      shared_data_(nullptr),
      is_initialized_(false) {
  // Constructor initializes all members to safe defaults
  // Actual initialization happens in Initialize()
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
  return new_count;
}

LONG SharedMemoryManager::DecrementWindowCount() {
  if (!is_initialized_ || !shared_data_) {
    std::cerr << "SharedMemoryManager not initialized" << std::endl;
    return -1;
  }

  LONG new_count = InterlockedDecrement(&shared_data_->window_count);
  std::cout << "Window count decremented: " << new_count << std::endl;
  return new_count;
}

LONG SharedMemoryManager::GetWindowCount() const {
  if (!shared_data_) {
    return 0;
  }
  return shared_data_->window_count;
}

bool SharedMemoryManager::CreateSharedMemory() {
  // Create or open shared memory section
  shared_memory_handle_ = CreateFileMappingA(
      INVALID_HANDLE_VALUE,  // Use paging file
      nullptr,               // Default security
      PAGE_READWRITE,        // Read/write access
      0,                     // High-order DWORD of size
      kSharedMemorySize,     // Low-order DWORD of size
      kSharedMemoryName);    // Name of mapping object

  if (shared_memory_handle_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "CreateFileMappingA failed: " << error << std::endl;
    return false;
  }

  // Check if shared memory already existed
  bool already_exists = (GetLastError() == ERROR_ALREADY_EXISTS);

  // Map shared memory to process address space
  shared_data_ = static_cast<SharedMemoryData*>(
      MapViewOfFile(shared_memory_handle_,  // Handle to map object
                    FILE_MAP_ALL_ACCESS,     // Read/write access
                    0,                       // High-order DWORD of offset
                    0,                       // Low-order DWORD of offset
                    kSharedMemorySize));     // Number of bytes to map

  if (shared_data_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "MapViewOfFile failed: " << error << std::endl;
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
    return false;
  }

  // First process initializes the data
  if (!already_exists) {
    shared_data_->window_count = 0;
    for (int i = 0; i < 3; i++) {
      shared_data_->reserved[i] = 0;
    }
    std::cout << "Shared memory created" << std::endl;
  } else {
    std::cout << "Shared memory opened (already exists)" << std::endl;
  }

  return true;
}

void SharedMemoryManager::Cleanup() {
  if (shared_data_) {
    UnmapViewOfFile(shared_data_);
    shared_data_ = nullptr;
  }

  if (shared_memory_handle_) {
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
  }

  is_initialized_ = false;
}
