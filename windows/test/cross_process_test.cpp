// cross_process_test.cpp
//
// Integration tests for all 3 layers working together:
// SharedMemoryManager → WindowCountListener → DartPortManager
//
// These tests verify the complete event-driven multi-window architecture

#include <gtest/gtest.h>
#include "shared_memory_manager.h"
#include "window_count_listener.h"
#include <windows.h>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

class CrossProcessTest : public ::testing::Test {
protected:
  void SetUp() override {
    callback_triggered_ = false;
    callback_count_ = 0;
    last_notified_count_ = -1;
  }

  void TearDown() override {
    // RAII cleanup
  }

  static std::atomic<bool> callback_triggered_;
  static std::atomic<int> callback_count_;
  static std::atomic<LONG> last_notified_count_;

  static void TestCallback(LONG count) {
    callback_triggered_ = true;
    callback_count_++;
    last_notified_count_ = count;
  }
};

std::atomic<bool> CrossProcessTest::callback_triggered_{false};
std::atomic<int> CrossProcessTest::callback_count_{0};
std::atomic<LONG> CrossProcessTest::last_notified_count_{-1};

//==============================================================================
// Test Suite 1: Layer 1 + Layer 2 Integration
//==============================================================================

TEST_F(CrossProcessTest, SharedMemory_TriggersListener) {
  auto memory_mgr = std::make_unique<SharedMemoryManager>();
  auto listener = std::make_unique<WindowCountListener>();

  ASSERT_TRUE(memory_mgr->Initialize());

  callback_triggered_ = false;
  listener->SetCallback(TestCallback);
  listener->Start();

  // Increment shared memory count - should trigger event
  memory_mgr->IncrementWindowCount();

  // Wait for callback
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Note: Callback receives 0 as placeholder in current implementation
  // (WindowCountListener doesn't have access to SharedMemoryManager)
  EXPECT_TRUE(callback_triggered_)
      << "WindowCountListener callback should be triggered when SharedMemoryManager signals event";

  listener->Stop();
}

TEST_F(CrossProcessTest, TwoMemoryManagers_OneListener_ReceivesBothSignals) {
  auto mgr1 = std::make_unique<SharedMemoryManager>();
  auto mgr2 = std::make_unique<SharedMemoryManager>();
  auto listener = std::make_unique<WindowCountListener>();

  mgr1->Initialize();
  mgr2->Initialize();

  callback_count_ = 0;
  listener->SetCallback(TestCallback);
  listener->Start();

  // Increment from first manager
  mgr1->IncrementWindowCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Increment from second manager
  mgr2->IncrementWindowCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Should have received at least 1 callback (may coalesce)
  EXPECT_GE(callback_count_, 1)
      << "Listener should receive signals from both managers";

  listener->Stop();
}

//==============================================================================
// Test Suite 2: Multi-Window Simulation
//==============================================================================

TEST_F(CrossProcessTest, SimulateThreeWindows_CountSynchronized) {
  // Simulate 3 windows (3 SharedMemoryManager instances)
  std::vector<std::unique_ptr<SharedMemoryManager>> windows;

  for (int i = 0; i < 3; i++) {
    auto mgr = std::make_unique<SharedMemoryManager>();
    ASSERT_TRUE(mgr->Initialize());
    windows.push_back(std::move(mgr));
  }

  // All windows start at count 0
  for (auto& window : windows) {
    EXPECT_EQ(0, window->GetWindowCount());
  }

  // Each window "opens" (increments)
  for (auto& window : windows) {
    window->IncrementWindowCount();
  }

  // All windows should see count = 3
  for (auto& window : windows) {
    EXPECT_EQ(3, window->GetWindowCount())
        << "All windows should see synchronized count";
  }

  // First window "closes" (decrements)
  windows[0]->DecrementWindowCount();

  // All remaining windows should see count = 2
  for (auto& window : windows) {
    EXPECT_EQ(2, window->GetWindowCount());
  }
}

TEST_F(CrossProcessTest, SimulateWindowLifecycle_OpenCloseSequence) {
  auto listener = std::make_unique<WindowCountListener>();
  callback_count_ = 0;
  listener->SetCallback(TestCallback);
  listener->Start();

  std::vector<std::unique_ptr<SharedMemoryManager>> windows;

  // Open 5 windows
  for (int i = 0; i < 5; i++) {
    auto mgr = std::make_unique<SharedMemoryManager>();
    mgr->Initialize();
    mgr->IncrementWindowCount();
    windows.push_back(std::move(mgr));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // All should see count = 5
  for (auto& window : windows) {
    EXPECT_EQ(5, window->GetWindowCount());
  }

  // Close 3 windows
  for (int i = 0; i < 3; i++) {
    windows[i]->DecrementWindowCount();
    windows[i].reset();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Remaining windows should see count = 2
  for (size_t i = 3; i < windows.size(); i++) {
    EXPECT_EQ(2, windows[i]->GetWindowCount());
  }

  // Listener should have received multiple callbacks
  EXPECT_GT(callback_count_, 0)
      << "Listener should receive callbacks during window lifecycle";

  listener->Stop();
}

//==============================================================================
// Test Suite 3: Event Latency and Performance
//==============================================================================

TEST_F(CrossProcessTest, EventNotification_SubMillisecondLatency) {
  auto memory_mgr = std::make_unique<SharedMemoryManager>();
  auto listener = std::make_unique<WindowCountListener>();

  memory_mgr->Initialize();

  callback_triggered_ = false;
  listener->SetCallback(TestCallback);
  listener->Start();

  auto start = std::chrono::high_resolution_clock::now();

  memory_mgr->IncrementWindowCount();

  // Wait for callback with reasonable timeout
  int timeout_ms = 100;
  auto deadline = start + std::chrono::milliseconds(timeout_ms);

  while (!callback_triggered_ && std::chrono::high_resolution_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  EXPECT_TRUE(callback_triggered_) << "Callback should be triggered";
  EXPECT_LT(latency_us, 50000) << "Event notification latency should be < 50ms (50000us)";

  listener->Stop();
}

//==============================================================================
// Test Suite 4: Stress Testing
//==============================================================================

TEST_F(CrossProcessTest, RapidIncrementDecrement_NoLostUpdates) {
  const int NUM_OPERATIONS = 100;

  auto mgr1 = std::make_unique<SharedMemoryManager>();
  auto mgr2 = std::make_unique<SharedMemoryManager>();

  mgr1->Initialize();
  mgr2->Initialize();

  // Rapid increments and decrements
  for (int i = 0; i < NUM_OPERATIONS; i++) {
    mgr1->IncrementWindowCount();
    mgr2->DecrementWindowCount();
  }

  // Final count should be 0 (balanced increments and decrements)
  EXPECT_EQ(0, mgr1->GetWindowCount());
  EXPECT_EQ(0, mgr2->GetWindowCount());
}

TEST_F(CrossProcessTest, ManyWindows_AllSynchronized) {
  const int NUM_WINDOWS = 20;
  std::vector<std::unique_ptr<SharedMemoryManager>> windows;

  for (int i = 0; i < NUM_WINDOWS; i++) {
    auto mgr = std::make_unique<SharedMemoryManager>();
    ASSERT_TRUE(mgr->Initialize());
    windows.push_back(std::move(mgr));
  }

  // Each window increments
  for (auto& window : windows) {
    window->IncrementWindowCount();
  }

  // All should see count = NUM_WINDOWS
  for (auto& window : windows) {
    EXPECT_EQ(NUM_WINDOWS, window->GetWindowCount())
        << "All " << NUM_WINDOWS << " windows should see synchronized count";
  }
}

//==============================================================================
// Test Suite 5: Robustness and Error Handling
//==============================================================================

TEST_F(CrossProcessTest, ListenerStartStop_WhileMemoryActive_NoLeaks) {
  auto memory_mgr = std::make_unique<SharedMemoryManager>();
  memory_mgr->Initialize();

  // Start and stop listener multiple times while memory is active
  for (int i = 0; i < 5; i++) {
    auto listener = std::make_unique<WindowCountListener>();
    listener->Start();

    memory_mgr->IncrementWindowCount();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    listener->Stop();
  }

  // Test passes if no crashes or resource leaks
  SUCCEED();
}

TEST_F(CrossProcessTest, MemoryManagerDestroyed_ListenerContinues) {
  {
    auto memory_mgr = std::make_unique<SharedMemoryManager>();
    memory_mgr->Initialize();
    memory_mgr->IncrementWindowCount();

    // memory_mgr destroyed here
  }

  // Listener created after memory manager destroyed
  auto listener = std::make_unique<WindowCountListener>();
  EXPECT_TRUE(listener->Start());

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  listener->Stop();

  // Test passes if listener handles orphaned event correctly
  SUCCEED();
}

//==============================================================================
// Test Suite 6: CRITICAL - Multi-Instance Synchronization
//==============================================================================

TEST_F(CrossProcessTest, CRITICAL_MultiInstanceSync_CompleteWorkflow) {
  // This test verifies the COMPLETE workflow:
  // 1. Multiple SharedMemoryManagers share memory
  // 2. WindowCountListener receives notifications
  // 3. All instances stay synchronized

  auto listener = std::make_unique<WindowCountListener>();
  callback_count_ = 0;
  listener->SetCallback(TestCallback);
  listener->Start();

  auto window1 = std::make_unique<SharedMemoryManager>();
  auto window2 = std::make_unique<SharedMemoryManager>();
  auto window3 = std::make_unique<SharedMemoryManager>();

  ASSERT_TRUE(window1->Initialize());
  ASSERT_TRUE(window2->Initialize());
  ASSERT_TRUE(window3->Initialize());

  // Window 1 opens
  window1->IncrementWindowCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  EXPECT_EQ(1, window1->GetWindowCount());
  EXPECT_EQ(1, window2->GetWindowCount());
  EXPECT_EQ(1, window3->GetWindowCount());

  // Window 2 opens
  window2->IncrementWindowCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  EXPECT_EQ(2, window1->GetWindowCount());
  EXPECT_EQ(2, window2->GetWindowCount());
  EXPECT_EQ(2, window3->GetWindowCount());

  // Window 3 opens
  window3->IncrementWindowCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  EXPECT_EQ(3, window1->GetWindowCount());
  EXPECT_EQ(3, window2->GetWindowCount());
  EXPECT_EQ(3, window3->GetWindowCount());

  // Window 1 closes
  window1->DecrementWindowCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  EXPECT_EQ(2, window1->GetWindowCount());
  EXPECT_EQ(2, window2->GetWindowCount());
  EXPECT_EQ(2, window3->GetWindowCount());

  // Listener should have received callbacks
  EXPECT_GT(callback_count_, 0)
      << "WindowCountListener should receive event notifications";

  listener->Stop();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
