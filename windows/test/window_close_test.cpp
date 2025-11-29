// window_close_test.cpp
//
// Google Test unit tests for RequestWindowClose FFI function.
// Tests that proper window close triggers DecrementWindowCount via OnDestroy.
//
// TDD Phase 1: RED - These tests will fail until implementation is complete.

#include <gtest/gtest.h>
#include <windows.h>
#include "shared_memory_manager.h"

// Note: RequestWindowClose is tested via GetProcAddress (runtime lookup)
// since we want the test to compile even when the function doesn't exist yet.

class WindowCloseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create fresh manager for each test
    manager_ = std::make_unique<SharedMemoryManager>();
    manager_->Initialize();
  }

  void TearDown() override {
    manager_.reset();
  }

  std::unique_ptr<SharedMemoryManager> manager_;
};

//==============================================================================
// Test Suite 1: FFI Function Existence
//==============================================================================

// Test 1.1: RequestWindowClose FFI function can be looked up
// NOTE: This test is DISABLED in standalone test executable because the function
// only exists in the Flutter app executable. The function is tested via Flutter
// integration tests instead. Enable when running tests linked with Flutter.
TEST_F(WindowCloseTest, DISABLED_RequestWindowClose_FunctionExists) {
  // Get the function pointer - this tests that the symbol is exported
  HMODULE hModule = GetModuleHandle(nullptr);
  ASSERT_NE(hModule, nullptr) << "Failed to get module handle";

  auto func = reinterpret_cast<void(*)()>(
      GetProcAddress(hModule, "RequestWindowClose"));

  // Function should exist and be non-null
  EXPECT_NE(func, nullptr) << "RequestWindowClose function not found in module";
}

//==============================================================================
// Test Suite 2: SharedMemoryManager Decrement Behavior
//==============================================================================

// Test 2.1: DecrementWindowCount reduces count correctly
TEST_F(WindowCloseTest, DecrementWindowCount_ReducesCount) {
  // Set up: increment to 2
  LONG count1 = manager_->IncrementWindowCount();
  ASSERT_EQ(1, count1);
  LONG count2 = manager_->IncrementWindowCount();
  ASSERT_EQ(2, count2);

  // Act: decrement
  LONG count_after_dec = manager_->DecrementWindowCount();

  // Assert: count is now 1
  EXPECT_EQ(1, count_after_dec);
  EXPECT_EQ(1, manager_->GetWindowCount());
}

// Test 2.2: Multiple decrements work correctly
TEST_F(WindowCloseTest, DecrementWindowCount_MultipleTimes) {
  // Set up: increment to 3
  manager_->IncrementWindowCount();  // 1
  manager_->IncrementWindowCount();  // 2
  manager_->IncrementWindowCount();  // 3
  ASSERT_EQ(3, manager_->GetWindowCount());

  // Act: decrement twice
  EXPECT_EQ(2, manager_->DecrementWindowCount());
  EXPECT_EQ(1, manager_->DecrementWindowCount());

  // Assert: count is now 1
  EXPECT_EQ(1, manager_->GetWindowCount());
}

// Test 2.3: Decrement from 1 reaches 0
TEST_F(WindowCloseTest, DecrementWindowCount_FromOne_ReachesZero) {
  manager_->IncrementWindowCount();  // 1
  ASSERT_EQ(1, manager_->GetWindowCount());

  LONG count = manager_->DecrementWindowCount();

  EXPECT_EQ(0, count);
  EXPECT_EQ(0, manager_->GetWindowCount());
}

//==============================================================================
// Test Suite 3: Event Signaling on Decrement
//==============================================================================

// Test 3.1: DecrementWindowCount signals event
TEST_F(WindowCloseTest, DecrementWindowCount_SignalsEvent) {
  // Open the same event that SharedMemoryManager creates
  HANDLE event = OpenEventA(
      SYNCHRONIZE,
      FALSE,
      "Local\\FlutterWindowCountChanged");
  ASSERT_NE(event, nullptr) << "Failed to open event - SharedMemoryManager may not have created it";

  // Increment first so we have something to decrement
  manager_->IncrementWindowCount();

  // The increment will have signaled the event, so reset it first
  WaitForSingleObject(event, 100);  // Consume the increment signal

  // Act: decrement - this should signal the event
  manager_->DecrementWindowCount();

  // Assert: event should be signaled within timeout
  DWORD result = WaitForSingleObject(event, 1000);
  EXPECT_EQ(WAIT_OBJECT_0, result)
      << "Event was not signaled within timeout after DecrementWindowCount";

  CloseHandle(event);
}

//==============================================================================
// Test Suite 4: Cross-Instance Decrement Visibility
//==============================================================================

// Test 4.1: Decrement in one manager is visible in another
TEST_F(WindowCloseTest, Decrement_VisibleAcrossInstances) {
  // Set up: increment in first manager
  manager_->IncrementWindowCount();  // count = 1
  manager_->IncrementWindowCount();  // count = 2
  ASSERT_EQ(2, manager_->GetWindowCount());

  // Create second manager (simulates second window/process)
  auto manager2 = std::make_unique<SharedMemoryManager>();
  manager2->Initialize();

  // Verify second manager sees the count
  ASSERT_EQ(2, manager2->GetWindowCount());

  // Act: decrement in first manager
  manager_->DecrementWindowCount();

  // Assert: second manager sees the decrement
  EXPECT_EQ(1, manager2->GetWindowCount());
}

// Test 4.2: Decrement in second manager is visible in first
TEST_F(WindowCloseTest, Decrement_VisibleInOriginalInstance) {
  // Set up: increment to 2
  manager_->IncrementWindowCount();
  manager_->IncrementWindowCount();
  ASSERT_EQ(2, manager_->GetWindowCount());

  // Create second manager and decrement there
  auto manager2 = std::make_unique<SharedMemoryManager>();
  manager2->Initialize();

  // Act: decrement in second manager
  manager2->DecrementWindowCount();

  // Assert: original manager sees the decrement
  EXPECT_EQ(1, manager_->GetWindowCount());
}
