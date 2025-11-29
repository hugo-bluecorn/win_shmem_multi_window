import 'dart:io';
import 'dart:isolate';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'window_manager_ffi.dart';

void main() {
  // Initialize Dart API DL before running the app.
  // This MUST be done before any Dart C API functions are called.
  final ffi = WindowManagerFFI();
  final result = ffi.initializeDartApi();

  if (result != 0) {
    debugPrint('ERROR: Failed to initialize Dart API DL: $result');
    // Continue anyway - app will run but FFI won't work
  } else {
    debugPrint('Dart API DL initialized successfully');
  }

  runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Multi-Window Counter',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        useMaterial3: true,
      ),
      home: const WindowCounterScreen(),
    );
  }
}

/// Screen that displays the current window count.
///
/// Receives real-time updates from C++ layer via ReceivePort.
/// Window count is synchronized across all windows using shared memory.
class WindowCounterScreen extends StatefulWidget {
  const WindowCounterScreen({super.key});

  @override
  State<WindowCounterScreen> createState() => _WindowCounterScreenState();
}

class _WindowCounterScreenState extends State<WindowCounterScreen> {
  WindowManagerFFI? _ffi;
  ReceivePort? _receivePort;

  int _windowCount = 0;
  String _status = 'Initializing...';

  @override
  void initState() {
    super.initState();
    _initializeFFI();
  }

  void _initializeFFI() {
    try {
      // Create FFI instance
      _ffi = WindowManagerFFI();

      // Create ReceivePort to receive window count updates from C++
      _receivePort = ReceivePort();

      // Set up listener for incoming messages
      _receivePort!.listen((message) {
        debugPrint('Dart: Received window count update: $message');

        // Update UI with new window count
        setState(() {
          _windowCount = message as int;
          _status = 'Connected';
        });
      });

      // Register SendPort with C++ layer
      final registered = _ffi!.registerWindowCountPort(_receivePort!.sendPort);

      if (registered) {
        debugPrint('Dart: Successfully registered window count port');
        setState(() {
          _status = 'Listening for updates...';
        });
      } else {
        debugPrint('ERROR: Failed to register window count port');
        setState(() {
          _status = 'Registration failed';
        });
      }
    } catch (e) {
      debugPrint('ERROR: Failed to initialize FFI: $e');
      setState(() {
        _status = 'Error: $e';
      });
    }
  }

  @override
  void dispose() {
    // Unregister port from C++ layer
    if (_ffi != null && _receivePort != null) {
      try {
        final unregistered = _ffi!.unregisterWindowCountPort(_receivePort!.sendPort);
        if (unregistered) {
          debugPrint('Dart: Successfully unregistered window count port');
        } else {
          debugPrint('WARNING: Port not found during unregistration');
        }
      } catch (e) {
        debugPrint('ERROR: Failed to unregister port: $e');
      }

      // Close ReceivePort
      _receivePort!.close();
    }

    super.dispose();
  }

  /// Creates a new application window by launching a new Flutter instance.
  ///
  /// In development mode, runs `flutter run -d windows`.
  /// In production mode, launches the built executable.
  Future<void> _createNewWindow() async {
    try {
      debugPrint('Creating new window...');

      // Detect debug vs production mode
      const bool isProduction = bool.fromEnvironment('dart.vm.product');

      if (isProduction) {
        // Production: Launch the built executable
        await Process.start(
          Platform.resolvedExecutable,
          [],
          mode: ProcessStartMode.detached,
        );
        debugPrint('New window launched (production mode)');
      } else {
        // Development: Launch flutter run
        await Process.start(
          'flutter',
          ['run', '-d', 'windows'],
          workingDirectory: Directory.current.path,
          mode: ProcessStartMode.detached,
        );
        debugPrint('New window launched (development mode)');
      }
    } catch (e) {
      debugPrint('ERROR: Failed to create new window: $e');
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to create window: $e')),
        );
      }
    }
  }

  /// Closes the current window.
  ///
  /// Calls SystemNavigator.pop() which triggers FlutterWindow::OnDestroy()
  /// for proper cleanup and window count decrement.
  void _closeCurrentWindow() {
    debugPrint('Closing current window...');
    SystemNavigator.pop();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Multi-Window Counter'),
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Text(
              'Active Windows:',
              style: TextStyle(fontSize: 24),
            ),
            const SizedBox(height: 20),
            Text(
              '$_windowCount',
              style: Theme.of(context).textTheme.displayLarge?.copyWith(
                    color: Theme.of(context).colorScheme.primary,
                    fontWeight: FontWeight.bold,
                  ),
            ),
            const SizedBox(height: 40),
            Container(
              padding: const EdgeInsets.all(16),
              decoration: BoxDecoration(
                color: _status == 'Connected'
                    ? Colors.green.withValues(alpha: 0.1)
                    : Colors.orange.withValues(alpha: 0.1),
                borderRadius: BorderRadius.circular(8),
                border: Border.all(
                  color: _status == 'Connected'
                      ? Colors.green
                      : Colors.orange,
                ),
              ),
              child: Column(
                children: [
                  Icon(
                    _status == 'Connected'
                        ? Icons.check_circle
                        : Icons.sync,
                    color: _status == 'Connected'
                        ? Colors.green
                        : Colors.orange,
                    size: 32,
                  ),
                  const SizedBox(height: 8),
                  Text(
                    _status,
                    style: TextStyle(
                      fontSize: 16,
                      color: _status == 'Connected'
                          ? Colors.green
                          : Colors.orange,
                    ),
                  ),
                ],
              ),
            ),
            const SizedBox(height: 40),
            ElevatedButton(
              onPressed: _createNewWindow,
              child: const Text('Create New Window'),
            ),
            const SizedBox(height: 16),
            ElevatedButton(
              onPressed: _closeCurrentWindow,
              child: const Text('Close This Window'),
            ),
            const SizedBox(height: 40),
            const Text(
              'Use the buttons above to create or close windows',
              style: TextStyle(
                fontSize: 14,
                fontStyle: FontStyle.italic,
                color: Colors.grey,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
