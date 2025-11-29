// window_count_listener.cpp
//
// Implementation of WindowCountListener for event-driven notifications.

#include "window_count_listener.h"

#include <iostream>

// Event configuration constants
namespace {
const char* kEventName = "Local\\FlutterWindowCountChanged";
constexpr DWORD kWaitTimeout = 5000;  // 5 second timeout for safety
}  // anonymous namespace

WindowCountListener::WindowCountListener()
    : update_event_(nullptr),
      is_running_(false),
      callback_(nullptr),
      last_notified_count_(-1) {
  // Constructor initializes members to safe defaults
  // Actual initialization happens in Start()
}

WindowCountListener::~WindowCountListener() {
  Stop();
  Cleanup();
}

bool WindowCountListener::Start() {
  if (is_running_) {
    std::cout << "WindowCountListener already running" << std::endl;
    return true;  // Idempotent - already started
  }

  if (!CreateUpdateEvent()) {
    std::cerr << "Failed to create event for WindowCountListener" << std::endl;
    return false;
  }

  // Set running flag before starting thread
  is_running_ = true;

  // Start background thread
  listener_thread_ = std::thread(&WindowCountListener::ListenerThreadFunction, this);

  std::cout << "WindowCountListener started" << std::endl;
  return true;
}

void WindowCountListener::Stop() {
  if (!is_running_) {
    return;  // Not running, nothing to stop
  }

  // Signal thread to stop
  is_running_ = false;

  // Wake up the waiting thread by signaling the event
  if (update_event_) {
    SetEvent(update_event_);
  }

  // Wait for thread to finish
  if (listener_thread_.joinable()) {
    listener_thread_.join();
  }

  std::cout << "WindowCountListener stopped" << std::endl;
}

void WindowCountListener::SetCallback(WindowCountCallback callback) {
  callback_ = callback;
}

bool WindowCountListener::IsRunning() const {
  return is_running_;
}

void WindowCountListener::ListenerThreadFunction() {
  std::cout << "WindowCountListener thread started" << std::endl;

  while (is_running_) {
    // Wait for event to be signaled (blocks thread, zero CPU usage)
    DWORD result = WaitForSingleObject(update_event_, kWaitTimeout);

    if (!is_running_) {
      break;  // Stop requested
    }

    if (result == WAIT_OBJECT_0) {
      // Event was signaled - window count changed
      // With manual-reset event, ALL waiting threads across processes wake up.
      std::cout << "Window count changed notification received" << std::endl;

      // Reset event after short delay to allow other threads to wake.
      // This is a compromise - gives ~10ms for other processes to catch the signal.
      Sleep(10);
      ResetEvent(update_event_);

      // Execute callback if set
      if (callback_) {
        try {
          // Note: Callback should read actual count from SharedMemoryManager.
          // Pass 0 as placeholder - callback ignores this and reads from shared memory.
          callback_(0);
        } catch (const std::exception& e) {
          std::cerr << "Callback threw exception: " << e.what() << std::endl;
        } catch (...) {
          std::cerr << "Callback threw unknown exception" << std::endl;
        }
      }
    } else if (result == WAIT_TIMEOUT) {
      // Timeout - continue loop (allows checking is_running_ periodically)
      continue;
    } else {
      // Error occurred
      DWORD error = GetLastError();
      std::cerr << "WaitForSingleObject failed: " << error << std::endl;
      break;
    }
  }

  std::cout << "WindowCountListener thread exiting" << std::endl;
}

bool WindowCountListener::CreateUpdateEvent() {
  if (update_event_ != nullptr) {
    return true;  // Already created
  }

  // Create manual-reset event (TRUE parameter)
  // Manual-reset stays signaled until explicitly reset, allowing
  // ALL waiting threads across processes to wake up and process.
  update_event_ = CreateEventA(
      nullptr,       // Default security
      TRUE,          // Manual-reset event (allows all waiters to wake)
      FALSE,         // Initially non-signaled
      kEventName);   // Event name

  if (update_event_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "CreateEventA failed: " << error << std::endl;
    return false;
  }

  // ERROR_ALREADY_EXISTS means another process created the event
  // This is normal and expected for multi-process scenarios
  bool already_exists = (GetLastError() == ERROR_ALREADY_EXISTS);

  if (already_exists) {
    std::cout << "Window count listener event opened (already exists): "
              << kEventName << std::endl;
  } else {
    std::cout << "Window count listener event created: "
              << kEventName << std::endl;
  }

  return true;
}

void WindowCountListener::Cleanup() {
  if (update_event_) {
    CloseHandle(update_event_);
    update_event_ = nullptr;
  }
}
