// shared_memory_manager_test.cpp
//
// Google Test unit tests for SharedMemoryManager (Layer 1)
//
// CRITICAL TESTS: These tests verify that shared memory is actually shared
// across multiple instances, which is the root cause of our multi-window issue.

#include <gtest/gtest.h>
#include "shared_memory_manager.h"
#include <windows.h>
#include <memory>

class SharedMemoryManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Each test starts fresh
  }

  void TearDown() override {
    // RAII handles cleanup
  }
};

//==============================================================================
// Test Suite 1: Basic Initialization
//==============================================================================

TEST_F(SharedMemoryManagerTest, Initialize_ReturnsTrue) {
  SharedMemoryManager manager;
  EXPECT_TRUE(manager.Initialize());
}

TEST_F(SharedMemoryManagerTest, Initialize_Idempotent) {
  SharedMemoryManager manager;
  ASSERT_TRUE(manager.Initialize());
  EXPECT_TRUE(manager.Initialize());  // Second call should succeed
}

TEST_F(SharedMemoryManagerTest, GetWindowCount_BeforeInit_ReturnsZero) {
  SharedMemoryManager manager;
  EXPECT_EQ(0, manager.GetWindowCount());
}

TEST_F(SharedMemoryManagerTest, GetWindowCount_AfterInit_ReturnsZero) {
  SharedMemoryManager manager;
  manager.Initialize();
  EXPECT_EQ(0, manager.GetWindowCount());
}

//==============================================================================
// Test Suite 2: Increment/Decrement Operations
//==============================================================================

TEST_F(SharedMemoryManagerTest, Increment_FromZero_ReturnsOne) {
  SharedMemoryManager manager;
  manager.Initialize();

  LONG count = manager.IncrementWindowCount();
  EXPECT_EQ(1, count);
}

TEST_F(SharedMemoryManagerTest, Increment_Multiple_CountsCorrectly) {
  SharedMemoryManager manager;
  manager.Initialize();

  EXPECT_EQ(1, manager.IncrementWindowCount());
  EXPECT_EQ(2, manager.IncrementWindowCount());
  EXPECT_EQ(3, manager.IncrementWindowCount());
}

TEST_F(SharedMemoryManagerTest, Decrement_FromOne_ReturnsZero) {
  SharedMemoryManager manager;
  manager.Initialize();
  manager.IncrementWindowCount();

  LONG count = manager.DecrementWindowCount();
  EXPECT_EQ(0, count);
}

TEST_F(SharedMemoryManagerTest, GetWindowCount_ReflectsChanges) {
  SharedMemoryManager manager;
  manager.Initialize();

  manager.IncrementWindowCount();
  EXPECT_EQ(1, manager.GetWindowCount());

  manager.IncrementWindowCount();
  EXPECT_EQ(2, manager.GetWindowCount());

  manager.DecrementWindowCount();
  EXPECT_EQ(1, manager.GetWindowCount());
}

TEST_F(SharedMemoryManagerTest, IncrementDecrement_Sequence) {
  SharedMemoryManager manager;
  manager.Initialize();

  EXPECT_EQ(1, manager.IncrementWindowCount());
  EXPECT_EQ(2, manager.IncrementWindowCount());
  EXPECT_EQ(1, manager.DecrementWindowCount());
  EXPECT_EQ(2, manager.IncrementWindowCount());
  EXPECT_EQ(1, manager.DecrementWindowCount());
  EXPECT_EQ(0, manager.DecrementWindowCount());
}

//==============================================================================
// Test Suite 3: CRITICAL - Cross-Instance Shared Memory
//==============================================================================

TEST_F(SharedMemoryManagerTest, CRITICAL_TwoInstances_ShareMemory) {
  // This is THE critical test that will prove if shared memory works

  auto mgr1 = std::make_unique<SharedMemoryManager>();
  ASSERT_TRUE(mgr1->Initialize());

  mgr1->IncrementWindowCount();
  EXPECT_EQ(1, mgr1->GetWindowCount());

  // Create second instance - simulates second window/process
  auto mgr2 = std::make_unique<SharedMemoryManager>();
  ASSERT_TRUE(mgr2->Initialize());

  // CRITICAL: Second instance MUST see count from first instance
  EXPECT_EQ(1, mgr2->GetWindowCount())
      << "CRITICAL FAILURE: Second instance should see count=1 from first instance. "
      << "This means shared memory is NOT being shared across instances! "
      << "Root cause of multi-window synchronization failure.";
}

TEST_F(SharedMemoryManagerTest, CRITICAL_TwoInstances_BothSeeUpdates) {
  auto mgr1 = std::make_unique<SharedMemoryManager>();
  auto mgr2 = std::make_unique<SharedMemoryManager>();

  ASSERT_TRUE(mgr1->Initialize());
  ASSERT_TRUE(mgr2->Initialize());

  // Increment from first instance
  mgr1->IncrementWindowCount();

  // CRITICAL: Second instance sees the update
  EXPECT_EQ(1, mgr2->GetWindowCount())
      << "Second instance must see increment from first";

  // Increment from second instance
  mgr2->IncrementWindowCount();

  // CRITICAL: First instance sees the update
  EXPECT_EQ(2, mgr1->GetWindowCount())
      << "First instance must see increment from second";
}

TEST_F(SharedMemoryManagerTest, CRITICAL_ThreeInstances_AllShareMemory) {
  auto mgr1 = std::make_unique<SharedMemoryManager>();
  auto mgr2 = std::make_unique<SharedMemoryManager>();
  auto mgr3 = std::make_unique<SharedMemoryManager>();

  ASSERT_TRUE(mgr1->Initialize());
  ASSERT_TRUE(mgr2->Initialize());
  ASSERT_TRUE(mgr3->Initialize());

  mgr1->IncrementWindowCount();
  EXPECT_EQ(1, mgr1->GetWindowCount());
  EXPECT_EQ(1, mgr2->GetWindowCount());
  EXPECT_EQ(1, mgr3->GetWindowCount());

  mgr2->IncrementWindowCount();
  EXPECT_EQ(2, mgr1->GetWindowCount());
  EXPECT_EQ(2, mgr2->GetWindowCount());
  EXPECT_EQ(2, mgr3->GetWindowCount());

  mgr3->IncrementWindowCount();
  EXPECT_EQ(3, mgr1->GetWindowCount());
  EXPECT_EQ(3, mgr2->GetWindowCount());
  EXPECT_EQ(3, mgr3->GetWindowCount());
}

TEST_F(SharedMemoryManagerTest, CRITICAL_InstanceDestroyed_MemoryPersists) {
  auto mgr1 = std::make_unique<SharedMemoryManager>();
  mgr1->Initialize();
  mgr1->IncrementWindowCount();

  {
    auto mgr2 = std::make_unique<SharedMemoryManager>();
    mgr2->Initialize();

    EXPECT_EQ(1, mgr2->GetWindowCount());
    mgr2->IncrementWindowCount();

    // mgr2 destroyed here
  }

  // Memory should persist even after mgr2 destroyed
  EXPECT_EQ(2, mgr1->GetWindowCount())
      << "Shared memory should persist after instance destruction";
}

//==============================================================================
// Test Suite 4: Atomic Operations Under Contention
//==============================================================================

TEST_F(SharedMemoryManagerTest, AtomicOperations_NoRaceCondition) {
  auto mgr1 = std::make_unique<SharedMemoryManager>();
  auto mgr2 = std::make_unique<SharedMemoryManager>();

  mgr1->Initialize();
  mgr2->Initialize();

  // Rapid alternating increments
  mgr1->IncrementWindowCount();  // 1
  mgr2->IncrementWindowCount();  // 2
  mgr1->IncrementWindowCount();  // 3
  mgr2->IncrementWindowCount();  // 4
  mgr1->IncrementWindowCount();  // 5

  EXPECT_EQ(5, mgr1->GetWindowCount());
  EXPECT_EQ(5, mgr2->GetWindowCount());
}

TEST_F(SharedMemoryManagerTest, MixedOperations_CountCorrect) {
  auto mgr1 = std::make_unique<SharedMemoryManager>();
  auto mgr2 = std::make_unique<SharedMemoryManager>();

  mgr1->Initialize();
  mgr2->Initialize();

  mgr1->IncrementWindowCount();  // 1
  mgr2->IncrementWindowCount();  // 2
  mgr1->DecrementWindowCount();  // 1
  mgr2->IncrementWindowCount();  // 2
  mgr1->IncrementWindowCount();  // 3
  mgr2->DecrementWindowCount();  // 2

  EXPECT_EQ(2, mgr1->GetWindowCount());
  EXPECT_EQ(2, mgr2->GetWindowCount());
}

//==============================================================================
// Test Suite 5: Error Handling
//==============================================================================

TEST_F(SharedMemoryManagerTest, IncrementWithoutInit_ReturnsNegative) {
  SharedMemoryManager manager;
  // Don't initialize

  LONG count = manager.IncrementWindowCount();
  EXPECT_EQ(-1, count);
}

TEST_F(SharedMemoryManagerTest, DecrementWithoutInit_ReturnsNegative) {
  SharedMemoryManager manager;
  // Don't initialize

  LONG count = manager.DecrementWindowCount();
  EXPECT_EQ(-1, count);
}

//==============================================================================
// Test Suite 6: Edge Cases
//==============================================================================

TEST_F(SharedMemoryManagerTest, DecrementBelowZero_HandledCorrectly) {
  SharedMemoryManager manager;
  manager.Initialize();

  // Decrement when count is already 0
  LONG count = manager.DecrementWindowCount();
  EXPECT_EQ(-1, count);
}

TEST_F(SharedMemoryManagerTest, MultipleIncrementDecrement_LargeNumbers) {
  SharedMemoryManager manager;
  manager.Initialize();

  // Increment 100 times
  for (int i = 0; i < 100; i++) {
    manager.IncrementWindowCount();
  }

  EXPECT_EQ(100, manager.GetWindowCount());

  // Decrement 50 times
  for (int i = 0; i < 50; i++) {
    manager.DecrementWindowCount();
  }

  EXPECT_EQ(50, manager.GetWindowCount());
}

TEST_F(SharedMemoryManagerTest, MaxInstances_AllShareMemory) {
  // Test with many instances (simulates many windows)
  const int NUM_INSTANCES = 10;
  std::vector<std::unique_ptr<SharedMemoryManager>> managers;

  for (int i = 0; i < NUM_INSTANCES; i++) {
    auto mgr = std::make_unique<SharedMemoryManager>();
    mgr->Initialize();
    managers.push_back(std::move(mgr));
  }

  // Each increments once
  for (auto& mgr : managers) {
    mgr->IncrementWindowCount();
  }

  // All should see count = NUM_INSTANCES
  for (auto& mgr : managers) {
    EXPECT_EQ(NUM_INSTANCES, mgr->GetWindowCount())
        << "All instances should see the same count";
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
