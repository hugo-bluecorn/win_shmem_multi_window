// window_manager_ffi.dart
//
// Dart FFI bindings for DartPortManager C++ layer.
// Enables Dart isolates to receive window count updates via ReceivePort.

import 'dart:ffi';
import 'dart:isolate';

// FFI function signatures (C side)
typedef InitDartApiDLNative = IntPtr Function(Pointer<Void>);
typedef RegisterWindowCountPortNative = Bool Function(Int64);
typedef UnregisterWindowCountPortNative = Bool Function(Int64);

// FFI function signatures (Dart side)
typedef InitDartApiDLDart = int Function(Pointer<Void>);
typedef RegisterWindowCountPortDart = bool Function(int);
typedef UnregisterWindowCountPortDart = bool Function(int);

/// WindowManagerFFI provides access to C++ DartPortManager functions.
///
/// This class enables Dart isolates to:
/// 1. Initialize Dart API DL (required before any Dart C API usage)
/// 2. Register ReceivePort to receive window count notifications
/// 3. Unregister ReceivePort when no longer needed
///
/// Architecture Flow:
///   Dart ReceivePort → RegisterWindowCountPort (FFI) →
///   DartPortManager (C++) → Dart_PostCObject_DL →
///   ReceivePort.listen() callback → setState()
class WindowManagerFFI {
  // Late-initialized FFI functions
  late final InitDartApiDLDart _initDartApiDL;
  late final RegisterWindowCountPortDart _registerWindowCountPort;
  late final UnregisterWindowCountPortDart _unregisterWindowCountPort;

  WindowManagerFFI() {
    // Load the native library (process = current executable)
    final DynamicLibrary nativeLib = DynamicLibrary.process();

    // Lookup FFI function pointers
    _initDartApiDL = nativeLib.lookupFunction<InitDartApiDLNative,
        InitDartApiDLDart>('InitDartApiDL');

    _registerWindowCountPort = nativeLib.lookupFunction<
        RegisterWindowCountPortNative,
        RegisterWindowCountPortDart>('RegisterWindowCountPort');

    _unregisterWindowCountPort = nativeLib.lookupFunction<
        UnregisterWindowCountPortNative,
        UnregisterWindowCountPortDart>('UnregisterWindowCountPort');
  }

  /// Initialize Dart API DL.
  ///
  /// MUST be called once before using any Dart C API functions.
  /// Initializes function pointers (Dart_PostCObject_DL, etc.) in C++ layer.
  ///
  /// Returns 0 on success, non-zero on failure.
  ///
  /// Example:
  ///   final ffi = WindowManagerFFI();
  ///   final result = ffi.initializeDartApi();
  ///   if (result != 0) {
  ///     print('Failed to initialize Dart API: $result');
  ///   }
  int initializeDartApi() {
    return _initDartApiDL(NativeApi.initializeApiDLData);
  }

  /// Register a Dart SendPort to receive window count notifications.
  ///
  /// When window count changes, C++ layer will call Dart_PostCObject_DL
  /// to send the new count to this port. The ReceivePort.listen() callback
  /// will receive the count as an int.
  ///
  /// Returns true if registration successful.
  ///
  /// Example:
  ///   final receivePort = ReceivePort();
  ///   ffi.registerWindowCountPort(receivePort.sendPort);
  ///   receivePort.listen((message) {
  ///     final count = message as int;
  ///     setState(() { windowCount = count; });
  ///   });
  bool registerWindowCountPort(SendPort sendPort) {
    return _registerWindowCountPort(sendPort.nativePort);
  }

  /// Unregister a previously registered SendPort.
  ///
  /// Should be called when window is destroyed to prevent messages
  /// to stale ports. Call in dispose() method.
  ///
  /// Returns true if port was found and removed.
  ///
  /// Example (in State.dispose()):
  ///   ffi.unregisterWindowCountPort(_receivePort.sendPort);
  ///   _receivePort.close();
  bool unregisterWindowCountPort(SendPort sendPort) {
    return _unregisterWindowCountPort(sendPort.nativePort);
  }
}
