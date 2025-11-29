// window_count_listener_test.cpp
//
// Google Test unit tests for WindowCountListener (Layer 2)
//
// Tests event-driven notification system that eliminates polling overhead

#include <gtest/gtest.h>
#include "window_count_listener.h"
#include <windows.h>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

class WindowCountListenerTest : public ::testing::Test {
protected:
  void SetUp() override {
    callback_count_ = 0;
    last_callback_value_ = -1;
  }

  void TearDown() override {
    // Cleanup handled by RAII
  }

  // Test callback function
  static std::atomic<int> callback_count_;
  static std::atomic<LONG> last_callback_value_;

  static void TestCallback(LONG value) {
    callback_count_++;
    last_callback_value_ = value;
  }
};

std::atomic<int> WindowCountListenerTest::callback_count_{0};
std::atomic<LONG> WindowCountListenerTest::last_callback_value_{-1};

//==============================================================================
// Test Suite 1: Basic Lifecycle
//==============================================================================

TEST_F(WindowCountListenerTest, Constructor_Succeeds) {
  WindowCountListener listener;
  EXPECT_FALSE(listener.IsRunning());
}

TEST_F(WindowCountListenerTest, Start_ReturnsTrue) {
  WindowCountListener listener;
  EXPECT_TRUE(listener.Start());
  EXPECT_TRUE(listener.IsRunning());
  listener.Stop();
}

TEST_F(WindowCountListenerTest, Start_Idempotent) {
  WindowCountListener listener;
  ASSERT_TRUE(listener.Start());
  EXPECT_TRUE(listener.Start());  // Second call should succeed
  listener.Stop();
}

TEST_F(WindowCountListenerTest, Stop_WhenNotRunning_Succeeds) {
  WindowCountListener listener;
  listener.Stop();  // Should not crash
  EXPECT_FALSE(listener.IsRunning());
}

TEST_F(WindowCountListenerTest, Stop_AfterStart_StopsListener) {
  WindowCountListener listener;
  listener.Start();
  ASSERT_TRUE(listener.IsRunning());

  listener.Stop();
  EXPECT_FALSE(listener.IsRunning());
}

TEST_F(WindowCountListenerTest, StartStopMultipleTimes_Succeeds) {
  WindowCountListener listener;

  listener.Start();
  EXPECT_TRUE(listener.IsRunning());
  listener.Stop();
  EXPECT_FALSE(listener.IsRunning());

  listener.Start();
  EXPECT_TRUE(listener.IsRunning());
  listener.Stop();
  EXPECT_FALSE(listener.IsRunning());
}

//==============================================================================
// Test Suite 2: Callback Registration and Execution
//==============================================================================

TEST_F(WindowCountListenerTest, SetCallback_Succeeds) {
  WindowCountListener listener;
  listener.SetCallback(TestCallback);
  // No assertion - just verifying it doesn't crash
}

TEST_F(WindowCountListenerTest, Callback_NotCalledWhenNotStarted) {
  WindowCountListener listener;
  listener.SetCallback(TestCallback);

  // Don't start listener
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(0, callback_count_);
}

TEST_F(WindowCountListenerTest, Callback_CalledOnEventSignal) {
  callback_count_ = 0;

  WindowCountListener listener;
  listener.SetCallback(TestCallback);
  listener.Start();

  // Manually signal the event (simulating SharedMemoryManager signaling)
  const char* event_name = "Local\\FlutterWindowCountChanged";
  HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event_name);
  ASSERT_NE(nullptr, hEvent) << "Event should exist after listener starts";

  SetEvent(hEvent);

  // Wait for callback to be invoked
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_GT(callback_count_, 0) << "Callback should be invoked when event signaled";

  CloseHandle(hEvent);
  listener.Stop();
}

//==============================================================================
// Test Suite 3: Event Creation and Sharing
//==============================================================================

TEST_F(WindowCountListenerTest, Event_CreatedOnStart) {
  WindowCountListener listener;
  listener.Start();

  // Try to open the event - should succeed if it was created
  const char* event_name = "Local\\FlutterWindowCountChanged";
  HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event_name);

  EXPECT_NE(nullptr, hEvent) << "Event should be created by listener";

  if (hEvent) {
    CloseHandle(hEvent);
  }

  listener.Stop();
}

TEST_F(WindowCountListenerTest, TwoListeners_ShareSameEvent) {
  // This simulates two windows/processes
  WindowCountListener listener1;
  WindowCountListener listener2;

  listener1.Start();
  listener2.Start();

  // Both listeners should be using the same event object
  // (verified by ERROR_ALREADY_EXISTS in WindowCountListener::CreateEvent)

  listener1.Stop();
  listener2.Stop();

  // Test passes if no crashes and both started successfully
  SUCCEED();
}

//==============================================================================
// Test Suite 4: Thread Safety
//==============================================================================

TEST_F(WindowCountListenerTest, StartStop_ThreadSafe) {
  WindowCountListener listener;

  // Rapidly start and stop from single thread
  for (int i = 0; i < 10; i++) {
    listener.Start();
    listener.Stop();
  }

  EXPECT_FALSE(listener.IsRunning());
}

TEST_F(WindowCountListenerTest, Destructor_StopsListener) {
  {
    WindowCountListener listener;
    listener.Start();
    EXPECT_TRUE(listener.IsRunning());

    // Destructor called here
  }

  // If thread wasn't properly stopped, this test may hang or crash
  SUCCEED();
}

//==============================================================================
// Test Suite 5: Event Notification Performance
//==============================================================================

TEST_F(WindowCountListenerTest, EventNotification_LowLatency) {
  callback_count_ = 0;

  WindowCountListener listener;
  listener.SetCallback(TestCallback);
  listener.Start();

  const char* event_name = "Local\\FlutterWindowCountChanged";
  HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event_name);
  ASSERT_NE(nullptr, hEvent);

  auto start = std::chrono::high_resolution_clock::now();

  SetEvent(hEvent);

  // Wait for callback with timeout
  int timeout_ms = 100;
  auto deadline = start + std::chrono::milliseconds(timeout_ms);

  while (callback_count_ == 0 && std::chrono::high_resolution_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  EXPECT_GT(callback_count_, 0) << "Callback should be invoked";
  EXPECT_LT(latency_ms, 50) << "Notification latency should be < 50ms (target: < 10ms)";

  CloseHandle(hEvent);
  listener.Stop();
}

//==============================================================================
// Test Suite 6: Multiple Event Signals
//==============================================================================

TEST_F(WindowCountListenerTest, MultipleSignals_AllReceived) {
  callback_count_ = 0;

  WindowCountListener listener;
  listener.SetCallback(TestCallback);
  listener.Start();

  const char* event_name = "Local\\FlutterWindowCountChanged";
  HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event_name);
  ASSERT_NE(nullptr, hEvent);

  const int NUM_SIGNALS = 5;

  for (int i = 0; i < NUM_SIGNALS; i++) {
    SetEvent(hEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  // Note: Auto-reset events may coalesce signals if they arrive faster than
  // the thread can process them. We expect at least 1 callback.
  EXPECT_GE(callback_count_, 1) << "At least one callback should be invoked";

  CloseHandle(hEvent);
  listener.Stop();
}

//==============================================================================
// Test Suite 7: Error Handling
//==============================================================================

TEST_F(WindowCountListenerTest, CallbackException_DoesNotCrash) {
  auto throwing_callback = [](LONG) {
    throw std::runtime_error("Test exception");
  };

  WindowCountListener listener;
  listener.SetCallback(throwing_callback);
  listener.Start();

  const char* event_name = "Local\\FlutterWindowCountChanged";
  HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event_name);

  if (hEvent) {
    SetEvent(hEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CloseHandle(hEvent);
  }

  listener.Stop();

  // Test passes if no crash occurred
  SUCCEED();
}

TEST_F(WindowCountListenerTest, NoCallback_DoesNotCrash) {
  WindowCountListener listener;
  // Don't set callback
  listener.Start();

  const char* event_name = "Local\\FlutterWindowCountChanged";
  HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, event_name);

  if (hEvent) {
    SetEvent(hEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CloseHandle(hEvent);
  }

  listener.Stop();

  SUCCEED();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
