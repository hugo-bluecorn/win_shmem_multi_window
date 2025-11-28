// window_count_listener.h
//
// Event-driven listener for window count changes.
// Runs background thread waiting on Windows Event with zero CPU overhead.

#ifndef RUNNER_WINDOW_COUNT_LISTENER_H_
#define RUNNER_WINDOW_COUNT_LISTENER_H_

#include <windows.h>

#include <atomic>
#include <functional>
#include <thread>

// Callback function type for window count change notifications.
// Called when window count changes, receives new count as parameter.
using WindowCountCallback = std::function<void(LONG new_count)>;

// Listens for window count changes via Windows Event objects.
//
// Creates background thread that waits on named Event using
// WaitForSingleObject, achieving zero CPU overhead when idle.
// When SharedMemoryManager signals the event (via SetEvent),
// the thread wakes up and can execute optional callback.
//
// Thread-safe: Uses atomic flag for start/stop control.
// RAII: Automatically stops thread and cleans up resources.
//
// Example usage:
//   WindowCountListener listener;
//   listener.SetCallback([](LONG count) {
//     std::cout << "New count: " << count << std::endl;
//   });
//   listener.Start();
//   // ... listener runs in background ...
//   listener.Stop();  // Or automatic on destruction
class WindowCountListener {
 public:
  // Constructs WindowCountListener with uninitialized state.
  // Call Start() to begin listening.
  WindowCountListener();

  // Stops listener thread and cleans up resources.
  // Uses RAII pattern for automatic cleanup.
  ~WindowCountListener();

  // Starts background listener thread.
  //
  // Creates Windows Event if not already created.
  // Spawns background thread that waits on event.
  // Thread remains blocked (zero CPU) until event is signaled.
  //
  // Returns true on success, false on error.
  // Safe to call multiple times (idempotent).
  bool Start();

  // Stops background listener thread.
  //
  // Sets running flag to false, signals event to wake thread,
  // then joins thread to wait for clean exit.
  //
  // Safe to call when not running (no-op).
  void Stop();

  // Sets callback function to execute when window count changes.
  //
  // Callback is invoked in background thread context when event
  // is signaled. Should be set before calling Start().
  //
  // Pass nullptr to disable callback.
  void SetCallback(WindowCountCallback callback);

  // Returns true if listener thread is currently running.
  bool IsRunning() const;

 private:
  // Background thread function that waits on event.
  //
  // Runs in loop:
  // 1. Wait on event (WaitForSingleObject blocks, zero CPU)
  // 2. When signaled, wake up
  // 3. Execute callback if set
  // 4. Repeat until is_running_ becomes false
  void ListenerThreadFunction();

  // Creates Windows Event object.
  //
  // Event name: "Local\FlutterWindowCountChanged"
  // Type: Auto-reset (automatically resets after WaitForSingleObject)
  //
  // Returns true on success, false on error.
  bool CreateEvent();

  // Cleans up event handle.
  //
  // Closes Windows Event handle if valid.
  // Safe to call multiple times.
  void Cleanup();

  HANDLE update_event_;                  // Event signaled on count change
  std::thread listener_thread_;          // Background listener thread
  std::atomic<bool> is_running_;         // Thread running flag (atomic)
  WindowCountCallback callback_;         // Optional notification callback
};

#endif  // RUNNER_WINDOW_COUNT_LISTENER_H_
