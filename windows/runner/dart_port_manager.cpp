// dart_port_manager.cpp
//
// Implementation of DartPortManager for C++ to Dart communication using
// Dart C API DL (Dynamic Linking).
//
// Architecture: Layer 3 of event-driven multi-window IPC
//   SharedMemoryManager → WindowCountListener → DartPortManager → Dart
//
// The Dart API DL allows native code to call Dart API functions via function
// pointers that are initialized at runtime. This approach:
//   1. Avoids static linking against Dart runtime
//   2. Works across different Flutter SDK versions
//   3. Requires one-time initialization via InitDartApiDL
//
// Key Dart API DL conventions:
//   - All types/functions suffixed with _DL (e.g., Dart_Port_DL)
//   - Function pointers initialized by Dart_InitializeApiDL
//   - Must call InitDartApiDL from Dart before using API

#include "dart_port_manager.h"

#include <algorithm>
#include <iostream>

DartPortManager::DartPortManager() {
  // Constructor initializes members to safe defaults.
  // The ports_ vector starts empty; Dart isolates register via FFI.
  // No Dart API calls here - initialization happens from Dart side.
}

DartPortManager::~DartPortManager() {
  // Destructor - ports are automatically cleaned up when vector is destroyed.
  // Dart_Port_DL is just int64_t, so no special cleanup required.
  // Dart isolates should unregister before destruction, but not critical.
}

bool DartPortManager::RegisterPort(Dart_Port_DL port, LONG initial_count) {
  // Thread-safe port registration using RAII mutex guard.
  // Called from Dart via FFI when window creates ReceivePort.
  //
  // Flow: Dart creates ReceivePort → sendPort.nativePort →
  //       FFI call to RegisterWindowCountPort → this method
  std::lock_guard<std::mutex> lock(ports_mutex_);

  ports_.push_back(port);
  std::cout << "Dart port registered: " << port << std::endl;

  // Send initial count to newly registered port if provided.
  // This ensures Dart receives the current state immediately.
  if (initial_count >= 0) {
    Dart_CObject message;
    message.type = Dart_CObject_kInt64;
    message.value.as_int64 = static_cast<int64_t>(initial_count);

    bool result = Dart_PostCObject_DL(port, &message);
    if (result) {
      std::cout << "Sent initial count (" << initial_count
                << ") to newly registered port" << std::endl;
    } else {
      std::cerr << "Failed to send initial count to port" << std::endl;
    }
  }

  return true;
}

bool DartPortManager::UnregisterPort(Dart_Port_DL port) {
  // Thread-safe port removal using RAII mutex guard.
  // Called from Dart via FFI when window is destroyed (dispose()).
  //
  // Uses std::find + erase pattern for O(n) removal.
  // Acceptable since N is small (<10 windows typically).
  std::lock_guard<std::mutex> lock(ports_mutex_);

  auto it = std::find(ports_.begin(), ports_.end(), port);
  if (it != ports_.end()) {
    ports_.erase(it);
    std::cout << "Dart port unregistered: " << port << std::endl;
    return true;
  }

  return false;  // Port not found (already removed or never registered)
}

void DartPortManager::NotifyWindowCountChanged(LONG new_count) {
  // Broadcast window count update to all registered Dart isolates.
  // Called from WindowCountListener::ListenerThreadFunction when event signals.
  //
  // Thread Safety: Acquires mutex to protect ports_ during iteration.
  // Dart_PostCObject_DL is thread-safe and can be called from any thread.
  //
  // Performance: O(n) where n = number of registered ports (typically <10).
  // Dart_PostCObject_DL is non-blocking, so minimal latency (<1ms per port).
  std::lock_guard<std::mutex> lock(ports_mutex_);

  if (ports_.empty()) {
    return;  // No Dart isolates registered, nothing to notify
  }

  std::cout << "Notifying " << ports_.size() << " Dart port(s) of window count: "
            << new_count << std::endl;

  // Create Dart_CObject message structure.
  // Dart_CObject is a C struct that represents Dart objects for FFI.
  // Using kInt64 type to send LONG as 64-bit integer to Dart.
  Dart_CObject message;
  message.type = Dart_CObject_kInt64;
  message.value.as_int64 = static_cast<int64_t>(new_count);

  // Broadcast to all registered Dart ports using Dart_PostCObject_DL.
  // Dart_PostCObject_DL posts message to Dart isolate's message queue.
  // The Dart isolate's ReceivePort.listen() callback will receive it.
  //
  // Error Handling: If posting fails (e.g., stale port), log error but
  // continue broadcasting to other ports. Don't crash entire app.
  for (Dart_Port_DL port : ports_) {
    bool result = Dart_PostCObject_DL(port, &message);
    if (result) {
      std::cout << "Posted to Dart port: " << port << std::endl;
    } else {
      // Post failed - port may be invalid or Dart isolate terminated.
      // Future enhancement: Remove invalid ports from registry.
      std::cerr << "Failed to post to Dart port: " << port << std::endl;
    }
  }
}

// ============================================================================
// FFI Exports - C functions callable from Dart via FFI
// ============================================================================

// Global DartPortManager instance shared across all Dart isolates.
// This allows multiple Flutter windows (each with its own Dart isolate)
// to register for window count notifications.
//
// Alternative design: Per-window DartPortManager owned by FlutterWindow.
// Current approach is simpler and sufficient for this use case.
static DartPortManager g_dart_port_manager;

// Current window count, updated by FlutterWindow.
// Used to send initial count to newly registered ports.
static LONG g_current_window_count = 0;

DartPortManager& GetGlobalDartPortManager() {
  return g_dart_port_manager;
}

void SetCurrentWindowCount(LONG count) {
  g_current_window_count = count;
}

extern "C" {

/// Initialize Dart API DL function pointers.
///
/// MUST be called once from Dart before using any Dart C API functions.
/// Initializes the function pointers (Dart_PostCObject_DL, etc.) to point
/// to the actual implementations in the Dart runtime.
///
/// Dart usage:
///   import 'dart:ffi';
///   import 'dart:nativewrappers';
///
///   final initDartApiDL = DynamicLibrary.process()
///       .lookupFunction<IntPtr Function(Pointer<Void>),
///                       int Function(Pointer<Void>)>('InitDartApiDL');
///   initDartApiDL(NativeApi.initializeApiDLData);
///
/// @param data Initialization data from NativeApi.initializeApiDLData
/// @return 0 on success, non-zero on failure
__declspec(dllexport) intptr_t InitDartApiDL(void* data) {
  return Dart_InitializeApiDL(data);
}

/// Register Dart SendPort for window count notifications.
///
/// Adds the port to DartPortManager's registry. When window count changes,
/// NotifyWindowCountChanged broadcasts to all registered ports.
///
/// Dart usage:
///   final receivePort = ReceivePort();
///   registerWindowCountPort(receivePort.sendPort.nativePort);
///   receivePort.listen((message) {
///     setState(() { windowCount = message as int; });
///   });
///
/// @param port Dart_Port_DL from SendPort.nativePort
/// @return true if registration successful
__declspec(dllexport) bool RegisterWindowCountPort(Dart_Port_DL port) {
  // Register port and send current window count immediately
  return g_dart_port_manager.RegisterPort(port, g_current_window_count);
}

/// Unregister Dart SendPort when no longer needed.
///
/// Removes the port from DartPortManager's registry. Should be called
/// when window is destroyed to prevent messages to stale ports.
///
/// Dart usage (in dispose()):
///   unregisterWindowCountPort(_receivePort.sendPort.nativePort);
///   _receivePort.close();
///
/// @param port Dart_Port_DL to unregister
/// @return true if port was found and removed
__declspec(dllexport) bool UnregisterWindowCountPort(Dart_Port_DL port) {
  return g_dart_port_manager.UnregisterPort(port);
}

}  // extern "C"
