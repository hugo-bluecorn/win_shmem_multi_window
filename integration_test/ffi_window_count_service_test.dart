// ffi_window_count_service_test.dart
//
// Integration test for FFIWindowCountService.
// Tests the real FFI connection to C++ layer.
// Uses stream-based waiting patterns (no hardcoded delays).
// Must run with: flutter test integration_test/

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:win_shmem_multi_window/services/ffi_window_count_service.dart';
import 'package:win_shmem_multi_window/services/window_count_service.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('FFIWindowCountService Integration', () {
    test('implements WindowCountService', () {
      final service = FFIWindowCountService();
      expect(service, isA<WindowCountService>());
      service.dispose();
    });

    test('initialize returns true when native layer is available', () async {
      final service = FFIWindowCountService();
      final result = await service.initialize();
      expect(result, isTrue);
      service.dispose();
    });

    test('windowCountStream emits after initialize', () async {
      final service = FFIWindowCountService();

      // Set up future BEFORE initialize to catch the emission
      final futureCount = service.windowCountStream.first;

      await service.initialize();

      // Wait for actual stream emission (with timeout as safety net)
      final count = await futureCount.timeout(
        const Duration(seconds: 2),
        onTimeout: () => throw TestFailure(
          'windowCountStream did not emit within 2 seconds after initialize',
        ),
      );

      expect(count, greaterThanOrEqualTo(1),
          reason: 'Should receive count >= 1 (at least this window exists)');

      service.dispose();
    });

    test('currentCount is set after stream emits', () async {
      final service = FFIWindowCountService();

      expect(service.currentCount, isNull,
          reason: 'currentCount should be null before initialize');

      // Set up future BEFORE initialize
      final futureCount = service.windowCountStream.first;

      await service.initialize();

      // Wait for stream to emit (which also sets currentCount)
      await futureCount.timeout(
        const Duration(seconds: 2),
        onTimeout: () => throw TestFailure(
          'windowCountStream did not emit within 2 seconds',
        ),
      );

      expect(service.currentCount, isNotNull,
          reason: 'currentCount should be set after stream emits');
      expect(service.currentCount, greaterThanOrEqualTo(1));

      service.dispose();
    });

    test('stream emits valid count (not zero or negative)', () async {
      final service = FFIWindowCountService();

      final futureCount = service.windowCountStream.first;
      await service.initialize();

      final count = await futureCount.timeout(const Duration(seconds: 2));

      expect(count, isA<int>());
      expect(count, greaterThan(0),
          reason: 'Window count should always be positive');

      service.dispose();
    });
  });
}
