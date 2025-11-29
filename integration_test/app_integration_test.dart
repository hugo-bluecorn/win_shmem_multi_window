// app_integration_test.dart
//
// Integration test for refactored main.dart.
// Tests that the app uses FFIWindowCountService and WindowCounterWidget.
// Uses stream-based waiting patterns for count verification.

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:win_shmem_multi_window/main.dart';
import 'package:win_shmem_multi_window/widgets/window_counter_widget.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('App Integration', () {
    testWidgets('displays WindowCounterWidget', (tester) async {
      await tester.pumpWidget(const MainApp());
      await tester.pumpAndSettle();

      expect(find.byType(WindowCounterWidget), findsOneWidget);
    });

    testWidgets('displays window count from native layer', (tester) async {
      await tester.pumpWidget(const MainApp());

      // Pump frames until count is displayed (stream-based approach)
      // Keep pumping until we see a valid count or timeout
      int? displayedCount;
      final stopwatch = Stopwatch()..start();
      const timeout = Duration(seconds: 3);

      while (displayedCount == null && stopwatch.elapsed < timeout) {
        await tester.pump(const Duration(milliseconds: 50));

        final countFinder = find.byWidgetPredicate(
          (widget) =>
              widget is Text &&
              widget.style?.fontSize == 48 &&
              int.tryParse(widget.data ?? '') != null &&
              int.parse(widget.data!) > 0,
        );

        if (countFinder.evaluate().isNotEmpty) {
          final countWidget = tester.widget<Text>(countFinder);
          displayedCount = int.parse(countWidget.data!);
        }
      }

      expect(displayedCount, isNotNull,
          reason: 'Count should be displayed within $timeout');
      expect(displayedCount, greaterThanOrEqualTo(1),
          reason: 'Count should be at least 1 (this window)');
    });

    testWidgets('has Create New Window button', (tester) async {
      await tester.pumpWidget(const MainApp());
      await tester.pumpAndSettle();

      expect(find.text('Create New Window'), findsOneWidget);
    });

    testWidgets('has Close This Window button', (tester) async {
      await tester.pumpWidget(const MainApp());
      await tester.pumpAndSettle();

      expect(find.text('Close This Window'), findsOneWidget);
    });
  });
}
