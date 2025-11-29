import 'dart:io';

import 'package:flutter/material.dart';

import 'services/ffi_window_count_service.dart';
import 'widgets/window_counter_widget.dart';
import 'window_manager_ffi.dart';

void main() {
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

/// Screen that displays the current window count using FFIWindowCountService.
class WindowCounterScreen extends StatefulWidget {
  const WindowCounterScreen({super.key});

  @override
  State<WindowCounterScreen> createState() => _WindowCounterScreenState();
}

class _WindowCounterScreenState extends State<WindowCounterScreen> {
  late final FFIWindowCountService _service;

  @override
  void initState() {
    super.initState();
    _service = FFIWindowCountService();
  }

  @override
  void dispose() {
    _service.dispose();
    super.dispose();
  }

  /// Creates a new application window by launching a new instance of this app.
  Future<void> _createNewWindow() async {
    try {
      debugPrint('Creating new window...');
      debugPrint('Executable path: ${Platform.resolvedExecutable}');

      await Process.start(
        Platform.resolvedExecutable,
        [],
        mode: ProcessStartMode.detached,
      );

      debugPrint('New window launched successfully');
    } catch (e) {
      debugPrint('ERROR: Failed to create new window: $e');
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Failed to create window: $e')),
        );
      }
    }
  }

  /// Closes the current window via Win32 WM_CLOSE message.
  ///
  /// Uses FFI to send WM_CLOSE to the window, triggering proper cleanup:
  /// WM_CLOSE → WM_DESTROY → OnDestroy() → DecrementWindowCount()
  ///
  /// This ensures window count is properly decremented, unlike exit(0)
  /// which would bypass the Win32 message loop.
  void _closeCurrentWindow() {
    debugPrint('Closing current window via WM_CLOSE...');
    final ffi = WindowManagerFFI();
    ffi.closeWindow();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Multi-Window Counter'),
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
      ),
      body: Center(
        child: WindowCounterWidget(
          service: _service,
          onCreateWindow: _createNewWindow,
          onCloseWindow: _closeCurrentWindow,
        ),
      ),
    );
  }
}
