---
name: tdd-test
description: Run tests for TDD tasks or specific test files
---

# /tdd-test - Run Tests

## Description

Run tests for a specific test file or all tests in your project. Useful for verifying tests fail (Red phase) or pass (Green phase) during TDD development.

---

## Instructions

1. **Get the test file path** from the user (or infer from TDD task if provided)
2. **Determine the test command:**
   - Check for framework-specific test configuration
   - Use appropriate test command for the framework
   - Examples: `dart test`, `npm test`, `pytest`, `flutter test`
3. **Run tests:**
   - Execute the framework's test command
   - Capture and display full test output
   - Show test results clearly
4. **Run analysis (if applicable):**
   - Execute code analysis/linting
   - Examples: `dart analyze`, `eslint`, `pylint`
5. **Provide summary:**
   - Display test count (passed/failed)
   - Highlight failures clearly
   - Provide phase context (Red/Green/Refactor)
   - Suggest next steps

---

## Usage Examples

### Example 1: Test a specific file
```
/tdd-test test/email_validator_test.dart
```

### Example 2: Test from TDD task file
```
/tdd-test .claude/tdd-tasks/add-email-validator.md
```

(Reads test file paths from the TDD task markdown)

### Example 3: Run all tests
```
/tdd-test
```

---

## Output Format

### RED Phase Output

When tests are expected to fail:

```
🔴 RED PHASE - Tests Should Fail

Testing: test/email_validator_test.dart

Test Results:
❌ Test 1: validates valid email format - FAILED
❌ Test 2: rejects invalid email format - FAILED
❌ Test 3: handles edge cases - FAILED

Failed: 3
Passed: 0
Total: 3

✅ RED PHASE COMPLETE: Tests are failing as expected.

Next Step: Implement minimal code to pass these tests (/tdd-implement)
```

### GREEN Phase Output

When tests should pass:

```
🟢 GREEN PHASE - All Tests Passing

Testing: test/email_validator_test.dart

Test Results:
✅ Test 1: validates valid email format - PASSED
✅ Test 2: rejects invalid email format - PASSED
✅ Test 3: handles edge cases - PASSED

Passed: 3
Failed: 0
Total: 3

✅ GREEN PHASE COMPLETE: All tests are passing!

Code Analysis:
🔍 Dart Analyze: [No issues | X issues found]

Next Step: Refactor code for quality (/tdd-implement or proceed with refactoring)
```

### REFACTOR Phase Output

After refactoring:

```
🟡 REFACTOR PHASE - Verify Tests Still Pass

Testing: test/email_validator_test.dart

Test Results:
✅ Test 1: validates valid email format - PASSED
✅ Test 2: rejects invalid email format - PASSED
✅ Test 3: handles edge cases - PASSED

Passed: 3
Failed: 0
Total: 3

Code Analysis:
🟢 Dart Analyze: No issues found

✅ REFACTOR PHASE COMPLETE: Code quality improved, tests still passing!

Next Step: Commit refactoring and create PR
```

---

## Framework Test Commands

The command should detect and use appropriate test runners:

| Framework | Language | Test Command | Analysis |
|-----------|----------|--------------|----------|
| Dart/Flutter | Dart | `dart test [file]` | `dart analyze` |
| Flutter | Dart | `flutter test [file]` | `flutter analyze` |
| pytest | Python | `pytest [file]` | `pylint [file]` |
| unittest | Python | `python -m unittest` | `pylint [file]` |
| Jest | JavaScript | `npm test` or `jest [file]` | `eslint [file]` |
| Vitest | JavaScript | `npm test` or `vitest [file]` | `eslint [file]` |
| Mocha | JavaScript | `npm test` | `eslint [file]` |
| pytest | Rust | `cargo test` | `cargo clippy` |
| Go | Go | `go test ./...` | `go vet` |

**Note:** Users can specify the test command in their TDD task file if needed.

---

## File Path Resolution

### If test file path provided:
```
/tdd-test test/features/email_validator_test.dart
```
→ Runs: `dart test test/features/email_validator_test.dart`

### If TDD task file provided:
```
/tdd-test .claude/tdd-tasks/add-email-validator.md
```
→ Read test file paths from markdown
→ Run tests for those files

### If no path provided:
```
/tdd-test
```
→ Run all tests in the project

---

## Expected Outputs

### Test Output
```
✅ 10 tests passed
❌ 2 tests failed
⏭️  5 tests skipped

FAILURES:
[Stack trace and error messages]
```

### Analysis Output
```
🔍 Code Analysis Results

[File]: [Line] [Issue]
[File]: [Line] [Issue]

Total Issues: 3
Errors: 0
Warnings: 3
```

---

## Phase Context

Help users understand test results in TDD context:

### RED Phase ❌
- Tests are expected to fail
- This is correct and expected
- Failing tests prove the test is working
- Next step: Implement minimal code

### GREEN Phase ✅
- All tests should pass
- This proves implementation works
- All test cases are satisfied
- Next step: Refactor or move to next feature

### REFACTOR Phase ✅
- Tests must remain passing
- Refactoring improves code quality
- If tests fail, revert changes
- Next step: Commit and create PR

---

## Common Test Scenarios

### Scenario 1: Running Tests in RED Phase

```
User: /tdd-test
Output shows: ❌ 3 tests failed
Response: "Perfect! Tests are failing as expected. Now implement
minimal code to pass them."
```

### Scenario 2: Running Tests in GREEN Phase

```
User: /tdd-test
Output shows: ✅ 3 tests passed
Response: "Excellent! All tests passing. You can now refactor
the code for better quality while keeping tests green."
```

### Scenario 3: Test Failures During Development

```
User: /tdd-test
Output shows: ❌ 1 of 3 tests failed
Response: "One test is still failing. Review the error message
and adjust implementation to pass this test too."
```

### Scenario 4: Linting Issues Found

```
User: /tdd-test
Tests: ✅ All passing
Analysis: ⚠️ 5 issues found
Response: "Tests pass but code has linting issues. Fix these
during refactoring phase."
```

---

## Troubleshooting

### Tests not found
- Check file path spelling
- Verify test files exist
- Check that test files follow naming convention (e.g., `*_test.dart`)
- Run with no arguments to test all: `/tdd-test`

### Test command not found
- Verify test dependencies are installed
- Check that test framework is configured
- Consult framework-specific TDD guide
- May need to install dependencies first

### Cryptic error messages
- Show full error output
- Provide suggestions based on error type
- Link to framework documentation
- Ask user to review implementation

### Performance issues
- Tests are slow - use mocks/stubs
- Too many tests - run specific file
- Memory issues - check for infinite loops
- Suggest: Run single test file first

---

## Integration with /tdd-implement

This command is called within `/tdd-implement` to:

1. **Verify RED phase:** Run tests, confirm they fail
2. **Verify GREEN phase:** Run tests, confirm they pass
3. **Verify REFACTOR phase:** Run tests, confirm still passing

Users can also run this independently to:
- Quick verification during development
- Check specific file tests
- Monitor code quality with analysis
- Validate changes before commit

---

## Important Notes

### Output Clarity
- Show failing vs passing tests distinctly
- Use emojis/symbols for quick scanning
- Include actionable next steps
- Provide error details for debugging

### Framework Agnostic
- Support multiple frameworks
- Detect appropriate test runner
- Allow custom test commands
- Provide framework-specific guides

### Phase Awareness
- Remind user of current TDD phase
- Explain what results mean
- Suggest next steps
- Link to TDD guidelines

### Git Integration
- Test often during development
- Commit after confirming tests pass
- Use for CI/CD validation
- Support pre-commit hooks

---

## References

- **TDD Guidelines:** `docs/flutter-tdd-guide.md`
- **TDD Implement Command:** `.claude/commands/tdd-implement.md`
- **Version Control:** `docs/version-control.md`

---

**Created:** 2025-11-23
