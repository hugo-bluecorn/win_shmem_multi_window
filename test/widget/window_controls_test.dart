import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:win_shmem_multi_window/main.dart';

void main() {
  group('Window Control Buttons', () {
    testWidgets('Create New Window button exists', (WidgetTester tester) async {
      // Build the WindowCounterScreen
      await tester.pumpWidget(const MaterialApp(
        home: WindowCounterScreen(),
      ));

      // Wait for async initialization
      await tester.pumpAndSettle();

      // Find the "Create New Window" button
      final createButton = find.text('Create New Window');

      // Verify button exists
      expect(createButton, findsOneWidget);

      // Verify it's an ElevatedButton or FilledButton
      final buttonWidget = tester.widget(find.ancestor(
        of: createButton,
        matching: find.byType(ElevatedButton),
      ));
      expect(buttonWidget, isA<ElevatedButton>());
    });

    testWidgets('Close This Window button exists', (WidgetTester tester) async {
      // Build the WindowCounterScreen
      await tester.pumpWidget(const MaterialApp(
        home: WindowCounterScreen(),
      ));

      // Wait for async initialization
      await tester.pumpAndSettle();

      // Find the "Close This Window" button
      final closeButton = find.text('Close This Window');

      // Verify button exists
      expect(closeButton, findsOneWidget);

      // Verify it's an ElevatedButton or OutlinedButton
      final buttonWidget = tester.widget(find.ancestor(
        of: closeButton,
        matching: find.byType(ElevatedButton),
      ));
      expect(buttonWidget, isA<ElevatedButton>());
    });

    testWidgets('Create New Window button is tappable',
        (WidgetTester tester) async {
      // Build the WindowCounterScreen
      await tester.pumpWidget(const MaterialApp(
        home: WindowCounterScreen(),
      ));

      // Wait for async initialization
      await tester.pumpAndSettle();

      // Find and tap the "Create New Window" button
      final createButton = find.text('Create New Window');
      expect(createButton, findsOneWidget);

      // Tap the button - should not throw exception
      await tester.tap(createButton);
      await tester.pump();

      // If we get here without exception, test passes
      expect(createButton, findsOneWidget);
    });

    testWidgets('Close This Window button is tappable',
        (WidgetTester tester) async {
      // Build the WindowCounterScreen
      await tester.pumpWidget(const MaterialApp(
        home: WindowCounterScreen(),
      ));

      // Wait for async initialization
      await tester.pumpAndSettle();

      // Find and tap the "Close This Window" button
      final closeButton = find.text('Close This Window');
      expect(closeButton, findsOneWidget);

      // Tap the button - should not throw exception
      await tester.tap(closeButton);
      await tester.pump();

      // If we get here without exception, test passes
      expect(closeButton, findsOneWidget);
    });

    testWidgets('Buttons have proper layout and spacing',
        (WidgetTester tester) async {
      // Build the WindowCounterScreen
      await tester.pumpWidget(const MaterialApp(
        home: WindowCounterScreen(),
      ));

      // Wait for async initialization
      await tester.pumpAndSettle();

      // Find both buttons
      final createButton = find.text('Create New Window');
      final closeButton = find.text('Close This Window');

      expect(createButton, findsOneWidget);
      expect(closeButton, findsOneWidget);

      // Verify buttons are in a Column layout
      final column = find.ancestor(
        of: createButton,
        matching: find.byType(Column),
      );
      expect(column, findsWidgets);

      // Verify SizedBox spacing exists between buttons
      final sizedBoxes = find.byType(SizedBox);
      expect(sizedBoxes, findsWidgets);

      // Get button positions to verify they're vertically stacked
      final createButtonPos = tester.getCenter(createButton);
      final closeButtonPos = tester.getCenter(closeButton);

      // Create button should be above close button
      expect(createButtonPos.dy < closeButtonPos.dy, isTrue);
    });

    testWidgets('Error handling for failed window creation',
        (WidgetTester tester) async {
      // Build the WindowCounterScreen
      await tester.pumpWidget(const MaterialApp(
        home: WindowCounterScreen(),
      ));

      // Wait for async initialization
      await tester.pumpAndSettle();

      // This test verifies that if Process.start() fails,
      // the app doesn't crash and shows a SnackBar.
      // We can't easily mock Process.start() in widget tests,
      // so this test primarily verifies the button exists
      // and can be tapped without crashing the app.

      final createButton = find.text('Create New Window');
      expect(createButton, findsOneWidget);

      // Tap the button
      await tester.tap(createButton);
      await tester.pump();

      // App should not crash (if we get here, test passes)
      expect(find.byType(WindowCounterScreen), findsOneWidget);
    });
  });
}
