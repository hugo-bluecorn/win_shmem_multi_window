# C++ Testing Guide - Google Test

**Framework:** Google Test (gtest)
**Version:** Latest via CMake FetchContent
**Test Runner:** CTest (built into CMake)

---

## Overview

This project uses **Google Test** for all C++ native code unit testing. Google Test is the industry-standard xUnit-style testing framework for C++.

**Key Benefits:**
- Industry standard with extensive documentation
- xUnit-style familiar to most developers
- Rich assertion macros (EXPECT_*, ASSERT_*)
- Test fixtures for setup/teardown
- Parameterized tests support
- Integrates seamlessly with CMake/CTest

---

## Project Testing Structure

```
windows/
├── runner/
│   ├── shared_memory_manager.{h,cpp}       # Production code
│   ├── window_count_listener.{h,cpp}       # Production code
│   ├── dart_port_manager.{h,cpp}           # Production code
│   └── ...
├── test/
│   ├── shared_memory_manager_test.cpp      # Unit tests
│   ├── window_count_listener_test.cpp      # Unit tests
│   ├── dart_port_manager_test.cpp          # Unit tests
│   └── CMakeLists.txt                      # Test build config
└── CMakeLists.txt                          # Main build config
```

---

## Google Test Setup

### CMake Configuration

Google Test is added via CMake FetchContent in `windows/test/CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0  # or latest stable version
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()
```

### Test Executable

Each test file is compiled into a test executable:

```cmake
add_executable(shared_memory_manager_test
  shared_memory_manager_test.cpp
  ../runner/shared_memory_manager.cpp
)

target_link_libraries(shared_memory_manager_test
  GTest::gtest_main
)

target_include_directories(shared_memory_manager_test PRIVATE
  ../runner
)

add_test(NAME SharedMemoryManagerTest COMMAND shared_memory_manager_test)
```

---

## Writing Tests

### Basic Test Structure

```cpp
#include <gtest/gtest.h>
#include "shared_memory_manager.h"

// Simple test
TEST(SharedMemoryManagerTest, InitializeSucceeds) {
  SharedMemoryManager manager;
  EXPECT_TRUE(manager.Initialize());
}

// Test with multiple assertions
TEST(SharedMemoryManagerTest, IncrementWindowCount) {
  SharedMemoryManager manager;
  manager.Initialize();

  LONG count = manager.IncrementWindowCount();
  EXPECT_EQ(1, count);

  count = manager.IncrementWindowCount();
  EXPECT_EQ(2, count);
}
```

### Test Fixtures (for setup/teardown)

```cpp
class SharedMemoryTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Runs before each test
    manager_ = std::make_unique<SharedMemoryManager>();
    manager_->Initialize();
  }

  void TearDown() override {
    // Runs after each test
    manager_.reset();
  }

  std::unique_ptr<SharedMemoryManager> manager_;
};

TEST_F(SharedMemoryTest, GetWindowCountInitiallyZero) {
  EXPECT_EQ(0, manager_->GetWindowCount());
}
```

---

## Google Test Assertions

### EXPECT vs ASSERT

- **EXPECT_***: Non-fatal - test continues after failure
- **ASSERT_***: Fatal - test stops immediately after failure

**Use EXPECT_* for most tests** (allows seeing all failures in one run)
**Use ASSERT_* when continuing after failure is unsafe** (e.g., null pointer checks)

### Common Assertions

```cpp
// Boolean conditions
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Equality
EXPECT_EQ(expected, actual);
EXPECT_NE(val1, val2);

// Comparisons
EXPECT_LT(val1, val2);  // Less than
EXPECT_LE(val1, val2);  // Less or equal
EXPECT_GT(val1, val2);  // Greater than
EXPECT_GE(val1, val2);  // Greater or equal

// Pointers
EXPECT_EQ(nullptr, ptr);
EXPECT_NE(nullptr, ptr);

// Strings (C++ strings)
EXPECT_STREQ("expected", str.c_str());
EXPECT_STRNE("not this", str.c_str());

// Floating point (with tolerance)
EXPECT_FLOAT_EQ(expected, actual);
EXPECT_DOUBLE_EQ(expected, actual);
EXPECT_NEAR(expected, actual, tolerance);
```

---

## Running Tests

### Build Tests

```bash
# From project root
cd windows/test
cmake -B build -S .
cmake --build build
```

### Run All Tests

```bash
# Using CTest (recommended)
cd windows/test/build
ctest --output-on-failure

# Or run test executable directly
./shared_memory_manager_test
```

### Run Specific Test

```bash
# Run only tests matching a filter
./shared_memory_manager_test --gtest_filter=SharedMemoryManagerTest.InitializeSucceeds

# Run tests from a specific test suite
./shared_memory_manager_test --gtest_filter=SharedMemoryManagerTest.*
```

### Verbose Output

```bash
# Show all test output (even passing tests)
./shared_memory_manager_test --gtest_verbose=1

# Repeat tests (for flaky test detection)
./shared_memory_manager_test --gtest_repeat=10
```

---

## Test Coverage Best Practices

### What to Test

**DO test:**
- ✅ Public API behavior
- ✅ Edge cases (null inputs, boundary values)
- ✅ Error handling paths
- ✅ Thread safety (if applicable)
- ✅ Cross-process behavior (shared memory, events)
- ✅ Resource cleanup (RAII, destructors)

**DON'T test:**
- ❌ Private implementation details
- ❌ Windows API internals (assume they work)
- ❌ Trivial getters/setters (unless they have logic)

### Test Organization

Group tests by:
1. **Component** - One test file per class/module
2. **Functionality** - Tests for related features together
3. **Test type** - Unit tests vs integration tests

### Naming Conventions

```cpp
// Test suite: <ClassName>Test
// Test name: <MethodName>_<Scenario>_<ExpectedBehavior>

TEST(SharedMemoryManagerTest, Initialize_WhenCalled_ReturnsTrue) { ... }
TEST(SharedMemoryManagerTest, IncrementWindowCount_FromZero_ReturnsOne) { ... }
TEST(SharedMemoryManagerTest, GetWindowCount_BeforeInitialize_ReturnsZero) { ... }
```

---

## Windows-Specific Testing Considerations

### Shared Memory Testing

```cpp
TEST(SharedMemoryTest, MultipleInstances_ShareSameMemory) {
  // First instance
  SharedMemoryManager mgr1;
  mgr1.Initialize();
  mgr1.IncrementWindowCount();

  // Second instance (simulates second process)
  SharedMemoryManager mgr2;
  mgr2.Initialize();

  // Should see count from first instance
  EXPECT_EQ(1, mgr2.GetWindowCount());

  mgr2.IncrementWindowCount();

  // Both see updated count
  EXPECT_EQ(2, mgr1.GetWindowCount());
  EXPECT_EQ(2, mgr2.GetWindowCount());
}
```

### Cleanup Between Tests

Always clean up Windows resources:

```cpp
class WindowsResourceTest : public ::testing::Test {
protected:
  void TearDown() override {
    // Close handles, unmap memory, etc.
    if (handle_ != nullptr) {
      CloseHandle(handle_);
      handle_ = nullptr;
    }
  }

  HANDLE handle_ = nullptr;
};
```

---

## Continuous Integration

### GitHub Actions Example

```yaml
- name: Build and Test C++ (Windows)
  run: |
    cd windows/test
    cmake -B build -S .
    cmake --build build --config Release
    cd build
    ctest --output-on-failure -C Release
```

---

## Troubleshooting

### Test Fails to Link

**Error:** Undefined reference to Dart API symbols

**Solution:** Mock Dart APIs or exclude Dart-dependent code from unit tests

```cpp
// In test file - mock Dart_PostCObject_DL
extern "C" bool Dart_PostCObject_DL(int64_t port, void* message) {
  return true;  // Mock implementation
}
```

### Test Executable Doesn't Run

**Error:** "The code execution cannot proceed because ... .dll was not found"

**Solution:** Ensure all DLL dependencies are in PATH or same directory

### Shared Memory Tests Interfere

**Issue:** Tests fail when run together but pass individually

**Solution:** Use unique names per test:

```cpp
TEST(SharedMemoryTest, Test1) {
  const char* name = "Local\\TestSharedMemory_Test1";
  // Use unique name for this test
}
```

---

## References

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
- [CMake Testing](https://cmake.org/cmake/help/latest/manual/ctest.1.html)

---

**Created:** 2025-11-29
**Framework Version:** Google Test 1.14.0+
**Last Updated:** 2025-11-29
