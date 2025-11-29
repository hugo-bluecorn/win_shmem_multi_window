// mock_dart_api_dl.h
//
// Mock Dart API DL header for unit testing without Dart runtime.
// Provides stub types and functions that mimic the real Dart C API.
//
// This allows testing DartPortManager in isolation without depending
// on the actual Dart runtime, enabling pure C++ unit tests.

#ifndef TEST_MOCK_DART_API_DL_H_
#define TEST_MOCK_DART_API_DL_H_

#include <cstdint>
#include <functional>
#include <vector>

// ============================================================================
// Dart API DL Types (matching dart_api_dl.h signatures)
// ============================================================================

/// Port identifier for Dart isolate communication.
/// In real Dart, this is int64_t representing a SendPort's native handle.
typedef int64_t Dart_Port_DL;

/// Enum for Dart_CObject types.
/// Only including types we actually use in tests.
typedef enum {
  Dart_CObject_kNull = 0,
  Dart_CObject_kBool = 1,
  Dart_CObject_kInt32 = 2,
  Dart_CObject_kInt64 = 3,
  Dart_CObject_kDouble = 4,
  Dart_CObject_kString = 5,
  Dart_CObject_kArray = 6,
  Dart_CObject_kTypedData = 7,
  Dart_CObject_kExternalTypedData = 8,
  Dart_CObject_kSendPort = 9,
  Dart_CObject_kCapability = 10,
  Dart_CObject_kNativePointer = 11,
  Dart_CObject_kUnsupported = 12,
  Dart_CObject_kNumberOfTypes = 13,
} Dart_CObject_Type;

/// C representation of a Dart object for FFI.
/// Simplified version that only includes fields we use.
struct Dart_CObject {
  Dart_CObject_Type type;
  union {
    bool as_bool;
    int32_t as_int32;
    int64_t as_int64;
    double as_double;
    const char* as_string;
    struct {
      int64_t id;
      Dart_Port_DL origin_id;
    } as_send_port;
    struct {
      intptr_t length;
      struct Dart_CObject** values;
    } as_array;
  } value;
};

// ============================================================================
// Mock Function Implementation
// ============================================================================

namespace mock_dart_api {

/// Callback type for intercepting Dart_PostCObject calls.
using PostCObjectCallback = std::function<bool(Dart_Port_DL, Dart_CObject*)>;

/// Records of all PostCObject calls for verification.
struct PostCObjectCall {
  Dart_Port_DL port;
  Dart_CObject_Type type;
  int64_t value_as_int64;
};

/// Global state for mock Dart API.
/// Thread safety: Tests should be single-threaded or use proper synchronization.
struct MockState {
  bool initialized = false;
  PostCObjectCallback custom_callback = nullptr;
  std::vector<PostCObjectCall> post_calls;
  bool post_should_fail = false;
};

/// Get global mock state (singleton pattern).
inline MockState& GetMockState() {
  static MockState state;
  return state;
}

/// Reset mock state between tests.
inline void Reset() {
  auto& state = GetMockState();
  state.initialized = false;
  state.custom_callback = nullptr;
  state.post_calls.clear();
  state.post_should_fail = false;
}

/// Set custom callback for PostCObject.
inline void SetPostCObjectCallback(PostCObjectCallback callback) {
  GetMockState().custom_callback = callback;
}

/// Configure PostCObject to fail.
inline void SetPostShouldFail(bool should_fail) {
  GetMockState().post_should_fail = should_fail;
}

/// Get recorded PostCObject calls.
inline const std::vector<PostCObjectCall>& GetPostCalls() {
  return GetMockState().post_calls;
}

}  // namespace mock_dart_api

// ============================================================================
// Mock Dart API DL Functions (matching dart_api_dl.h signatures)
// ============================================================================

/// Mock implementation of Dart_InitializeApiDL.
/// In real Dart, this initializes function pointers for the C API.
/// Our mock just marks the API as initialized.
inline intptr_t Dart_InitializeApiDL(void* data) {
  (void)data;  // Unused in mock
  mock_dart_api::GetMockState().initialized = true;
  return 0;  // Success
}

/// Mock implementation of Dart_PostCObject_DL.
/// In real Dart, this posts a message to a Dart isolate.
/// Our mock records the call and optionally invokes a test callback.
inline bool Dart_PostCObject_DL(Dart_Port_DL port, Dart_CObject* object) {
  auto& state = mock_dart_api::GetMockState();

  // Record the call for verification
  mock_dart_api::PostCObjectCall call;
  call.port = port;
  call.type = object->type;
  call.value_as_int64 = (object->type == Dart_CObject_kInt64)
                            ? object->value.as_int64
                            : 0;
  state.post_calls.push_back(call);

  // Check if we should simulate failure
  if (state.post_should_fail) {
    return false;
  }

  // Invoke custom callback if set
  if (state.custom_callback) {
    return state.custom_callback(port, object);
  }

  return true;  // Success by default
}

#endif  // TEST_MOCK_DART_API_DL_H_
