---
name: tdd-implement
description: Execute full TDD workflow with Red-Green-Refactor cycle
---

# /tdd-implement - Test-Driven Implementation Workflow

## Description

Execute a complete Test-Driven Development (TDD) workflow for implementing a new feature. Guides you through the Red-Green-Refactor cycle with proper git integration and clean commits based on zscott/pane methodology.

---

## Overview

This command orchestrates the entire TDD process:

1. **RED Phase:** Write failing tests
2. **GREEN Phase:** Implement minimal code to pass tests
3. **REFACTOR Phase:** Improve code quality
4. **GIT:** Create clean commits for each phase
5. **PR:** Create pull request when done

---

## Instructions

### Preparation

Before running `/tdd-implement`, ensure:
- [ ] You've created a TDD task file (use `/tdd-new` if not)
- [ ] You're on the correct git branch
- [ ] You've read relevant documentation
- [ ] You understand the feature requirements

### Execution

```
/tdd-implement .claude/tdd-tasks/[feature-name].md
```

Or let the command guide you through creating a new TDD task:

```
/tdd-implement
```

---

## Full Workflow

## PHASE 1: RED - Write Failing Tests

### Step 1.1: Review Requirements

```
📋 Reading TDD Task File: .claude/tdd-tasks/[feature-name].md

Feature: [Feature Name]
Requirements:
- [Requirement 1]
- [Requirement 2]
- [Requirement 3]

Tests to implement:
1. Test 1: [Description]
2. Test 2: [Description]
3. Test 3: [Description]
```

### Step 1.2: Create Test File

```
📁 Creating test file: test/[feature]_test.dart

Location: test/[feature]_test.dart
Framework: [test/flutter_test/pytest/jest]
```

### Step 1.3: Write Failing Tests

```
✍️  Writing tests to fail...

Test 1: validates [behavior] - ❌ Expected to fail
Test 2: validates [behavior] - ❌ Expected to fail
Test 3: validates [behavior] - ❌ Expected to fail
```

### Step 1.4: Verify Tests Fail

```
🔴 Running tests to confirm they fail...

/tdd-test test/[feature]_test.dart

Result: ❌ 3 tests failed (CORRECT - Red phase)

✅ RED PHASE COMPLETE

All tests are failing as expected. This confirms:
✓ Tests are working correctly
✓ Tests validate the right behavior
✓ Feature doesn't exist yet (correct)

Next: Move to GREEN phase
```

### Step 1.5: Commit Test Code

```
📝 Committing test code...

Commit message:
Red: Add test for [feature]

[Detailed description of what tests validate
and why this behavior is needed]

Commit hash: abc1234
```

**Commit format (NO Claude footer):**
```
Red: Add test for [feature]

Tests for [specific behaviors]:
- Test that validates [behavior 1]
- Test that validates [behavior 2]
- Test that validates [behavior 3]

These tests establish the contract for the feature
before any implementation code is written.
```

---

## PHASE 2: GREEN - Implement Minimal Code

### Step 2.1: Create Implementation File

```
📁 Creating implementation file: lib/[feature].dart

Location: lib/[feature].dart
Type: [Class/Function/Service]
Dependencies: [List from TDD task]
```

### Step 2.2: Write Minimal Implementation

**Guidance:**
```
🎯 Implementing minimal code to pass tests...

Focus: Make tests pass with simplest possible implementation
- Don't over-engineer
- Don't add extra features
- Don't optimize prematurely
- Can hardcode values if needed for now

Example (minimal):
✓ Use simple if-statements instead of complex logic
✓ Return hardcoded values if all tests expect the same
✓ Add TODO comments for future improvements
✓ Keep functions small and focused
```

### Step 2.3: Run Tests Frequently

```
✅ Running tests after each implementation...

/tdd-test test/[feature]_test.dart

Progress:
Test 1: ❌ → ✅ (passing)
Test 2: ❌ → ✅ (passing)
Test 3: ❌ → ✅ (passing)

Result: ✅ 3/3 tests passing

Continue with next test or move to commit
```

### Step 2.4: Verify All Tests Pass

```
🟢 Final test run for GREEN phase...

/tdd-test test/[feature]_test.dart

Result: ✅ ALL TESTS PASSING

✅ GREEN PHASE COMPLETE

Implementation satisfies all test requirements:
✓ All test cases pass
✓ No test failures
✓ Feature works as specified
✓ Ready to refactor for quality

Next: Move to REFACTOR phase
```

### Step 2.5: Commit Implementation

```
📝 Committing implementation code...

Commit message:
Green: Implement [feature]

[Minimal implementation that satisfies test requirements.
Describe what was implemented and which tests it passes.]

Commit hash: def5678
```

**Commit format:**
```
Green: Implement [feature]

Implements [specific functionality] with minimal code to pass
all test requirements. Implementation includes:
- [Component 1]
- [Component 2]
- [Component 3]

All tests now pass. Ready for refactoring.
```

---

## PHASE 3: REFACTOR - Improve Code Quality

### Step 3.1: Code Review for Improvements

**Guidance:**
```
🔍 Identifying refactoring opportunities...

Check for:
✓ Code duplication (extract functions/classes)
✓ Naming clarity (are names descriptive?)
✓ Function length (should be < 20 lines)
✓ Complexity (can it be simplified?)
✓ Comments (needed for non-obvious logic?)
✓ Error handling (proper validation?)
✓ Performance (can it be optimized?)
```

### Step 3.2: Run Code Analysis

```
📊 Running code analysis...

/tdd-test test/[feature]_test.dart

Linting Results:
- [File]: [Line] [Style issue] → ⚠️ Fix during refactoring
- [File]: [Line] [Style issue] → ⚠️ Fix during refactoring

Code Quality Score: [X/10]
```

### Step 3.3: Make Refactoring Changes

**For minor refactoring (single commit):**
```
✏️ Making refactoring improvements...

Changes:
1. Extract regex pattern to named constant
2. Improve variable naming for clarity
3. Add helpful code comments

Verify after each change:
/tdd-test test/[feature]_test.dart
```

**For major refactoring (multiple commits):**
```
✏️ Making major refactoring improvements...

This will be done in stages:

Stage 1: Extract validation logic
Stage 2: Improve error handling
Stage 3: Optimize performance
Stage 4: Improve documentation

Each stage: code change → test → commit
```

### Step 3.4: Run Final Tests

```
✅ Running final test verification...

/tdd-test test/[feature]_test.dart

Result: ✅ ALL TESTS STILL PASSING

Code Analysis: 🟢 No issues
Performance: ✅ Optimized
Readability: ✅ Improved
```

### Step 3.5: Commit Refactoring

```
📝 Committing refactoring improvements...

Commit message:
Refactor: [description of improvements]

[Explain why these changes improve the code and what was changed.]

Commit hash: ghi9012
```

**Commit format:**
```
Refactor: Extract email validation pattern

Extracted the email regex pattern to a named constant for
improved readability and easier maintenance. Updated related
code to use the constant instead of inline regex.

Benefits:
- Easier to understand validation rules
- Single source of truth for pattern
- Simpler to update pattern in future
```

---

## PHASE 4: GIT WORKFLOW & PULL REQUEST

### Step 4.1: View Commit History

```
📊 Feature commits created:

abc1234 Red: Add test for [feature]
def5678 Green: Implement [feature]
ghi9012 Refactor: Improve [feature] code quality

Clean history: ✅ Each phase clearly separated
```

### Step 4.2: Create Pull Request

```
🔗 Creating pull request...

Branch: feature/[feature-name]
Base: main
Commits: 3

PR Title:
feat: Implement [feature]

PR Description:
## Summary

[Brief description of feature and its purpose]

## Tests
- All TDD tests created and passing
- Full test coverage for feature

## Changes
- Red: Created test suite
- Green: Implemented feature
- Refactor: Improved code quality

## Checklist
- [x] All tests pass
- [x] Code analyzed (no linting issues)
- [x] Documentation updated
- [x] No breaking changes

Closes: [Issue #123 if applicable]
```

### Step 4.3: Code Review

```
🔍 Ready for code review

PR Link: [PR URL]

Next Steps for Reviewer:
1. Review test coverage
2. Verify implementation
3. Check for edge cases
4. Approve or request changes
```

---

## Commit Message Best Practices

### ✅ DO

```
Red: Add test for email validation

Tests for email address validation including:
- Valid email formats with standard characters
- Rejection of invalid formats (missing @, domain)
- Edge cases (dots, hyphens, underscores)

These tests establish the contract before implementation.
```

```
Green: Implement email validation

Minimal email validation implementation using regex pattern.
Validates against RFC 5322 basic requirements. Passes all
test cases. Can be enhanced in refactor phase.
```

```
Refactor: Extract regex pattern to constant

Extracted email validation regex to a named VALID_EMAIL_PATTERN
constant for improved maintainability and readability. Updated
validation function to use the constant.
```

### ❌ DON'T

```
Red: add test
(too brief, no context)

Green: implement everything
(vague, no specifics)

Refactor: various improvements
(unclear what changed)

🤖 Generated with Claude Code
(NO AI footer - clean commits)
```

---

## Phase Discipline Rules

### RED PHASE - STRICT RULES

```
✅ ALLOWED:
- Write test code
- Add test data/fixtures
- Write comments explaining tests

❌ NOT ALLOWED:
- Modify implementation code
- Refactor non-test code
- Run implementation

✓ Commit only when tests are confirmed FAILING
```

### GREEN PHASE - STRICT RULES

```
✅ ALLOWED:
- Write implementation code
- Minimal code to pass tests
- Add TODOs for refactoring

❌ NOT ALLOWED:
- Modify test code
- Over-implement features not tested
- Refactor code

✓ Commit only when all tests PASS
```

### REFACTOR PHASE - STRICT RULES

```
✅ ALLOWED:
- Improve code quality
- Extract functions
- Rename variables
- Optimize performance

❌ NOT ALLOWED:
- Add new features
- Change behavior
- Break tests

✓ Tests must remain PASSING
```

---

## Troubleshooting

### Tests won't pass

```
Problem: GREEN phase tests failing
Solution:
1. Review error message carefully
2. Check test expectations vs implementation
3. Implement missing logic
4. Re-run tests
5. Repeat until all pass
```

### Over-implemented features

```
Problem: You added features the tests don't require
Solution:
1. Remove extra code
2. Verify tests still pass
3. Keep implementation minimal
4. Extra features can be added later
```

### Lost track of phase

```
Problem: Not sure if tests should pass/fail
Solution:
- RED: Tests should FAIL ❌
- GREEN: Tests should PASS ✅
- REFACTOR: Tests should PASS ✅
Use /tdd-test to verify current state
```

### Merge conflicts

```
Problem: Can't merge PR due to conflicts
Solution:
1. Update feature branch from main
2. Resolve conflicts carefully
3. Re-run tests to ensure still pass
4. Force-push only if necessary
5. Notify code reviewer
```

### Want to add more tests

```
Problem: Thought of additional test cases
Solution:
1. You can add more tests anytime
2. Run new tests - they'll fail
3. Implement code to pass them
4. Refactor to consolidate
5. Commit additional test+implementation
```

---

## Summary

### What was accomplished

```
✅ Feature fully implemented with TDD:

Red Phase ❌
- Created comprehensive test suite
- All tests fail initially (expected)
- Confirmed test coverage

Green Phase ✅
- Minimal implementation code written
- All tests now passing
- Feature requirements satisfied

Refactor Phase ✅
- Code quality improved
- Removed duplication
- Enhanced readability
- Optimized performance

Git & PR 🔗
- Clean commit history (3 commits)
- Each phase clearly separated
- Pull request ready for review
```

### Code Quality Metrics

```
📊 Final Status:

Tests: ✅ 3/3 passing
Coverage: 100%
Code Analysis: ✅ No issues
Documentation: ✅ Complete
Git History: ✅ Clean
```

---

## Next Steps

1. **Code Review** - Share PR for team review
2. **Iterate** - Address review feedback
3. **Merge** - Merge to main when approved
4. **Next Feature** - Use `/tdd-new` for next feature

---

## Important Reminders

### TDD Discipline
- Red first, tests must fail
- Green with minimal code
- Refactor with tests green
- Commit after each phase

### Git Cleanliness
- No Claude footers in commits
- Clear phase separation
- Descriptive messages
- One feature per branch

### Code Quality
- Minimal in green phase
- Improved in refactor phase
- All tests passing always
- Analysis issues resolved

### Communication
- Clear commit messages
- Explain your changes
- Link to issues
- Provide context

---

## References

- **TDD Guidelines:** `docs/flutter-tdd-guide.md`
- **TDD Test Command:** `.claude/commands/tdd-test.md`
- **TDD New Command:** `.claude/commands/tdd-new.md`
- **Version Control:** `docs/version-control.md`
- **Framework Guides:** Check `docs/` directory

---

**Created:** 2025-11-23
