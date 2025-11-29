// dart_api_dl.h - Redirect to mock for test builds
//
// This file exists in the test directory to intercept includes of
// dart_api_dl.h and redirect them to our mock implementation.
// CMake's include path order ensures this file is found before the real one.

#ifndef TEST_DART_API_DL_REDIRECT_H_
#define TEST_DART_API_DL_REDIRECT_H_

#include "mock_dart_api_dl.h"

#endif  // TEST_DART_API_DL_REDIRECT_H_
