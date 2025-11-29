// ffi_window_count_service.dart
//
// TDD GREEN Phase: FFI-based implementation of WindowCountService.
// Connects to C++ layer via WindowManagerFFI.

import 'dart:async';
import 'dart:isolate';

import 'package:flutter/foundation.dart';

import '../window_manager_ffi.dart';
import 'window_count_service.dart';

/// FFI-based implementation of WindowCountService.
///
/// Connects to the C++ DartPortManager to receive window count updates.
class FFIWindowCountService implements WindowCountService {
  WindowManagerFFI? _ffi;
  ReceivePort? _receivePort;
  final StreamController<int> _controller = StreamController<int>.broadcast();
  int? _currentCount;
  bool _isInitialized = false;

  @override
  Stream<int> get windowCountStream => _controller.stream;

  @override
  int? get currentCount => _currentCount;

  @override
  Future<bool> initialize() async {
    if (_isInitialized) {
      return true;
    }

    try {
      _ffi = WindowManagerFFI();

      // Initialize Dart API DL (required before any FFI calls)
      final apiResult = _ffi!.initializeDartApi();
      if (apiResult != 0) {
        debugPrint('FFIWindowCountService: Failed to init Dart API: $apiResult');
        return false;
      }

      _receivePort = ReceivePort();

      // Listen for updates from C++ layer
      _receivePort!.listen((message) {
        debugPrint('FFIWindowCountService: Received count: $message');
        final count = message as int;
        _currentCount = count;
        _controller.add(count);
      });

      // Register port with C++ layer
      final registered = _ffi!.registerWindowCountPort(_receivePort!.sendPort);

      if (registered) {
        debugPrint('FFIWindowCountService: Port registered');
        _isInitialized = true;
        return true;
      } else {
        debugPrint('FFIWindowCountService: Failed to register port');
        return false;
      }
    } catch (e) {
      debugPrint('FFIWindowCountService: Error during initialization: $e');
      return false;
    }
  }

  @override
  void dispose() {
    if (_ffi != null && _receivePort != null) {
      try {
        _ffi!.unregisterWindowCountPort(_receivePort!.sendPort);
      } catch (e) {
        debugPrint('FFIWindowCountService: Error unregistering port: $e');
      }
      _receivePort!.close();
    }
    _controller.close();
    _isInitialized = false;
  }
}
