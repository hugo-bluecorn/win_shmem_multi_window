// mock_window_count_service_test.dart
//
// TDD RED Phase: Tests for MockWindowCountService.
// Defines expected behavior for testable mock implementation.

import 'package:flutter_test/flutter_test.dart';

// This import should FAIL - class doesn't exist yet
import 'package:win_shmem_multi_window/services/mock_window_count_service.dart';
import 'package:win_shmem_multi_window/services/window_count_service.dart';

void main() {
  group('MockWindowCountService', () {
    test('implements WindowCountService', () {
      final service = MockWindowCountService();
      expect(service, isA<WindowCountService>());
      service.dispose();
    });

    test('initialize returns true', () async {
      final service = MockWindowCountService();
      final result = await service.initialize();
      expect(result, isTrue);
      service.dispose();
    });

    test('emits initial count after initialize', () async {
      final service = MockWindowCountService(initialCount: 1);
      final counts = <int>[];
      service.windowCountStream.listen(counts.add);

      await service.initialize();
      await Future.delayed(Duration.zero);

      expect(counts, contains(1));
      service.dispose();
    });

    test('currentCount is null before initialize', () {
      final service = MockWindowCountService();
      expect(service.currentCount, isNull);
      service.dispose();
    });

    test('currentCount equals initial count after initialize', () async {
      final service = MockWindowCountService(initialCount: 5);
      await service.initialize();
      expect(service.currentCount, equals(5));
      service.dispose();
    });

    test('simulateCountChange updates currentCount', () async {
      final service = MockWindowCountService();
      await service.initialize();

      service.simulateCountChange(10);

      expect(service.currentCount, equals(10));
      service.dispose();
    });

    test('simulateCountChange emits on stream', () async {
      final service = MockWindowCountService();
      await service.initialize();

      final counts = <int>[];
      service.windowCountStream.listen(counts.add);

      service.simulateCountChange(7);
      await Future.delayed(Duration.zero);

      expect(counts, contains(7));
      service.dispose();
    });
  });
}
