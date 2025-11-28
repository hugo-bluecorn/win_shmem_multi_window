# C++ Style Guide for Flutter Windows Project

This guide defines C++ coding standards for the Windows native implementation of this Flutter project. It is based on the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with adaptations for Flutter Windows development.

**Last Updated:** 2025-11-28

---

## Table of Contents

1. [General Principles](#general-principles)
2. [Naming Conventions](#naming-conventions)
3. [File Organization](#file-organization)
4. [Formatting](#formatting)
5. [Classes and Structs](#classes-and-structs)
6. [Functions](#functions)
7. [Variables](#variables)
8. [Pointers and References](#pointers-and-references)
9. [Memory Management](#memory-management)
10. [Windows API Conventions](#windows-api-conventions)
11. [Comments and Documentation](#comments-and-documentation)
12. [Error Handling](#error-handling)

---

## General Principles

### Code Philosophy

- **Clarity over cleverness** - Write code that is easy to understand
- **Consistency** - Follow existing patterns in the Flutter Windows codebase
- **Safety** - Prefer RAII, avoid manual memory management where possible
- **Performance** - Be mindful of performance, but don't optimize prematurely

### C++ Version

- **Target:** C++17 (Flutter Windows default)
- **Avoid:** Experimental features or newer standards not supported by MSVC

---

## Naming Conventions

### Files

- **Header files:** `.h` extension
- **Implementation files:** `.cpp` extension
- **Names:** `snake_case` (e.g., `shared_memory_manager.h`)
- **Match class names:** File `shared_memory_manager.h` contains class `SharedMemoryManager`

```cpp
// ✅ Good
shared_memory_manager.h
shared_memory_manager.cpp

// ❌ Bad
SharedMemoryManager.h
sharedMemoryManager.cpp
```

### Classes and Structs

- **Classes:** `PascalCase` (e.g., `SharedMemoryManager`, `WindowCountListener`)
- **Structs:** `PascalCase` (e.g., `SharedMemoryData`, `WindowInfo`)

```cpp
// ✅ Good
class SharedMemoryManager {
  // ...
};

struct SharedMemoryData {
  volatile LONG window_count;
};

// ❌ Bad
class shared_memory_manager { };
class sharedMemoryManager { };
```

### Functions and Methods

- **Public methods:** `PascalCase` (e.g., `IncrementWindowCount()`, `GetHandle()`)
- **Private methods:** `PascalCase` (e.g., `InitializeSharedMemory()`)
- **Functions:** `PascalCase` (consistent with methods)

```cpp
// ✅ Good
class WindowManager {
 public:
  bool CreateWindow();
  void DestroyWindow();

 private:
  void InitializeComponents();
};

// ❌ Bad
class WindowManager {
 public:
  bool create_window();  // Wrong casing
  void Destroy_Window(); // Inconsistent
};
```

### Variables

#### Member Variables

- **Private members:** `snake_case_` with trailing underscore
- **Public members (rare):** `snake_case` without trailing underscore

```cpp
// ✅ Good
class SharedMemoryManager {
 private:
  HANDLE shared_memory_handle_;
  SharedMemoryData* shared_data_;
  bool is_initialized_;
};

// ❌ Bad
class SharedMemoryManager {
 private:
  HANDLE sharedMemoryHandle;   // No trailing underscore
  SharedMemoryData* m_pData;   // Hungarian notation
  bool _isInitialized;         // Leading underscore
};
```

#### Local Variables

- **Local variables:** `snake_case`
- **Function parameters:** `snake_case`

```cpp
// ✅ Good
void ProcessData(int window_count, const std::string& name) {
  int total_count = window_count + 1;
  bool is_valid = ValidateInput(name);
}

// ❌ Bad
void ProcessData(int WindowCount, const std::string& Name) {
  int TotalCount = WindowCount + 1;
  bool IsValid = ValidateInput(Name);
}
```

### Constants

- **Constants:** `kPascalCase` with `k` prefix
- **Macros:** `UPPER_CASE_WITH_UNDERSCORES` (avoid macros when possible)

```cpp
// ✅ Good
const int kDefaultWindowCount = 1;
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
constexpr size_t kSharedMemorySize = 16;

#define MAX_BUFFER_SIZE 1024  // Only when macro is necessary

// ❌ Bad
const int DEFAULT_WINDOW_COUNT = 1;  // Wrong case
const char* SharedMemoryName = "..."; // Wrong case
```

### Namespaces

- **Prefer no namespace** for Flutter Windows runner code (matches existing code)
- **If using namespace:** `snake_case`
- **Never:** `using namespace std;` in headers

```cpp
// ✅ Good (no namespace, matches Flutter Windows style)
class SharedMemoryManager {
  // ...
};

// ✅ Acceptable (if namespace needed)
namespace flutter_multiwindow {
class SharedMemoryManager {
  // ...
};
}  // namespace flutter_multiwindow

// ❌ Bad
using namespace std;  // In header file
namespace FlutterMultiWindow { }  // Wrong case
```

---

## File Organization

### Header File Structure

```cpp
// shared_memory_manager.h
#ifndef RUNNER_SHARED_MEMORY_MANAGER_H_
#define RUNNER_SHARED_MEMORY_MANAGER_H_

// 1. System includes (angle brackets)
#include <windows.h>
#include <string>

// 2. Project includes (quotes)
#include "flutter_window.h"

// 3. Forward declarations (if needed)
class WindowManager;

// 4. Class declaration
class SharedMemoryManager {
 public:
  // Public methods
  SharedMemoryManager();
  ~SharedMemoryManager();

  bool Initialize();
  LONG IncrementWindowCount();

 private:
  // Private methods
  void Cleanup();

  // Private members
  HANDLE shared_memory_handle_;
  SharedMemoryData* shared_data_;
};

#endif  // RUNNER_SHARED_MEMORY_MANAGER_H_
```

### Include Guards

- **Format:** `RUNNER_<FILENAME>_H_`
- **Use `#ifndef`/`#define`/`#endif`** (not `#pragma once`)

```cpp
// ✅ Good
#ifndef RUNNER_SHARED_MEMORY_MANAGER_H_
#define RUNNER_SHARED_MEMORY_MANAGER_H_
// ... content ...
#endif  // RUNNER_SHARED_MEMORY_MANAGER_H_

// ❌ Bad
#pragma once  // Not used in Flutter Windows
```

### Implementation File Structure

```cpp
// shared_memory_manager.cpp

// 1. Own header first
#include "shared_memory_manager.h"

// 2. System includes
#include <iostream>

// 3. Project includes
#include "utils.h"

// 4. Constants (file scope)
namespace {
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
constexpr size_t kSharedMemorySize = 16;
}  // anonymous namespace

// 5. Implementation
SharedMemoryManager::SharedMemoryManager()
    : shared_memory_handle_(nullptr),
      shared_data_(nullptr) {
  // Constructor implementation
}
```

---

## Formatting

### Indentation

- **2 spaces** per indentation level (matches Flutter Windows code)
- **No tabs** - use spaces only

```cpp
// ✅ Good
class Example {
 public:
  void Method() {
    if (condition) {
      DoSomething();
    }
  }
};

// ❌ Bad (4 spaces or tabs)
class Example {
    public:
        void Method() {
            if (condition) {
                DoSomething();
            }
        }
};
```

### Line Length

- **Maximum 80 characters** per line
- Break long lines logically

```cpp
// ✅ Good
bool SharedMemoryManager::CreateSharedMemory(
    const std::string& name,
    size_t size) {
  // Implementation
}

// ❌ Bad (exceeds 80 characters)
bool SharedMemoryManager::CreateSharedMemory(const std::string& name, size_t size) {
  // ...
}
```

### Braces

- **Opening brace on same line** for functions, classes, control structures
- **Closing brace on new line**

```cpp
// ✅ Good
void Function() {
  if (condition) {
    DoSomething();
  } else {
    DoSomethingElse();
  }
}

class MyClass {
 public:
  void Method();
};

// ❌ Bad
void Function()
{  // Opening brace on new line
  if (condition)
  {
    DoSomething();
  }
}
```

### Spacing

- **Space after keywords:** `if (`, `for (`, `while (`
- **No space before `(`:** Function calls `Function()`
- **Space around operators:** `a + b`, `x = y`

```cpp
// ✅ Good
if (condition) {
  result = value1 + value2;
  Function(arg);
}

// ❌ Bad
if(condition){  // Missing spaces
  result=value1+value2;  // Missing spaces
  Function (arg);  // Extra space
}
```

---

## Classes and Structs

### Class Declaration Order

```cpp
class MyClass {
 public:
  // 1. Constructors and destructor
  MyClass();
  ~MyClass();

  // 2. Public methods
  void PublicMethod();

  // 3. Public members (rare, prefer private)

 private:
  // 4. Private methods
  void PrivateHelper();

  // 5. Private members
  int member_variable_;
};
```

### Constructors

- **Use member initializer lists**
- **Initialize all members**
- **Order matches declaration order**

```cpp
// ✅ Good
SharedMemoryManager::SharedMemoryManager()
    : shared_memory_handle_(nullptr),
      shared_data_(nullptr),
      is_initialized_(false) {
  // Constructor body (if needed)
}

// ❌ Bad
SharedMemoryManager::SharedMemoryManager() {
  shared_memory_handle_ = nullptr;  // Assignment, not initialization
  shared_data_ = nullptr;
}
```

### Destructors

- **Always clean up resources**
- **Use RAII pattern** (Resource Acquisition Is Initialization)

```cpp
// ✅ Good
SharedMemoryManager::~SharedMemoryManager() {
  if (shared_data_) {
    UnmapViewOfFile(shared_data_);
    shared_data_ = nullptr;
  }
  if (shared_memory_handle_) {
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
  }
}
```

### Structs vs Classes

- **Struct:** Passive data holder, all public
- **Class:** Has methods and private data

```cpp
// ✅ Good - Struct for data
struct SharedMemoryData {
  volatile LONG window_count;
  DWORD reserved[3];
};

// ✅ Good - Class for behavior
class SharedMemoryManager {
 public:
  bool Initialize();
 private:
  HANDLE handle_;
};

// ❌ Bad - Struct with private members
struct SharedMemoryData {
 private:
  LONG window_count;  // Use class instead
};
```

---

## Functions

### Function Naming

- **Action verbs:** `Create`, `Initialize`, `Get`, `Set`, `Is`, `Has`
- **Clear purpose:** Name describes what the function does

```cpp
// ✅ Good
bool CreateWindow();
void InitializeSharedMemory();
int GetWindowCount();
bool IsInitialized();

// ❌ Bad
void DoStuff();
int Count();  // Ambiguous - get or set?
void Process();  // Too vague
```

### Function Parameters

- **Input parameters:** Pass by `const&` for objects, by value for primitives
- **Output parameters:** Pass by pointer (prefer return values)
- **Order:** Inputs first, then outputs

```cpp
// ✅ Good
bool ProcessData(const std::string& input, int value, std::string* output);
int CalculateSum(int a, int b);  // Return value, not output parameter

// ❌ Bad
bool ProcessData(std::string* output, std::string input);  // Wrong order
void GetValue(int* value);  // Prefer: int GetValue();
```

### Return Values

- **Prefer return values over output parameters**
- **Use `std::optional` for nullable return** (C++17)
- **Use `bool` for success/failure + out parameter for complex objects**

```cpp
// ✅ Good
int GetWindowCount();  // Simple value
std::optional<WindowInfo> GetWindowInfo();  // May not exist
bool GetLargeObject(LargeObject* out);  // Avoid copying

// ❌ Bad
void GetWindowCount(int* count);  // Use return value instead
```

### Function Length

- **Keep functions short** - ideally under 40 lines
- **One responsibility** per function
- **Extract helper functions** if logic is complex

---

## Variables

### Initialization

- **Always initialize variables**
- **Use initialization, not assignment**

```cpp
// ✅ Good
int count = 0;
HANDLE handle = nullptr;
std::string name = "default";

// In classes
MyClass() : member_(0), handle_(nullptr) { }

// ❌ Bad
int count;  // Uninitialized
HANDLE handle;
```

### Scope

- **Minimize variable scope**
- **Declare variables close to first use**

```cpp
// ✅ Good
void Function() {
  if (NeedData()) {
    int data = GetData();  // Declared when needed
    ProcessData(data);
  }
}

// ❌ Bad
void Function() {
  int data;  // Declared too early
  // ... 50 lines ...
  data = GetData();
}
```

### Global Variables

- **Avoid global variables** when possible
- **If needed:** Use `namespace` or `static` in `.cpp` file

```cpp
// ✅ Good - File-scoped constants
namespace {
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
}  // anonymous namespace

// ❌ Bad - Global mutable state
extern int g_window_count;  // Avoid
```

---

## Pointers and References

### Pointer vs Reference

- **Use references** when the argument must exist and won't be reassigned
- **Use pointers** for optional arguments or when null is meaningful

```cpp
// ✅ Good
void ProcessData(const DataObject& data);  // Must exist
void OptionalUpdate(UpdateInfo* info);     // May be null

// ❌ Bad
void ProcessData(const DataObject* data);  // Can't be null, use reference
void MustHaveData(DataObject* data);       // Can't be null, use reference
```

### Pointer Declaration

- **Asterisk/ampersand with type**
- **One pointer per line** (for clarity)

```cpp
// ✅ Good
char* ptr;
const std::string& name;

// Multiple declarations
char* ptr1;
char* ptr2;

// ❌ Bad
char *ptr;  // Asterisk with variable
char* ptr1, ptr2;  // Confusing - ptr2 is not a pointer!
```

### nullptr vs NULL

- **Use `nullptr`**, not `NULL` or `0`

```cpp
// ✅ Good
HANDLE handle = nullptr;
if (handle == nullptr) { }

// ❌ Bad
HANDLE handle = NULL;
if (handle == 0) { }
```

---

## Memory Management

### RAII (Resource Acquisition Is Initialization)

- **Acquire resources in constructor**
- **Release resources in destructor**
- **No manual `delete` in most cases**

```cpp
// ✅ Good - RAII pattern
class SharedMemoryManager {
 public:
  SharedMemoryManager() {
    shared_memory_handle_ = CreateFileMapping(...);
  }

  ~SharedMemoryManager() {
    if (shared_memory_handle_) {
      CloseHandle(shared_memory_handle_);
    }
  }

 private:
  HANDLE shared_memory_handle_;
};

// ❌ Bad - Manual cleanup required
class SharedMemoryManager {
 public:
  void Cleanup() {
    CloseHandle(shared_memory_handle_);  // Caller must remember
  }
};
```

### Smart Pointers

- **Prefer smart pointers** over raw pointers for ownership
- **`std::unique_ptr`** for sole ownership
- **`std::shared_ptr`** for shared ownership (use sparingly)

```cpp
// ✅ Good
std::unique_ptr<DataProcessor> processor = std::make_unique<DataProcessor>();

// ❌ Bad (manual memory management)
DataProcessor* processor = new DataProcessor();
// ... forget to delete ...
```

### Windows Handles

- **Handles are not pointers** - don't use smart pointers
- **Use RAII pattern** with CloseHandle in destructor

```cpp
// ✅ Good
class WindowsResourceManager {
 public:
  WindowsResourceManager() : handle_(CreateEvent(...)) { }
  ~WindowsResourceManager() {
    if (handle_) {
      CloseHandle(handle_);
    }
  }
 private:
  HANDLE handle_;
};

// ❌ Bad
std::unique_ptr<HANDLE> handle;  // Wrong - HANDLE is not a pointer
```

---

## Windows API Conventions

### Handle Naming

- **Suffix `_handle_`** for Windows handles

```cpp
// ✅ Good
HANDLE shared_memory_handle_;
HANDLE event_handle_;

// ❌ Bad
HANDLE shared_memory_;  // Ambiguous
HANDLE h_event;  // Hungarian notation
```

### Error Checking

- **Check all Windows API return values**
- **Use `GetLastError()` for detailed errors**

```cpp
// ✅ Good
HANDLE handle = CreateFileMapping(...);
if (handle == nullptr) {
  DWORD error = GetLastError();
  std::cerr << "CreateFileMapping failed: " << error << std::endl;
  return false;
}

// ❌ Bad
HANDLE handle = CreateFileMapping(...);
// No error checking!
```

### String Types

- **Use `std::string` for internal strings**
- **Convert to `wchar_t*` for Windows API**

```cpp
// ✅ Good
std::string name = "Local\\FlutterMultiWindowCounter";
std::wstring wide_name(name.begin(), name.end());
CreateEventW(nullptr, FALSE, FALSE, wide_name.c_str());

// ❌ Bad
char* name = "Local\\FlutterMultiWindowCounter";  // C-style string
CreateEventA(...);  // Use wide version (W) not ANSI (A)
```

### Atomic Operations

- **Use `Interlocked*` functions** for thread-safe counters
- **`volatile` keyword** for shared memory variables

```cpp
// ✅ Good
struct SharedMemoryData {
  volatile LONG window_count;
};

LONG new_count = InterlockedIncrement(&shared_data_->window_count);

// ❌ Bad
shared_data_->window_count++;  // Not thread-safe!
shared_data_->window_count = shared_data_->window_count + 1;  // Race condition
```

---

## Comments and Documentation

### File Headers

- **Include copyright/license** if required
- **Brief description** of file purpose

```cpp
// shared_memory_manager.h
//
// Manages Windows shared memory for multi-window synchronization.
// Creates a named shared memory section accessible across all
// Flutter window processes.

#ifndef RUNNER_SHARED_MEMORY_MANAGER_H_
#define RUNNER_SHARED_MEMORY_MANAGER_H_
```

### Class Documentation

- **Document class purpose**
- **Document non-obvious behavior**

```cpp
// Manages a Windows shared memory section for cross-process communication.
//
// The first process creates the shared memory, subsequent processes open
// the existing section. All processes map the same physical memory.
//
// Thread-safe: Uses atomic operations for counter updates.
class SharedMemoryManager {
  // ...
};
```

### Function Documentation

- **Document public interface**
- **Describe parameters and return values**
- **Note any preconditions or side effects**

```cpp
// Increments the window count atomically.
//
// Returns the new window count after incrementing.
// Thread-safe: Uses InterlockedIncrement for atomic operation.
LONG IncrementWindowCount();
```

### Inline Comments

- **Explain why, not what**
- **Comment complex logic**
- **Avoid obvious comments**

```cpp
// ✅ Good
// Use Local\ namespace to scope to current login session (not Global\)
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";

// Wait for event with timeout to prevent hanging if window crashes
DWORD result = WaitForSingleObject(event_handle_, 5000);

// ❌ Bad
// Increment the window count
window_count++;  // Obvious from code

// Set handle to null
handle = nullptr;  // Unnecessary
```

### TODO Comments

- **Format:** `// TODO(username): Description`
- **Include context**

```cpp
// ✅ Good
// TODO(username): Add mutex protection for multi-field updates
// TODO(username): Handle ERROR_ACCESS_DENIED gracefully

// ❌ Bad
// TODO: Fix this
// HACK: This is temporary
```

---

## Error Handling

### Return Value Conventions

- **`bool` for success/failure**
- **`nullptr` for failed pointer allocation**
- **Check all error conditions**

```cpp
// ✅ Good
bool Initialize() {
  handle_ = CreateFileMapping(...);
  if (handle_ == nullptr) {
    return false;  // Indicate failure
  }
  return true;
}

// ❌ Bad
void Initialize() {
  handle_ = CreateFileMapping(...);
  // No error handling!
}
```

### Logging Errors

- **Use `std::cerr` for errors**
- **Use `std::cout` for debug info**
- **Include context in error messages**

```cpp
// ✅ Good
if (handle == nullptr) {
  DWORD error = GetLastError();
  std::cerr << "CreateFileMapping failed for '"
            << kSharedMemoryName << "': Error " << error << std::endl;
  return false;
}

// ❌ Bad
if (handle == nullptr) {
  std::cout << "Error" << std::endl;  // No context
}
```

### Assertions

- **Use assertions for invariants**
- **Not for user errors** (use proper error handling)

```cpp
// ✅ Good
#include <cassert>

void ProcessData(DataObject* data) {
  assert(data != nullptr);  // Invariant: caller must provide data
  // ...
}

// ❌ Bad
assert(user_input.length() > 0);  // User error, not invariant
```

---

## Best Practices Summary

### DO

✅ Use RAII for resource management
✅ Initialize all variables
✅ Check all Windows API return values
✅ Use `const` wherever possible
✅ Use references for non-null parameters
✅ Follow existing Flutter Windows code style
✅ Write self-documenting code with clear names
✅ Keep functions short and focused
✅ Use atomic operations for shared memory

### DON'T

❌ Use raw `new`/`delete` (prefer smart pointers or RAII)
❌ Use global mutable state
❌ Ignore return values or error codes
❌ Use Hungarian notation (`m_`, `p`, `sz`, etc.)
❌ Use `using namespace std;` in headers
❌ Write functions longer than 40 lines
❌ Use macros when `const` or `inline` works
❌ Use `NULL` (use `nullptr`)

---

## Examples

### Complete Class Example

```cpp
// shared_memory_manager.h
#ifndef RUNNER_SHARED_MEMORY_MANAGER_H_
#define RUNNER_SHARED_MEMORY_MANAGER_H_

#include <windows.h>

// Shared memory data structure (16 bytes, cache-aligned)
struct SharedMemoryData {
  volatile LONG window_count;
  DWORD reserved[3];
};

// Manages Windows shared memory for multi-window synchronization.
class SharedMemoryManager {
 public:
  SharedMemoryManager();
  ~SharedMemoryManager();

  // Initializes shared memory. Returns true on success.
  bool Initialize();

  // Atomically increments window count. Returns new count.
  LONG IncrementWindowCount();

  // Atomically decrements window count. Returns new count.
  LONG DecrementWindowCount();

  // Returns current window count (non-atomic read).
  LONG GetWindowCount() const;

 private:
  // Creates or opens the shared memory section.
  bool CreateSharedMemory();

  // Cleans up handles and unmaps memory.
  void Cleanup();

  HANDLE shared_memory_handle_;
  SharedMemoryData* shared_data_;
  bool is_initialized_;
};

#endif  // RUNNER_SHARED_MEMORY_MANAGER_H_
```

```cpp
// shared_memory_manager.cpp
#include "shared_memory_manager.h"

#include <iostream>

namespace {
const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
constexpr size_t kSharedMemorySize = sizeof(SharedMemoryData);
}  // anonymous namespace

SharedMemoryManager::SharedMemoryManager()
    : shared_memory_handle_(nullptr),
      shared_data_(nullptr),
      is_initialized_(false) {
}

SharedMemoryManager::~SharedMemoryManager() {
  Cleanup();
}

bool SharedMemoryManager::Initialize() {
  if (is_initialized_) {
    return true;
  }

  if (!CreateSharedMemory()) {
    return false;
  }

  is_initialized_ = true;
  return true;
}

LONG SharedMemoryManager::IncrementWindowCount() {
  if (!is_initialized_ || !shared_data_) {
    std::cerr << "SharedMemoryManager not initialized" << std::endl;
    return -1;
  }

  LONG new_count = InterlockedIncrement(&shared_data_->window_count);
  std::cout << "Window count incremented: " << new_count << std::endl;
  return new_count;
}

LONG SharedMemoryManager::DecrementWindowCount() {
  if (!is_initialized_ || !shared_data_) {
    return -1;
  }

  LONG new_count = InterlockedDecrement(&shared_data_->window_count);
  std::cout << "Window count decremented: " << new_count << std::endl;
  return new_count;
}

LONG SharedMemoryManager::GetWindowCount() const {
  if (!shared_data_) {
    return 0;
  }
  return shared_data_->window_count;
}

bool SharedMemoryManager::CreateSharedMemory() {
  // Create or open shared memory section
  shared_memory_handle_ = CreateFileMappingA(
      INVALID_HANDLE_VALUE,
      nullptr,
      PAGE_READWRITE,
      0,
      kSharedMemorySize,
      kSharedMemoryName);

  if (shared_memory_handle_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "CreateFileMapping failed: " << error << std::endl;
    return false;
  }

  bool already_exists = (GetLastError() == ERROR_ALREADY_EXISTS);

  // Map shared memory to process address space
  shared_data_ = static_cast<SharedMemoryData*>(
      MapViewOfFile(shared_memory_handle_,
                    FILE_MAP_ALL_ACCESS,
                    0,
                    0,
                    kSharedMemorySize));

  if (shared_data_ == nullptr) {
    DWORD error = GetLastError();
    std::cerr << "MapViewOfFile failed: " << error << std::endl;
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
    return false;
  }

  // First process initializes the data
  if (!already_exists) {
    shared_data_->window_count = 0;
    for (int i = 0; i < 3; i++) {
      shared_data_->reserved[i] = 0;
    }
    std::cout << "Shared memory created" << std::endl;
  } else {
    std::cout << "Shared memory opened (already exists)" << std::endl;
  }

  return true;
}

void SharedMemoryManager::Cleanup() {
  if (shared_data_) {
    UnmapViewOfFile(shared_data_);
    shared_data_ = nullptr;
  }

  if (shared_memory_handle_) {
    CloseHandle(shared_memory_handle_);
    shared_memory_handle_ = nullptr;
  }

  is_initialized_ = false;
}
```

---

## References

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Windows API Documentation](https://docs.microsoft.com/en-us/windows/win32/)
- [Flutter Windows Engine](https://github.com/flutter/engine/tree/main/shell/platform/windows)
- [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)

---

## Version History

- **2025-11-28:** Initial version based on Google C++ Style Guide and Flutter Windows conventions

---

**Document Maintainer:** Project Team
**Review Frequency:** Update as needed when patterns evolve
