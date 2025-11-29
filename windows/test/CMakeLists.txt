# CMakeLists.txt for Google Test unit tests
cmake_minimum_required(VERSION 3.14)
project(SharedMemoryTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Include runner directory for production code headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../runner)

# Test executable: SharedMemoryManager tests
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

# Test executable: WindowCountListener tests
add_executable(window_count_listener_test
  window_count_listener_test.cpp
  ../runner/window_count_listener.cpp
)

target_link_libraries(window_count_listener_test
  GTest::gtest_main
)

target_include_directories(window_count_listener_test PRIVATE
  ../runner
)

add_test(NAME WindowCountListenerTest COMMAND window_count_listener_test)

# Test executable: DartPortManager tests (with mocked Dart API)
add_executable(dart_port_manager_test
  dart_port_manager_test.cpp
  ../runner/dart_port_manager.cpp
)

target_link_libraries(dart_port_manager_test
  GTest::gtest_main
)

target_include_directories(dart_port_manager_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../runner
)

add_test(NAME DartPortManagerTest COMMAND dart_port_manager_test)

# Test executable: Cross-process integration tests
add_executable(cross_process_test
  cross_process_test.cpp
  ../runner/shared_memory_manager.cpp
  ../runner/window_count_listener.cpp
)

target_link_libraries(cross_process_test
  GTest::gtest_main
)

target_include_directories(cross_process_test PRIVATE
  ../runner
)

add_test(NAME CrossProcessTest COMMAND cross_process_test)

# Test executable: Window close tests (RequestWindowClose FFI)
add_executable(window_close_test
  window_close_test.cpp
  ../runner/shared_memory_manager.cpp
)

target_link_libraries(window_close_test
  GTest::gtest_main
)

target_include_directories(window_close_test PRIVATE
  ../runner
)

add_test(NAME WindowCloseTest COMMAND window_close_test)
