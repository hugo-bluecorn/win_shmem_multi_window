# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.2.1] - 2025-11-29

### Changed
- **Documentation audit**: Updated all outdated markdown files for v0.2.0 release
  - `CLAUDE.md`: Updated project status from "template setup" to "fully functional"
  - `SHARED_MEMORY_COUNTER_GUIDE.md`: Fixed event type from auto-reset to manual-reset
  - `TEST_IMPLEMENTATION_STATUS.md`: Added resolution status and test results
  - `windows/test/README.md`: Corrected test count to 66 tests

## [0.2.0] - 2025-11-29

### Added
- **RequestWindowClose FFI function**: Proper window close via Win32 WM_CLOSE message
- **Comprehensive C++ test suite**: 66 tests across 5 test suites
  - SharedMemoryManager tests (20 tests)
  - WindowCountListener tests (17 tests)
  - DartPortManager tests (13 tests)
  - CrossProcess tests (10 tests)
  - WindowClose tests (6 tests)
- **Flutter test suite**: 35 tests
  - Unit tests (22 tests)
  - FFI integration tests (5 tests)
  - App integration tests (4 tests)
  - Multi-window sync tests (4 tests)
- **TDD task documentation** for orphan window detection (future work)
- **C++ testing guide** with Google Test patterns
- **Stream-based test patterns** replacing hardcoded Duration delays

### Fixed
- **Cross-process notification bug**: Changed from auto-reset to manual-reset event
  - All windows now receive count updates (not just one)
  - Added Sleep(10ms) + ResetEvent() pattern for reliable cross-process notification
- **Window close decrement bug**: Window count now properly decrements when closing windows
  - Root cause: `exit(0)` bypassed Win32 message loop, skipping OnDestroy()
  - Solution: `RequestWindowClose()` sends WM_CLOSE for proper cleanup path
- **Integration test reliability**: Stream-based waiting patterns instead of Duration delays

### Changed
- `lib/main.dart`: Replaced `exit(0)` with `WindowManagerFFI().closeWindow()`
- `lib/window_manager_ffi.dart`: Added `closeWindow()` method
- `windows/runner/dart_port_manager.cpp`: Added `RequestWindowClose()` export
- `windows/runner/shared_memory_manager.cpp`: Changed to manual-reset event
- `windows/runner/window_count_listener.cpp`: Changed to manual-reset with ResetEvent()

## [0.1.0] - 2025-11-28

### Added
- Initial multi-window Flutter application
- Windows shared memory management (SharedMemoryManager)
- Event-driven notifications (WindowCountListener)
- Dart FFI integration (DartPortManager)
- Real-time window count synchronization
- Create New Window button
- Close This Window button
- Three-layer IPC architecture documentation
