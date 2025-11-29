// window_counter_widget_test.dart
//
// TDD RED Phase: Widget tests for WindowCounterWidget.
// Tests define expected UI behavior with mock service.

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

// This import should FAIL - widget doesn't exist yet
import 'package:win_shmem_multi_window/widgets/window_counter_widget.dart';
import 'package:win_shmem_multi_window/services/mock_window_count_service.dart';

void main() {
  group('WindowCounterWidget', () {
    late MockWindowCountService mockService;

    setUp(() {
      mockService = MockWindowCountService();
    });

    tearDown(() {
      mockService.dispose();
    });

    testWidgets('displays window count from service',
        (WidgetTester tester) async {
      mockService = MockWindowCountService(initialCount: 3);

      await tester.pumpWidget(MaterialApp(
        home: Scaffold(
          body: WindowCounterWidget(service: mockService),
        ),
      ));
      await tester.pumpAndSettle();

      expect(find.text('3'), findsOneWidget);
    });

    testWidgets('updates display when count changes',
        (WidgetTester tester) async {
      mockService = MockWindowCountService(initialCount: 1);

      await tester.pumpWidget(MaterialApp(
        home: Scaffold(
          body: WindowCounterWidget(service: mockService),
        ),
      ));
      await tester.pumpAndSettle();

      expect(find.text('1'), findsOneWidget);

      mockService.simulateCountChange(5);
      await tester.pumpAndSettle();

      expect(find.text('5'), findsOneWidget);
    });

    testWidgets('has Create New Window button',
        (WidgetTester tester) async {
      await tester.pumpWidget(MaterialApp(
        home: Scaffold(
          body: WindowCounterWidget(service: mockService),
        ),
      ));
      await tester.pumpAndSettle();

      expect(find.text('Create New Window'), findsOneWidget);
    });

    testWidgets('has Close This Window button',
        (WidgetTester tester) async {
      await tester.pumpWidget(MaterialApp(
        home: Scaffold(
          body: WindowCounterWidget(service: mockService),
        ),
      ));
      await tester.pumpAndSettle();

      expect(find.text('Close This Window'), findsOneWidget);
    });

    testWidgets('Create button calls onCreateWindow callback',
        (WidgetTester tester) async {
      var called = false;

      await tester.pumpWidget(MaterialApp(
        home: Scaffold(
          body: WindowCounterWidget(
            service: mockService,
            onCreateWindow: () => called = true,
          ),
        ),
      ));
      await tester.pumpAndSettle();

      await tester.tap(find.text('Create New Window'));
      await tester.pumpAndSettle();

      expect(called, isTrue);
    });
  });
}
