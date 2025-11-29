// window_counter_widget.dart
//
// TDD GREEN Phase: Minimal implementation to pass tests.

import 'dart:async';

import 'package:flutter/material.dart';

import '../services/window_count_service.dart';

/// Widget that displays window count with dependency injection.
class WindowCounterWidget extends StatefulWidget {
  final WindowCountService service;
  final VoidCallback? onCreateWindow;
  final VoidCallback? onCloseWindow;

  const WindowCounterWidget({
    super.key,
    required this.service,
    this.onCreateWindow,
    this.onCloseWindow,
  });

  @override
  State<WindowCounterWidget> createState() => _WindowCounterWidgetState();
}

class _WindowCounterWidgetState extends State<WindowCounterWidget> {
  int _count = 0;
  StreamSubscription<int>? _subscription;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    // Subscribe BEFORE initialize to catch initial value
    _subscription = widget.service.windowCountStream.listen((count) {
      if (mounted) {
        setState(() => _count = count);
      }
    });
    await widget.service.initialize();
  }

  @override
  void dispose() {
    _subscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Text('$_count', style: const TextStyle(fontSize: 48)),
        const SizedBox(height: 20),
        ElevatedButton(
          onPressed: widget.onCreateWindow,
          child: const Text('Create New Window'),
        ),
        const SizedBox(height: 10),
        ElevatedButton(
          onPressed: widget.onCloseWindow,
          child: const Text('Close This Window'),
        ),
      ],
    );
  }
}
