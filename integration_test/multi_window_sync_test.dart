// multi_window_sync_test.dart
//
// Integration test for multi-window synchronization.
// Tests that window counts properly sync across processes.
// Uses stream-based waiting patterns (no hardcoded delays).

import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:win_shmem_multi_window/services/ffi_window_count_service.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('Multi-Window Synchronization', () {
    test('initial window count is 1 for single window', () async {
      final service = FFIWindowCountService();

      // Set up stream listener before initialize
      final futureCount = service.windowCountStream.first;

      await service.initialize();

      final count = await futureCount.timeout(
        const Duration(seconds: 2),
        onTimeout: () => throw TestFailure(
          'Did not receive initial count within 2 seconds',
        ),
      );

      expect(count, equals(1),
          reason: 'Single window should have count of 1');

      service.dispose();
    });

    test('count increases when new window is created', () async {
      final service = FFIWindowCountService();

      // Get initial count first
      final initialCountFuture = service.windowCountStream.first;
      await service.initialize();

      final initialCount = await initialCountFuture.timeout(
        const Duration(seconds: 2),
      );
      debugPrint('Initial count: $initialCount');

      // Now set up listener for count increase
      final countIncreaseFuture = service.windowCountStream
          .firstWhere((count) => count > initialCount)
          .timeout(
            const Duration(seconds: 5),
            onTimeout: () => throw TestFailure(
              'Count did not increase within 5 seconds after launching new window. '
              'Initial count was $initialCount. '
              'This may indicate cross-process notification is not working.',
            ),
          );

      // Launch a new window (new process)
      final exePath = Platform.resolvedExecutable;
      debugPrint('Launching new window: $exePath');

      final process = await Process.start(
        exePath,
        [],
        mode: ProcessStartMode.detached,
      );
      debugPrint('Launched process PID: ${process.pid}');

      // Wait for count to increase (event-driven, not delay-based)
      final newCount = await countIncreaseFuture;

      debugPrint('New count: $newCount');

      expect(newCount, equals(initialCount + 1),
          reason: 'Count should increase by 1 when new window is created');

      service.dispose();

      // Note: The spawned process continues running
      // It will decrement count when closed manually
    });

    test('stream emits updates in real-time (no polling)', () async {
      final service = FFIWindowCountService();
      final stopwatch = Stopwatch();

      // Measure time to receive first count
      final futureCount = service.windowCountStream.first;
      stopwatch.start();

      await service.initialize();

      final count = await futureCount.timeout(const Duration(seconds: 2));
      stopwatch.stop();

      debugPrint('Received count $count in ${stopwatch.elapsedMilliseconds}ms');

      // Should receive count quickly (event-driven, not polling)
      // Allow up to 500ms for initialization overhead
      expect(stopwatch.elapsedMilliseconds, lessThan(500),
          reason: 'Count should be received quickly via event notification, '
              'not via polling. Received in ${stopwatch.elapsedMilliseconds}ms');

      expect(count, greaterThan(0));

      service.dispose();
    });

    test('stream continues to receive updates', () async {
      final service = FFIWindowCountService();
      final receivedCounts = <int>[];

      // Collect counts received
      final subscription = service.windowCountStream.listen((count) {
        receivedCounts.add(count);
        debugPrint('Received count: $count');
      });

      await service.initialize();

      // Wait for initial count
      final initialCount = await service.windowCountStream.first
          .timeout(const Duration(seconds: 2));
      debugPrint('Initial count: $initialCount');

      // Launch one additional window
      final exePath = Platform.resolvedExecutable;
      debugPrint('Launching additional window...');
      await Process.start(exePath, [], mode: ProcessStartMode.detached);

      // Wait for count to increase by 1
      final newCount = await service.windowCountStream
          .firstWhere((c) => c > initialCount)
          .timeout(
            const Duration(seconds: 5),
            onTimeout: () => throw TestFailure(
              'Count did not increase within 5 seconds. '
              'Initial: $initialCount, Received: $receivedCounts',
            ),
          );

      debugPrint('New count after window creation: $newCount');
      debugPrint('All received counts: $receivedCounts');

      // Verify we received multiple updates
      expect(receivedCounts.length, greaterThanOrEqualTo(2),
          reason: 'Should receive initial count + at least 1 update');

      // Verify count increased
      expect(newCount, equals(initialCount + 1),
          reason: 'Count should increase by exactly 1');

      await subscription.cancel();
      service.dispose();
    });
  });
}
