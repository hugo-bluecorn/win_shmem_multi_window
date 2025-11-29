// window_count_service.dart
//
// TDD GREEN Phase: Minimal implementation to pass tests.

/// Abstract interface for window count service.
abstract class WindowCountService {
  /// Stream of window count updates.
  Stream<int> get windowCountStream;

  /// Initialize the service. Returns true on success.
  Future<bool> initialize();

  /// Dispose of resources.
  void dispose();

  /// Get the last known window count.
  int? get currentCount;
}
