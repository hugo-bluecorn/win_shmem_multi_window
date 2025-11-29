// dart_port_manager_test.cpp
//
// Google Test unit tests for DartPortManager (Layer 3)
//
// Tests C++ to Dart communication layer using centralized mock Dart API
// from mock_dart_api_dl.h

#include <gtest/gtest.h>
#include <windows.h>
#include <algorithm>

// Include dart_api_dl.h which redirects to our mock in test builds
// This must come BEFORE dart_port_manager.h
#include "dart_api_dl.h"

// Now include DartPortManager - it will use the mock Dart API types
#include "dart_port_manager.h"

class DartPortManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Reset mock state between tests
    mock_dart_api::Reset();

    // Get global instance
    manager_ = &GetGlobalDartPortManager();
  }

  void TearDown() override {
    // Unregister all test ports
    for (Dart_Port_DL port : test_ports_) {
      manager_->UnregisterPort(port);
    }
    test_ports_.clear();
  }

  DartPortManager* manager_;
  std::vector<Dart_Port_DL> test_ports_;

  Dart_Port_DL CreateTestPort() {
    static Dart_Port_DL next_port = 1000;
    Dart_Port_DL port = next_port++;
    test_ports_.push_back(port);
    return port;
  }

  // Helper to get post call count from mock
  size_t GetPostCallCount() const {
    return mock_dart_api::GetPostCalls().size();
  }

  // Helper to get posted values from mock
  std::vector<int64_t> GetPostedValues() const {
    std::vector<int64_t> values;
    for (const auto& call : mock_dart_api::GetPostCalls()) {
      if (call.type == Dart_CObject_kInt64) {
        values.push_back(call.value_as_int64);
      }
    }
    return values;
  }

  // Helper to get posted ports from mock
  std::vector<Dart_Port_DL> GetPostedPorts() const {
    std::vector<Dart_Port_DL> ports;
    for (const auto& call : mock_dart_api::GetPostCalls()) {
      ports.push_back(call.port);
    }
    return ports;
  }
};

//==============================================================================
// Test Suite 1: Port Registration
//==============================================================================

TEST_F(DartPortManagerTest, RegisterPort_Succeeds) {
  Dart_Port_DL port = CreateTestPort();
  bool result = manager_->RegisterPort(port, 0);
  EXPECT_TRUE(result);
}

TEST_F(DartPortManagerTest, RegisterPort_SendsInitialCount) {
  mock_dart_api::Reset();
  Dart_Port_DL port = CreateTestPort();
  LONG initial_count = 5;
  manager_->RegisterPort(port, initial_count);
  EXPECT_EQ(1u, GetPostCallCount());
  auto values = GetPostedValues();
  ASSERT_EQ(1u, values.size());
  EXPECT_EQ(initial_count, values[0]);
}

TEST_F(DartPortManagerTest, RegisterMultiplePorts_Succeeds) {
  Dart_Port_DL port1 = CreateTestPort();
  Dart_Port_DL port2 = CreateTestPort();
  Dart_Port_DL port3 = CreateTestPort();
  EXPECT_TRUE(manager_->RegisterPort(port1, 0));
  EXPECT_TRUE(manager_->RegisterPort(port2, 0));
  EXPECT_TRUE(manager_->RegisterPort(port3, 0));
}

//==============================================================================
// Test Suite 2: Port Unregistration
//==============================================================================

TEST_F(DartPortManagerTest, UnregisterPort_AfterRegister_ReturnsTrue) {
  Dart_Port_DL port = CreateTestPort();
  manager_->RegisterPort(port, 0);
  bool result = manager_->UnregisterPort(port);
  EXPECT_TRUE(result);
}

TEST_F(DartPortManagerTest, UnregisterPort_NotRegistered_ReturnsFalse) {
  Dart_Port_DL port = CreateTestPort();
  bool result = manager_->UnregisterPort(port);
  EXPECT_FALSE(result);
}

//==============================================================================
// Test Suite 3: Notification Broadcasting
//==============================================================================

TEST_F(DartPortManagerTest, NotifyWindowCountChanged_NoPorts_DoesNotCrash) {
  manager_->NotifyWindowCountChanged(5);
  EXPECT_EQ(0u, GetPostCallCount());
}

TEST_F(DartPortManagerTest, NotifyWindowCountChanged_OnePort_PostsMessage) {
  Dart_Port_DL port = CreateTestPort();
  manager_->RegisterPort(port, 0);
  mock_dart_api::Reset();
  LONG new_count = 10;
  manager_->NotifyWindowCountChanged(new_count);
  EXPECT_EQ(1u, GetPostCallCount());
  auto values = GetPostedValues();
  ASSERT_EQ(1u, values.size());
  EXPECT_EQ(new_count, values[0]);
}

TEST_F(DartPortManagerTest, NotifyWindowCountChanged_MultiplePorts_BroadcastsToAll) {
  Dart_Port_DL port1 = CreateTestPort();
  Dart_Port_DL port2 = CreateTestPort();
  Dart_Port_DL port3 = CreateTestPort();
  manager_->RegisterPort(port1, 0);
  manager_->RegisterPort(port2, 0);
  manager_->RegisterPort(port3, 0);
  mock_dart_api::Reset();
  LONG new_count = 42;
  manager_->NotifyWindowCountChanged(new_count);
  EXPECT_EQ(3u, GetPostCallCount());
  auto ports = GetPostedPorts();
  auto values = GetPostedValues();
  EXPECT_EQ(3u, ports.size());
  EXPECT_EQ(3u, values.size());
  for (int64_t value : values) {
    EXPECT_EQ(new_count, value);
  }
}

//==============================================================================
// Test Suite 4: FFI Export Functions
//==============================================================================

TEST_F(DartPortManagerTest, RegisterWindowCountPort_FFI_Succeeds) {
  Dart_Port_DL port = CreateTestPort();
  bool result = RegisterWindowCountPort(port);
  EXPECT_TRUE(result);
  manager_->UnregisterPort(port);
}

TEST_F(DartPortManagerTest, InitDartApiDL_FFI_Succeeds) {
  intptr_t result = InitDartApiDL(nullptr);
  EXPECT_EQ(0, result);
}

//==============================================================================
// Test Suite 5: Global Instance
//==============================================================================

TEST_F(DartPortManagerTest, GetGlobalDartPortManager_ReturnsSameInstance) {
  DartPortManager& instance1 = GetGlobalDartPortManager();
  DartPortManager& instance2 = GetGlobalDartPortManager();
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(DartPortManagerTest, SetCurrentWindowCount_UpdatesGlobal) {
  SetCurrentWindowCount(42);
  mock_dart_api::Reset();
  Dart_Port_DL port = CreateTestPort();
  RegisterWindowCountPort(port);
  auto values = GetPostedValues();
  ASSERT_GE(values.size(), 1u);
  EXPECT_EQ(42, values[0]);
  manager_->UnregisterPort(port);
}

//==============================================================================
// Test Suite 6: Error Handling
//==============================================================================

TEST_F(DartPortManagerTest, NotifyWindowCountChanged_PostFails_ContinuesWithOthers) {
  Dart_Port_DL port1 = CreateTestPort();
  Dart_Port_DL port2 = CreateTestPort();
  manager_->RegisterPort(port1, 0);
  manager_->RegisterPort(port2, 0);
  mock_dart_api::Reset();
  mock_dart_api::SetPostShouldFail(true);
  manager_->NotifyWindowCountChanged(5);
  EXPECT_EQ(2u, GetPostCallCount());
  mock_dart_api::SetPostShouldFail(false);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
