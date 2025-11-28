// dart_port_manager.h
//
// DartPortManager: Manages communication from C++ to Dart isolates.
//
// Layer 3 of the event-driven multi-window IPC architecture. Maintains a
// registry of Dart SendPort handles and broadcasts window count updates to
// all registered Dart isolates using Dart_PostCObject().
//
// Thread Safety: All public methods are thread-safe using std::mutex.

#ifndef RUNNER_DART_PORT_MANAGER_H_
#define RUNNER_DART_PORT_MANAGER_H_

#include <dart_api_dl.h>
#include <windows.h>

#include <mutex>
#include <vector>

/// Manages communication from C++ to Dart isolates via Dart C API.
///
/// DartPortManager maintains a registry of Dart SendPort handles and
/// broadcasts window count updates to all registered Dart isolates using
/// Dart_PostCObject(). This enables zero-latency UI updates without polling.
///
/// Architecture:
///   WindowCountListener (Layer 2) → DartPortManager::NotifyWindowCountChanged()
///                                  → Dart_PostCObject() to all ports
///                                  → Dart ReceivePort.listen()
///                                  → setState()
///
/// Thread Safety: All public methods are thread-safe using std::mutex.
///
/// Usage:
///   1. Dart creates ReceivePort and gets SendPort.nativePort
///   2. Dart calls RegisterWindowCountPort(port) via FFI
///   3. WindowCountListener calls NotifyWindowCountChanged(count)
///   4. DartPortManager broadcasts to all registered ports
///   5. Dart ReceivePort.listen() receives message
///   6. Dart calls setState() to update UI
class DartPortManager {
 public:
  DartPortManager();
  ~DartPortManager();

  /// Registers a Dart SendPort for receiving window count notifications.
  ///
  /// Adds the port to the internal registry. When NotifyWindowCountChanged()
  /// is called, all registered ports will receive the update via
  /// Dart_PostCObject().
  ///
  /// Thread-safe: Can be called from FFI thread.
  ///
  /// @param port Dart_Port_DL obtained from SendPort.nativePort in Dart
  /// @return true if registration successful
  bool RegisterPort(Dart_Port_DL port);

  /// Unregisters a previously registered Dart SendPort.
  ///
  /// Removes the port from the internal registry. The port will no longer
  /// receive window count notifications.
  ///
  /// Thread-safe: Can be called from FFI thread.
  ///
  /// @param port Dart_Port_DL to remove from registry
  /// @return true if port was found and removed, false if not found
  bool UnregisterPort(Dart_Port_DL port);

  /// Broadcasts window count update to all registered Dart ports.
  ///
  /// Creates Dart_CObject message with the new window count and calls
  /// Dart_PostCObject() for each registered port. This is called from
  /// WindowCountListener callback when the event is signaled.
  ///
  /// Error Handling: If Dart_PostCObject fails for a port, logs an error
  /// and continues broadcasting to remaining ports.
  ///
  /// Thread-safe: Can be called from background thread.
  ///
  /// @param new_count Current window count from SharedMemoryManager
  void NotifyWindowCountChanged(LONG new_count);

 private:
  /// Registered Dart SendPort handles.
  /// Protected by ports_mutex_ for thread-safe access.
  std::vector<Dart_Port_DL> ports_;

  /// Mutex protecting ports_ vector.
  /// Ensures thread-safe registration, unregistration, and broadcasting.
  std::mutex ports_mutex_;
};

// Get global DartPortManager instance for C++ code.
// This allows WindowCountListener to call NotifyWindowCountChanged.
DartPortManager& GetGlobalDartPortManager();

// FFI Exports for Dart binding
extern "C" {

/// FFI export: Initialize Dart API DL.
///
/// Must be called once from Dart before using any Dart C API functions:
///   final init = DynamicLibrary.process()
///       .lookupFunction<InitDartApiDLNative, InitDartApiDLDart>(
///           'InitDartApiDL');
///   init(NativeApi.initializeApiDLData);
///
/// @param data Dart_InitializeApiDL_data from NativeApi.initializeApiDLData
/// @return 0 on success, non-zero on failure
__declspec(dllexport) intptr_t InitDartApiDL(void* data);

/// FFI export: Register Dart SendPort for window count notifications.
///
/// Called from Dart via FFI:
///   final registerPort = DynamicLibrary.process()
///       .lookupFunction<RegisterPortNative, RegisterPortDart>(
///           'RegisterWindowCountPort');
///   registerPort(sendPort.nativePort);
///
/// @param port Dart_Port_DL from SendPort.nativePort
/// @return true if registration successful
__declspec(dllexport) bool RegisterWindowCountPort(Dart_Port_DL port);

/// FFI export: Unregister Dart SendPort.
///
/// Called from Dart via FFI when window is destroyed:
///   unregisterPort(sendPort.nativePort);
///
/// @param port Dart_Port_DL to unregister
/// @return true if port was found and removed
__declspec(dllexport) bool UnregisterWindowCountPort(Dart_Port_DL port);

}  // extern "C"

#endif  // RUNNER_DART_PORT_MANAGER_H_
