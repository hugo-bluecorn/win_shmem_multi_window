# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Windows Shared Memory Multi-Window Flutter Application**

This is a Flutter Windows desktop project implementing multi-window functionality using **Windows shared memory and event-driven IPC** (Inter-Process Communication). Unlike Flutter's official shared-engine multi-window API (currently experimental), this implementation uses separate Flutter engines per window with Windows kernel objects for instant cross-process state synchronization.

**Core Technologies:**
- Flutter Windows Desktop (Dart 3.10.1+)
- Windows Shared Memory API (`CreateFileMapping`, `MapViewOfFile`)
- Windows Events (`CreateEvent`, `SetEvent`, `WaitForSingleObject`)
- Dart FFI with native ports (`Dart_PostCObject`, `ReceivePort`)
- C++ atomic operations (`InterlockedIncrement`/`InterlockedDecrement`)

**Project Status:** v0.2.0 - Fully functional with 101 tests (66 C++, 35 Flutter). Multi-window creation, closing, and real-time synchronization working.

---

## Essential Commands

### Build & Run

```bash
# Clean and rebuild
flutter clean
flutter pub get
flutter run -d windows

# Build release
flutter build windows --release

# Run with verbose output
flutter run -d windows -v

# Analyze code
flutter analyze
```

### Testing

```bash
# Run all tests
flutter test

# Run specific test file
flutter test test/path/to/test_file.dart

# Run with coverage
flutter test --coverage

# Widget tests (uses flutter test)
flutter test test/features/

# Unit tests (can use dart test for non-Flutter code)
dart test test/core/
```

### TDD Workflow Commands

This project includes TDD workflow automation:

```bash
# Create new TDD task
/tdd-new <feature-description>

# Run tests with auto-detection
/tdd-test [optional-test-file]

# Execute full Red-Green-Refactor cycle
/tdd-implement <path-to-tdd-task-file>
```

See `.claude/commands/` for detailed command documentation.

---

## Architecture: Event-Driven Multi-Window IPC

### Design Philosophy

**Separate Engines + Shared Memory** (Current Implementation)
- Each window runs its own Flutter engine instance
- Windows shared memory stores global state (e.g., window counter)
- Event-driven notifications eliminate polling overhead
- **Zero CPU usage when idle** (<1% vs 10-20% with polling)
- **Instant updates** (<10ms latency via Windows events)

**vs. Official Shared Engine API** (Flutter 3.35+, experimental)
- Single engine managing multiple views
- Lower memory per window (~40-80 MB savings)
- Direct Dart state sharing
- Requires `--enable-windowing` flag + custom engine build
- Not yet stable (see `OFFICIAL_MULTIWINDOW_API_RESEARCH.md`)

### Three-Layer Architecture

```
┌─────────────────────────────────────────────────┐
│ Layer 1: Windows Shared Memory                 │
│  - Name: "Local\FlutterMultiWindowCounter"    │
│  - Atomic operations (InterlockedIncrement)    │
│  - Thread-safe across processes               │
└─────────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────────┐
│ Layer 2: Event Notification                    │
│  - Event: "Local\FlutterWindowCountChanged"    │
│  - Background threads wait (WaitForSingleObject)│
│  - Signaled on state change (SetEvent)         │
└─────────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────────┐
│ Layer 3: Dart Communication                    │
│  - DartPortManager registers SendPorts         │
│  - Dart_PostCObject() to all isolates          │
│  - ReceivePort.listen() → setState()           │
└─────────────────────────────────────────────────┘
```

**Event Flow:**
1. Window created/destroyed → `SharedMemoryManager::IncrementWindowCount()`
2. Atomic increment → `InterlockedIncrement(&shared_data_->window_count)`
3. Signal event → `SetEvent(update_event_)`
4. Listener threads wake → `WaitForSingleObject` returns
5. Notify Dart → `DartPortManager::NotifyWindowCountChanged()`
6. Post to isolates → `Dart_PostCObject()` to all registered ports
7. Dart receives → `ReceivePort.listen()` → `setState()`

**Result:** Zero-latency updates with minimal CPU usage (no polling timers).

---

## Key Implementation Files

### C++ Native Layer (windows/runner/)

**Shared Memory Management:**
- `shared_memory_manager.{h,cpp}` - Creates/manages named shared memory
- `window_count_listener.{h,cpp}` - Event listener background thread
- `dart_port_manager.{h,cpp}` - Manages Dart isolate communication

**Window Management:**
- `window_manager.{h,cpp}` - Window lifecycle, FFI exports
- `flutter_window.{h,cpp}` - Flutter window wrapper
- `win32_window.{h,cpp}` - Win32 window abstraction
- `main.cpp` - Application entry point

**Build Configuration:**
- `windows/runner/CMakeLists.txt` - Add new .cpp files here
- `windows/CMakeLists.txt` - Project-level build config

### Dart Layer (lib/)

- `lib/main.dart` - Flutter UI and ReceivePort event handling
- `lib/window_manager_ffi.dart` - FFI bindings to native functions (not yet implemented)

---

## Development Guidelines

### Adding C++ Native Code

When creating new native functionality:

1. **Create header and implementation files** in `windows/runner/`
2. **Update CMakeLists.txt:**
   ```cmake
   # windows/runner/CMakeLists.txt
   add_executable(${BINARY_NAME} WIN32
     # ... existing files ...
     "your_new_file.cpp"  # ADD HERE
   )
   ```
3. **Include Dart API if posting to Dart:**
   ```cpp
   #include <dart_api.h>
   ```

### FFI Function Exports

All C++ functions callable from Dart must be exported:

```cpp
// C++ (windows/runner/window_manager.cpp)
extern "C" {
__declspec(dllexport) ReturnType FunctionName(params) {
  // Implementation
}
}
```

Corresponding Dart FFI bindings:

```dart
// lib/window_manager_ffi.dart
typedef FunctionNameNative = ReturnType Function(NativeParamTypes);
typedef FunctionNameDart = DartReturnType Function(DartParamTypes);

final _functionName = DynamicLibrary.process()
    .lookupFunction<FunctionNameNative, FunctionNameDart>('FunctionName');
```

### Thread Safety Patterns

**For shared memory:**
- Use `InterlockedIncrement`/`Decrement` for counter operations
- Never use direct assignments to `volatile LONG` fields

**For local data structures:**
- Use `std::mutex` with `std::lock_guard` for protection
- Example: `DartPortManager::send_ports_` uses `ports_mutex_`

**For Windows events:**
- Auto-reset events don't require manual reset after `WaitForSingleObject`
- Manual-reset events need `ResetEvent()` call

### Flutter & Dart Best Practices

This project follows standard Flutter conventions (see `.claude/flutter-dart-rules.md` for complete guidelines):

**Naming Conventions:**
- Classes/Types: `PascalCase` (e.g., `WindowManager`, `SharedMemoryData`)
- Members/Variables/Functions: `camelCase` (e.g., `windowCount`, `createWindow()`)
- Files: `snake_case` (e.g., `window_manager_ffi.dart`, `counter_screen.dart`)
- Constants: `lowerCamelCase` (e.g., `kImplicitViewId`)

**Code Structure:**
- Prefer composition over inheritance
- Keep functions under 20 lines
- Line length max 80 characters
- Separate ephemeral state from app state
- Use sound null safety (avoid `!` unless guaranteed)

**Async Patterns:**
- Use `async`/`await` for asynchronous operations
- Use `Future` for single async values
- Use `Stream` for sequences of async events
- Always handle errors with try-catch blocks

---

## TDD Workflow

This project uses Test-Driven Development with Flutter-specific patterns.

### Test Types

**Unit Tests** (`package:test`):
- Pure Dart functions and classes
- Business logic, validators, formatters
- Service classes, repositories
- Run with: `dart test` or `flutter test`

**Widget Tests** (`package:flutter_test`):
- UI components and interactions
- State changes, navigation
- Run with: `flutter test`

**Integration Tests** (`package:integration_test`):
- Complete user flows
- End-to-end scenarios
- Run with: `flutter test integration_test/`

### TDD Cycle Commands

**1. Create New Feature Task:**
```bash
/tdd-new Add multi-window counter functionality
```
Creates:
- `notes/features/add-multi-window-counter-functionality.md` - Feature planning
- `.claude/tdd-tasks/add-multi-window-counter-functionality.md` - TDD specification

**2. Run Tests:**
```bash
/tdd-test                                    # All tests
/tdd-test test/features/counter/            # Specific directory
/tdd-test test/features/counter_test.dart   # Specific file
```

**3. Implement with TDD:**
```bash
/tdd-implement .claude/tdd-tasks/add-multi-window-counter-functionality.md
```

Executes full Red-Green-Refactor cycle with approval-gated commits.

See `.claude/flutter-tdd-guide.md` for detailed testing patterns.

---

## Implementation Guides

### SHARED_MEMORY_COUNTER_GUIDE.md (1300+ lines)

Comprehensive step-by-step guide for implementing the complete shared memory system:

**Part 1: Native Implementation (C++)**
- SharedMemoryManager class with CreateFileMapping
- WindowCountListener with event-driven threads
- DartPortManager for isolate communication
- CMakeLists.txt build configuration

**Part 2: Dart Implementation**
- FFI bindings and type definitions
- Event-driven updates (NO Timer.periodic)
- ReceivePort integration

**Part 3: Build and Run**
- Build commands and expected output
- Console debugging and verification

**Part 4: Verification & Testing**
- CPU usage verification (<1% vs 10-20%)
- Update latency tests (<10ms)
- Process Explorer shared memory inspection

**Part 5: Troubleshooting**
- Common errors and solutions
- Debug logging patterns
- Thread lifecycle issues

**Part 6: Performance Comparison**
- FFI polling vs shared memory + events
- CPU, latency, memory, scalability metrics

**Part 7: Advanced Topics**
- Multiple data fields in shared memory
- Named mutexes for exclusive access
- Multiple events for different notifications

**Use this guide when:**
- Implementing the shared memory system from scratch
- Debugging IPC communication issues
- Understanding the event-driven architecture
- Optimizing performance
- Extending with additional shared state

### OFFICIAL_MULTIWINDOW_API_RESEARCH.md (1000+ lines)

Research on Flutter's official multi-window API and migration path:

**Key Findings:**
- Engine-level support merged (July 2025, PR #168728)
- Dart API experimental (requires `--enable-windowing`)
- Shared-engine architecture vs separate-engine (current)
- `RegularWindowController` API for Win32
- Memory savings: ~40-80 MB per additional window
- Direct Dart state sharing vs IPC

**Timeline:**
- Flutter 3.24: Web multi-view only
- Flutter 3.35: Engine foundation landed
- Flutter 3.38.3: Experimental APIs available
- Future: Stable public API (monitor Issue #142845)

**Recommendations:**
- Production apps: Continue separate-engine approach
- Future-ready apps: Monitor official API development
- Hybrid: Design for migration, implement with current approach

**Use this guide when:**
- Evaluating migration to official API
- Understanding architectural trade-offs
- Planning long-term Flutter multi-window strategy
- Testing experimental Flutter features

---

## Debugging

### Console Output

Running via `flutter run -d windows` provides console output:

**C++:** Use `std::cout` / `std::cerr`
```cpp
std::cout << "Window count incremented: " << new_count << std::endl;
std::cerr << "Error: " << GetLastError() << std::endl;
```

**Dart:** Use `print()`
```dart
print('[$_windowId] Received window count update: $message');
```

### Windows-Specific Debugging

**Process Explorer (Sysinternals):**
1. View → Lower Pane View → Handles
2. Find your process
3. Search for "FlutterMultiWindowCounter"
4. Verify shared memory section handle

**Expected handles:**
- `Section\BaseNamedObjects\Local\FlutterMultiWindowCounter` (shared memory)
- `Event\BaseNamedObjects\Local\FlutterWindowCountChanged` (event)

### Common Issues

**"Shared memory already exists"**
- Normal - multiple instances share memory
- Code handles `ERROR_ALREADY_EXISTS` gracefully

**Events not signaling**
- Add debug logging in `NotifyWindowCountChanged()`
- Check `SetEvent()` return value
- Verify event handle is valid

**Dart not receiving messages**
- Verify port registration in C++ and Dart
- Check `Dart_PostCObject()` return value
- Add logging in `ReceivePort.listen()`

---

## Version Control Workflow

This project uses an **approval-gated commit workflow** (see `.claude/version-control.md`):

1. Claude Code stages changes and generates commit message
2. You review and approve/modify
3. Claude Code updates documentation (CHANGELOG.md always, README.md when needed)
4. Commit executes with approved message

**Commit Message Format (Conventional Commits):**
```
feat: Add shared memory manager for window synchronization
fix: Resolve event signaling race condition
refactor: Extract Dart port management to separate class
docs: Update architecture diagram in SHARED_MEMORY_COUNTER_GUIDE
test: Add unit tests for SharedMemoryManager
chore: Update CMakeLists.txt for new source files
```

**Before Every Commit:**
- Run `flutter analyze` and fix issues
- Run `flutter test` and ensure all pass
- Update `CHANGELOG.md` (always required)
- Update `README.md` if user-facing changes
- Format code: `dart format .`

---

## Project Structure

```
win_shmem_multi_window/
├── lib/                        # Dart application code
│   └── main.dart              # Flutter entry point (basic Hello World)
├── test/                       # Unit and widget tests
├── integration_test/           # Integration tests
├── windows/                    # Windows platform code
│   ├── runner/                # Native Windows code
│   │   ├── main.cpp          # WinMain entry point
│   │   ├── flutter_window.*  # Flutter window wrapper
│   │   ├── win32_window.*    # Win32 window abstraction
│   │   └── CMakeLists.txt    # Build config (add .cpp files here)
│   └── CMakeLists.txt         # Project-level build config
├── .claude/                    # Claude Code configuration
│   ├── commands/              # TDD workflow commands
│   ├── templates/             # Feature and task templates
│   ├── flutter-dart-rules.md # Flutter/Dart best practices (888 lines)
│   ├── flutter-tdd-guide.md  # TDD patterns for Flutter (695 lines)
│   ├── project-structure.md  # Architecture guidance
│   └── version-control.md    # Git workflow
├── notes/                      # Feature planning (created by /tdd-new)
├── SHARED_MEMORY_COUNTER_GUIDE.md  # Complete implementation guide (1300 lines)
├── OFFICIAL_MULTIWINDOW_API_RESEARCH.md # Official API research (1000 lines)
├── pubspec.yaml               # Dependencies
├── analysis_options.yaml      # Linter configuration
└── CLAUDE.md                  # This file
```

**Key Documentation:**
- `.claude/flutter-dart-rules.md` - Complete Flutter/Dart coding standards
- `.claude/flutter-tdd-guide.md` - Test patterns and examples
- `.claude/project-structure.md` - Directory organization patterns
- `SHARED_MEMORY_COUNTER_GUIDE.md` - Implementation walkthrough
- `OFFICIAL_MULTIWINDOW_API_RESEARCH.md` - Official API research

---

## Next Steps (Implementation Roadmap)

The project currently has:
- ✅ Flutter template setup
- ✅ TDD workflow commands configured
- ✅ Comprehensive implementation guide written
- ✅ Official API research completed

To implement the shared memory multi-window system:

1. **Follow SHARED_MEMORY_COUNTER_GUIDE.md** step-by-step
2. **Use TDD workflow:** `/tdd-new` → write tests → `/tdd-implement`
3. **Add C++ files to CMakeLists.txt** as you create them
4. **Test incrementally:** Build and verify each component
5. **Verify performance:** CPU usage, update latency, shared memory handles

**Recommended implementation order:**
1. SharedMemoryManager (shared memory creation)
2. WindowCountListener (event-driven updates)
3. DartPortManager (Dart communication)
4. Window lifecycle integration
5. Dart FFI bindings
6. UI implementation with ReceivePort

---

## Additional Resources

**Flutter Documentation:**
- [Flutter Desktop](https://docs.flutter.dev/platform-integration/windows/building)
- [Dart FFI](https://dart.dev/guides/libraries/c-interop)
- [Flutter Testing](https://docs.flutter.dev/testing)
- [Effective Dart](https://dart.dev/effective-dart)

**Windows API:**
- [Shared Memory](https://docs.microsoft.com/en-us/windows/win32/memory/creating-named-shared-memory)
- [Events](https://docs.microsoft.com/en-us/windows/win32/sync/using-event-objects)
- [Interlocked Operations](https://docs.microsoft.com/en-us/windows/win32/sync/interlocked-variable-access)

**Flutter Multi-Window:**
- [Issue #142845 - Multi View for Windows/MacOS](https://github.com/flutter/flutter/issues/142845)
- [desktop_multi_window package](https://pub.dev/packages/desktop_multi_window)

---

**Configuration Version:** Template v1.0
**Last Updated:** 2025-11-28
**Flutter SDK:** 3.10.1+
**Target Platform:** Windows Desktop
