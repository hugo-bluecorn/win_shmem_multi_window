// mock_window_count_service.dart
//
// TDD GREEN Phase: Minimal implementation to pass tests.

import 'dart:async';

import 'window_count_service.dart';

/// Mock implementation of WindowCountService for testing.
class MockWindowCountService implements WindowCountService {
  final StreamController<int> _controller = StreamController<int>.broadcast();
  int? _currentCount;
  final int _initialCount;

  MockWindowCountService({int initialCount = 1}) : _initialCount = initialCount;

  @override
  Stream<int> get windowCountStream => _controller.stream;

  @override
  int? get currentCount => _currentCount;

  @override
  Future<bool> initialize() async {
    _currentCount = _initialCount;
    _controller.add(_initialCount);
    return true;
  }

  @override
  void dispose() {
    _controller.close();
  }

  /// Simulate a count change for testing.
  void simulateCountChange(int newCount) {
    _currentCount = newCount;
    _controller.add(newCount);
  }
}
