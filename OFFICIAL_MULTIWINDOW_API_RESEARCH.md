# Official Flutter Multi-Window API Research

**Research Date:** 2025-11-28
**Flutter Version Analyzed:** 3.35+ (current stable: 3.38.3)
**Focus:** Windows Desktop Platform

## Executive Summary

Flutter's **official multi-window support** is currently **under active development** with foundational engine work merged in July 2025 (PR #168728). The implementation uses a **shared-engine architecture** where a single Flutter engine manages multiple windows, unlike the separate-engine approach documented in `FINDINGS.md`.

**Current Status:**
- ✅ Engine-level support: **Merged** (Windows, macOS)
- 🚧 Dart API: **Experimental** (requires `--enable-windowing` flag)
- ⏳ Linux support: **Pending**
- ⏳ Public stable API: **Not yet available**

---

## 1. Architecture Comparison

### Our Implementation (FINDINGS.md)
```
Window 1: FlutterWindow → FlutterViewController → Flutter Engine #1
Window 2: FlutterWindow → FlutterViewController → Flutter Engine #2
```
- Separate engines with isolated Dart VMs
- Each `PlatformDispatcher` sees only 1 view
- No direct state sharing
- Matches `desktop_multi_window` package approach

### Official Implementation (PR #168728)
```
Single Flutter Engine → Multiple FlutterHostWindows
                     → FlutterWindowController #1
                     → FlutterWindowController #2
```
- **Shared engine** with single Dart VM
- `PlatformDispatcher.views` returns all windows
- Direct state sharing possible
- Lower memory footprint

---

## 2. Embedder-Level API (C/C++)

### 2.1 Core Embedder Functions

**File:** `shell/platform/embedder/embedder.h`

#### FlutterEngineAddView

```c
typedef struct {
  size_t struct_size;                              // Must be sizeof(FlutterAddViewInfo)
  FlutterViewId view_id;                           // Unique identifier for the view
  const FlutterWindowMetricsEvent* view_metrics;  // View dimensions and DPR
  void* user_data;                                 // Embedder context
  FlutterAddViewCallback add_view_callback;        // Required callback
} FlutterAddViewInfo;

// Callback signature
typedef void (*FlutterAddViewCallback)(const FlutterAddViewResult* result);

// Result structure
typedef struct {
  size_t struct_size;
  bool added;      // Success indicator
  void* user_data; // Echoed from FlutterAddViewInfo
} FlutterAddViewResult;

// Function (signature inferred from usage)
FlutterEngineResult FlutterEngineAddView(
  FlutterEngine engine,
  const FlutterAddViewInfo* info
);
```

**Validation:**
- Engine, info, view_metrics, and add_view_callback must all be provided
- View ID must be unique
- view_metrics->view_id must match info->view_id
- Cannot be used if deprecated `present_layers_callback` is set

**Error Returns:**
- `kInvalidArguments` - Invalid or missing parameters
- Other codes TBD based on engine state

#### FlutterEngineRemoveView

```c
typedef struct {
  size_t struct_size;                                // Must be sizeof(FlutterRemoveViewInfo)
  FlutterViewId view_id;                             // View to remove
  void* user_data;                                   // Embedder context
  FlutterRemoveViewCallback remove_view_callback;    // Required callback
} FlutterRemoveViewInfo;

// Callback signature
typedef void (*FlutterRemoveViewCallback)(const FlutterRemoveViewResult* result);

// Function (signature inferred)
FlutterEngineResult FlutterEngineRemoveView(
  FlutterEngine engine,
  const FlutterRemoveViewInfo* info
);
```

**Important Constraints:**
- Implicit view (ID 0) **cannot be removed**
- Returns `kInvalidArguments` if attempting to remove implicit view
- Callback may execute before raster thread finishes with view (Issue #164564)

**Known Issue:** PR #164571 addresses timing where `removeview` callback is called too early, before raster thread completes rendering.

**Supporting Types:**
```c
typedef int64_t FlutterViewId;
constexpr FlutterViewId kFlutterImplicitViewId = 0;
```

**Sources:**
- [embedder.h (main)](https://github.com/flutter/engine/blob/main/shell/platform/embedder/embedder.h)
- [embedder.cc (main)](https://github.com/flutter/engine/blob/main/shell/platform/embedder/embedder.cc)

---

### 2.2 Windows Platform Implementation

**File:** `shell/platform/windows/flutter_windows_engine.h`

```cpp
class FlutterWindowsEngine {
 public:
  // Create a view that can display this engine's content
  // Returns null on failure
  std::unique_ptr<FlutterWindowsView> CreateView(
      std::unique_ptr<WindowBindingHandler> window);

  // Remove a view - engine will no longer render into it
  virtual void RemoveView(FlutterViewId view_id);

  // Get a view that displays this engine's content
  // Returns null if the view does not exist
  FlutterWindowsView* view(FlutterViewId view_id) const;

 private:
  // Thread-safe view storage
  std::unordered_map<FlutterViewId, FlutterWindowsView*> views_;
  mutable std::shared_mutex views_mutex_;

  // Atomic window counter
  std::atomic<int> window_count_;
};

// Implicit view constant
constexpr FlutterViewId kImplicitViewId = 0;
```

**Thread Safety Design:**
- **Raster thread**: Acquires `shared_lock` (read) on `views_mutex_` for presenting content
- **Platform thread**: Acquires `exclusive_lock` (write) for add/remove operations
- Window count uses `std::atomic<int>` for lock-free access

**Known Issue (GitHub #146248):**
> "FlutterEngineAddView blocks the platform thread eagerly and can cause unnecessary delay in input processing. Should block lazily only when needed."

**New Components (PR #168728):**
- `FlutterHostWindow` - Window abstraction
- `FlutterHostWindowController` - Window lifecycle management
- `FlutterGetWindowHandle()` - FFI export to retrieve native HWND

**Sources:**
- [flutter_windows_engine.h](https://github.com/flutter/engine/blob/main/shell/platform/windows/flutter_windows_engine.h)
- [PR #39824 - Unregister view on destruction](https://github.com/flutter/engine/pull/39824)

---

### 2.3 macOS Platform Implementation

**File:** `shell/platform/darwin/macos/framework/Source/FlutterEngine.mm`

```objc
@interface FlutterEngine
// Internal multi-view support (not exposed publicly yet)
- (void)addViewController:(FlutterViewController*)viewController;
- (void)removeViewController:(FlutterViewController*)viewController;
- (FlutterViewController*)viewControllerForId:(FlutterViewId)viewId;

// Public API (maintains single-view appearance for backward compatibility)
@property (nonatomic, weak) FlutterViewController* viewController;
- (void)setViewController:(FlutterViewController*)viewController;
@end
```

**Design Philosophy (PR #37976):**
> "Publicly FlutterEngine will only handle one (unmanaged) view controller while FlutterEngine will still support multi-view internally, and a future class will expose this functionality."

**View ID System:**
- First view: `kFlutterDefaultViewId` (default view)
- Additional views: Generated IDs
- Views mapped via `FlutterViewEngineProvider`
- Engine maintains **weak references** to view controllers

**Status:**
- Initially merged: Feb 9, 2023
- Reverted: Feb 10, 2023 (broke Swift plugins due to property→method syntax change)
- Re-landed: Feb 12, 2023

**macOS Compositor (PR #52253):**
```cpp
class FlutterCompositor {
  void AddView(FlutterViewId view_id);
  void RemoveView(FlutterViewId view_id);
  // Handles multiple views for rendering
};
```

**Sources:**
- [PR #37976 - macOS multi-view support](https://github.com/flutter/engine/pull/37976)
- [PR #52253 - Multiview compositor](https://github.com/flutter/engine/pull/52253)

---

### 2.4 Linux Platform Implementation

**File:** `shell/platform/linux/fl_engine.cc`

PR #54018 added `fl_engine_add/remove_view` functions for Linux multi-view support.

```c
// Function signatures (inferred from PR)
void fl_engine_add_view(FlEngine* engine, FlView* view);
void fl_engine_remove_view(FlEngine* engine, FlView* view);
```

**Status:**
- Merged: July 23, 2024
- Repository now archived (as of Feb 25, 2025)
- Note from author: "this internal API is not currently used"

**Sources:**
- [PR #54018 - Linux add/remove_view API](https://github.com/flutter/engine/pull/54018)

---

## 3. Dart-Level API

### 3.1 Framework API (Experimental)

**Status:** Experimental, requires feature flag

**Activation:**
```bash
flutter config --enable-windowing
```

**File:** Part of Flutter framework (location TBD in flutter/flutter repository)

### 3.2 RegularWindowController (Win32)

**Source:** PR #173715 - "Implement Regular Windows for the win32 framework"

```dart
import 'package:flutter/material.dart';
import 'package:flutter/services.dart'; // For windowing APIs

// Creating a window
final controller = RegularWindowController(
  delegate: CallbackRegularWindowControllerDelegate(
    onDestroyed: () {
      // Handle window destruction
      windowManager.remove(windowKey);
    },
  ),
  title: "My Window Title",
  preferredSize: Size(800, 600),
);

// Empty window (no initial content)
final emptyController = RegularWindowController.empty();

// Window lifecycle
controller.destroy();           // Close the window
controller.setFullscreen(true); // Toggle fullscreen
bool isFullscreen = controller.isFullscreen;
```

**Key Classes:**

**RegularWindowController**
- Primary API for window creation and management
- Extends `BaseWindowController`
- Implements `ChangeNotifier` for state change notifications

**CallbackRegularWindowControllerDelegate**
- Handles window lifecycle callbacks
- Provides `onDestroyed` callback

**Requirements:**
- Must use **locally-built engine** for testing
- Experimental feature (not stable)
- Windows platform only (macOS/Linux TBD)

**Sources:**
- [PR #173715 - Win32 regular windows](https://github.com/flutter/flutter/pull/173715)

---

### 3.3 PlatformDispatcher Multi-View APIs

**File:** `dart:ui` library

**Existing APIs (available now):**

```dart
import 'dart:ui' as ui;

// Get all views managed by the engine
final views = ui.PlatformDispatcher.instance.views;
// Type: Iterable<FlutterView>

// Get the implicit view (backward compatibility)
final implicitView = ui.PlatformDispatcher.instance.implicitView;
// Type: FlutterView?

// Access specific view properties
for (final view in views) {
  print('View ID: ${view.viewId}');
  print('Size: ${view.physicalSize}');
  print('DPR: ${view.devicePixelRatio}');
  print('Padding: ${view.padding}');
}
```

**Expected Future APIs (not yet available):**

```dart
// Hypothetical API based on design documents
final viewId = await ui.PlatformDispatcher.instance.createView(
  configuration: ViewConfiguration(
    title: 'New Window',
    size: Size(800, 600),
    position: Offset(100, 100),
  ),
);

await ui.PlatformDispatcher.instance.removeView(viewId);
```

**Current Limitation:**
> "No createView or removeView methods exist in dart:ui as of Flutter 3.38.3. View creation currently requires platform-specific code."

**Sources:**
- [PlatformDispatcher API docs](https://api.flutter.dev/flutter/dart-ui/PlatformDispatcher-class.html)
- [FlutterView API docs](https://api.flutter.dev/flutter/dart-ui/FlutterView-class.html)

---

### 3.4 Framework Integration

**Related PRs:**

**PR #20496 - Migration to PlatformDispatcher**
- Created `FlutterView`, `FlutterWindow`, `SingletonFlutterWindow` classes
- Separated view concepts from singleton window

**PR #39553 - PlatformDispatcher.implicitView**
- Introduced low-level primitive for backward compatibility
- Deprecates global `window` singleton

**PR #164353 - Window creation callback**
- Fixes for multi-window creation callbacks

**Sources:**
- [PR #20496 - PlatformDispatcher migration](https://github.com/flutter/engine/pull/20496)
- [PR #39553 - implicitView](https://github.com/flutter/engine/pull/39553)

---

## 4. Timeline and Roadmap

### Completed Milestones

| Date | Event | Status |
|------|-------|--------|
| **Feb 2023** | macOS FlutterEngine multi-view (PR #37976) | ✅ Merged |
| **Jul 2024** | Linux fl_engine_add/remove_view (PR #54018) | ✅ Merged |
| **Jul 2025** | Windows/macOS engine foundation (PR #168728) | ✅ Merged |
| **2025** | Win32 RegularWindowController (PR #173715) | ✅ Merged |

### Current State (Flutter 3.35 - 3.38)

**Stable Channel:**
- ❌ No public multi-window Dart API
- ✅ Engine foundation in place (Windows, macOS)
- ✅ Experimental flag `--enable-windowing` available
- ⚠️ Requires locally-built engine for full functionality

**What Works:**
- Native platform code can create multiple windows
- Engine supports multiple views via embedder API
- PlatformDispatcher.views lists all active views
- Thread-safe view management

**What Doesn't Work Yet:**
- No stable Dart API for window creation
- Linux support pending
- No official documentation or examples
- Experimental APIs may change

### Pending Work (From Issue #142845)

**Short Term:**
- [ ] Finalize desktop multi-view runner APIs (Windows, macOS)
- [ ] Complete Linux embedder updates
- [ ] Plugin ecosystem compatibility (decouple from view initialization)
- [ ] Dart windowing APIs for window management

**Medium Term:**
- [ ] DevTools integration for multi-view debugging
- [ ] Dialog windows, popup menus, tooltips migration
- [ ] Material 3 multi-window demo application
- [ ] Prevention of black flashes on new windows

**Long Term:**
- [ ] Support for multiple vsyncs on different refresh rates
- [ ] Performance optimizations (reduce redundant rendering)
- [ ] Full plugin compatibility across views

**Sources:**
- [Issue #142845 - Multi View for Windows/MacOS](https://github.com/flutter/flutter/issues/142845)
- [Issue #60131 - Migration to multi-window API](https://github.com/flutter/flutter/issues/60131)

---

## 5. Release Channels Analysis

### Flutter 3.24 (August 2024)

**Multi-View Feature: WEB ONLY**

```dart
// Web-specific multi-view (NOT desktop)
// In flutter_bootstrap.js:
await flutter.loader.loadEntrypoint({
  multiViewEnabled: true,
  onEntrypointLoaded: async (engineInitializer) => {
    await engineInitializer.initializeEngine({
      multiViewEnabled: true,
    });
  }
});
```

**Key Point:** Flutter 3.24's multi-view rendering was **exclusively for web applications**, allowing multiple HTML element embeddings.

**Sources:**
- [Flutter 3.24 release announcement](https://medium.com/flutter/flutter-3-24-dart-3-5-204b7d20c45d)
- [Flutter 3.24.0 release notes](https://docs.flutter.dev/release/release-notes/release-notes-3.24.0)

---

### Flutter 3.35 (Date TBD)

**Desktop Multi-Window Foundation Landed**

From Issue #173824:
> "Canonical landed the foundational logic to create and update windows in Windows and macOS (flutter/flutter#168728). Subsequent releases will update Linux and introduce experimental APIs to expose multi-window functionality."

**Status:** Engine support merged, public APIs pending

**Sources:**
- [Issue #173824 - How to Use Multiple Windows](https://github.com/flutter/flutter/issues/173824)

---

### Flutter 3.38.3 (Current Stable)

**Verified in WINDOWING_VERIFICATION.md:**

```
Flutter SDK: 3.38.3 (stable channel)
--enable-windowing: ✅ Works (despite showing "Unavailable")
PlatformDispatcher.views: ✅ Accessible
```

**Capabilities:**
- `PlatformDispatcher.instance.views` - ✅ Available
- `PlatformDispatcher.instance.implicitView` - ✅ Available
- View property access (viewId, physicalSize, etc.) - ✅ Available
- Dart window creation API - ❌ Not available
- RegularWindowController - ⚠️ Experimental (requires custom engine build)

---

### Flutter Master Branch (Development)

**Active Development Areas:**

1. **Windowing APIs** - Experimental window creation/management APIs
2. **Plugin Compatibility** - Ensuring plugins work across multiple views
3. **Linux Support** - Completing embedder updates
4. **Documentation** - Official guides and examples

**Canonical's Fork:**
- May contain preview implementations
- Reference in Issue #173824, but noted as "outdated"

**Sources:**
- [Flutter master branch](https://github.com/flutter/flutter)
- [Flutter engine repository](https://github.com/flutter/engine)

---

## 6. Comparison: Official vs Current Implementation

### Memory Usage

| Architecture | First Window | Additional Windows | Total (2 Windows) |
|--------------|-------------|-------------------|-------------------|
| **Separate Engines** (Current) | ~50-100 MB | ~50-100 MB each | ~100-200 MB |
| **Shared Engine** (Official) | ~50-100 MB | ~10-20 MB each | ~60-120 MB |

**Savings:** ~40-80 MB per additional window

### State Sharing

| Aspect | Separate Engines | Shared Engine |
|--------|-----------------|---------------|
| **Dart VM** | One per window | Single shared VM |
| **Isolate** | Separate | Single shared isolate |
| **State Sharing** | ❌ Requires IPC (FFI, platform channels) | ✅ Direct Dart-to-Dart |
| **PlatformDispatcher.views.length** | 1 (per window) | N (all windows) |
| **Communication** | Platform channels or FFI polling | Native Dart variables |

### Window Creation

**Current (Separate Engines):**
```cpp
// windows/runner/main.cpp
auto window1 = std::make_unique<FlutterWindow>(project);
window1->Create(...);  // Creates new FlutterViewController → new engine

auto window2 = std::make_unique<FlutterWindow>(project);
window2->Create(...);  // Creates another FlutterViewController → another engine
```

**Official (Shared Engine - Conceptual):**
```cpp
// Single engine instance
auto engine = std::make_unique<FlutterWindowsEngine>(project);

// Create multiple views on same engine
auto view1 = engine->CreateView(std::make_unique<WindowBindingHandler>());
auto view2 = engine->CreateView(std::make_unique<WindowBindingHandler>());
```

```dart
// Future Dart API (hypothetical)
final viewId = await PlatformDispatcher.instance.createView(...);
```

### Migration Path

**From Current Implementation to Official API:**

1. **Phase 1:** Continue using separate engines (current approach)
   - No migration needed
   - Works on stable channel
   - No shared state

2. **Phase 2:** Monitor Flutter master for stable API release
   - Track Issue #142845
   - Test with `--enable-windowing` flag
   - Evaluate RegularWindowController

3. **Phase 3:** Migrate to official API when stable
   - Rewrite window creation to use framework APIs
   - Refactor communication (remove FFI/platform channels)
   - Update state management to use shared Dart state
   - Test thoroughly on all target platforms

---

## 7. Testing the Official API (Current Method)

### Prerequisites

```bash
# 1. Enable windowing feature
flutter config --enable-windowing

# 2. Build Flutter from source (required for full functionality)
git clone https://github.com/flutter/flutter.git
cd flutter
./bin/flutter --version

# 3. Switch to master channel (for latest features)
flutter channel master
flutter upgrade
```

### Example Application (Based on PR #173715)

```dart
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

void main() {
  runApp(MultiWindowApp());
}

class MultiWindowApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: MainWindow(),
    );
  }
}

class MainWindow extends StatefulWidget {
  @override
  _MainWindowState createState() => _MainWindowState();
}

class _MainWindowState extends State<MainWindow> {
  final List<RegularWindowController> _windows = [];

  void _createWindow() {
    final controller = RegularWindowController(
      delegate: CallbackRegularWindowControllerDelegate(
        onDestroyed: () {
          setState(() {
            _windows.remove(controller);
          });
        },
      ),
      title: "Window ${_windows.length + 1}",
      preferredSize: Size(600, 400),
    );

    setState(() {
      _windows.add(controller);
    });
  }

  void _removeLastWindow() {
    if (_windows.isNotEmpty) {
      final controller = _windows.removeLast();
      controller.destroy();
      setState(() {});
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Multi-Window Manager')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text('Total Windows: ${_windows.length + 1}'),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: _createWindow,
              child: Text('Create Window'),
            ),
            ElevatedButton(
              onPressed: _removeLastWindow,
              child: Text('Remove Last Window'),
            ),
          ],
        ),
      ),
    );
  }

  @override
  void dispose() {
    for (final controller in _windows) {
      controller.destroy();
    }
    super.dispose();
  }
}
```

**Build and Run:**
```bash
# Requires locally-built engine
flutter run -d windows --local-engine=host_debug_unopt
```

**Expected Result:**
- Main window shows window count
- Clicking "Create Window" opens new windows
- Each window runs on **shared engine**
- State updates reflect across all windows

---

## 8. Known Issues and Limitations

### Embedder API Issues

**Issue #146248:** FlutterEngineAddView blocks platform thread eagerly
- **Impact:** Input processing delays
- **Workaround:** None (engine-level fix required)
- **Status:** Open

**Issue #164564:** FlutterEngineRemoveView callback executes too early
- **Impact:** Crashes if embedder accesses view during raster thread rendering
- **Fix:** PR #164571 (Only call removeview callback when raster thread is done)
- **Status:** Fixed in PR #164571

### Platform-Specific Limitations

**Windows:**
- Implicit view metrics use placeholder (0x0) until first reconfiguration event
- Window message routing requires careful threading

**macOS:**
- Public API still maintains single-view appearance
- Multi-view support is internal only
- Future class will expose full functionality

**Linux:**
- Embedder updates incomplete
- fl_engine_add/remove_view API exists but unused

### Framework Limitations

**Plugin Compatibility:**
- Many plugins assume single view
- Need to decouple plugin registration from view initialization
- Plugins must handle scenarios without available views

**Current Workarounds:**
- Use `desktop_multi_window` package (separate engines)
- Implement custom platform channels
- Use FFI for native window management (as in COUNTER_APP_IMPLEMENTATION_GUIDE.md)

---

## 9. Recommendations

### For Production Apps (Now - 2025)

**Option 1: Continue Separate-Engine Approach**
```
✅ Works on stable Flutter
✅ No experimental flags required
✅ Well-tested pattern (matches desktop_multi_window)
❌ Higher memory usage
❌ No shared Dart state
❌ Requires IPC for communication
```

**Best for:** Apps that need multi-window **now** and can tolerate isolated windows

**Option 2: Use desktop_multi_window Package**
```
✅ Mature ecosystem package
✅ Built-in platform channel helpers
✅ Active maintenance
✅ Same separate-engine architecture
❌ External dependency
❌ No shared state
```

**Best for:** Production apps needing multi-window with community support

### For Future-Ready Apps (2026+)

**Wait for Official Stable API**
```
✅ Shared engine (lower memory)
✅ Direct state sharing
✅ Official Flutter support
✅ Better performance
⏳ Not yet stable
⏳ Requires master channel
```

**Monitoring Strategy:**
1. Watch Issue #142845 for updates
2. Test with `--enable-windowing` on master channel
3. Evaluate RegularWindowController API
4. Plan migration when APIs stabilize

**Best for:** Apps with flexible timeline that can wait for official support

### Hybrid Approach

**Use separate engines now, design for migration:**
1. Isolate window communication in dedicated modules
2. Use abstract interfaces for window creation
3. Keep state management compatible with shared-engine model
4. Monitor Flutter releases for API stability

```dart
// Abstraction example
abstract class WindowManager {
  Future<WindowId> createWindow(WindowConfig config);
  Future<void> closeWindow(WindowId id);
  Stream<WindowEvent> get windowEvents;
}

// Current implementation: FFI-based (separate engines)
class FFIWindowManager implements WindowManager { ... }

// Future implementation: Official API (shared engine)
class FlutterWindowManager implements WindowManager { ... }
```

---

## 10. Key Differences from FINDINGS.md

### What FINDINGS.md Got Right ✅

1. **Separate engines in current implementation** - Confirmed
2. **Each FlutterViewController creates own engine** - Confirmed at line flutter_window.cpp:21-22
3. **PlatformDispatcher shows View Count: 1 per window** - Confirmed (separate dispatchers)
4. **Official design uses shared engine** - Confirmed by PR #168728
5. **Need for platform channels/IPC in current approach** - Confirmed

### What's New from This Research 🆕

1. **Official API is partially available**
   - FINDINGS.md: "Official API not yet available"
   - REALITY: Embedder API exists, Dart API experimental

2. **Concrete API signatures discovered**
   - FlutterEngineAddView/RemoveView structs documented
   - RegularWindowController API found in PR #173715
   - Windows CreateView/RemoveView methods identified

3. **Timeline clarification**
   - FINDINGS.md: "Wait for full release"
   - REALITY: Engine merged July 2025, Dart API in development

4. **Testing path identified**
   - Can test with `--enable-windowing` flag
   - RegularWindowController available (requires custom build)
   - Master channel has active development

5. **Specific PRs and issues tracked**
   - PR #168728: Foundation (merged)
   - PR #173715: Win32 windows (merged)
   - Issue #142845: Overall tracking
   - Issue #146248: Known performance issue

### Updated Conclusions

**FINDINGS.md Conclusion:**
> "Official shared-engine multi-view not yet available - Likely requires unreleased engine APIs"

**UPDATED Conclusion (2025-11-28):**
> "Official shared-engine multi-view **engine APIs are merged** as of July 2025. Dart framework APIs are **experimental** and require `--enable-windowing` flag + custom engine build. Stable public API expected in future Flutter releases (monitor Issue #142845)."

---

## 11. References and Sources

### Official Flutter Documentation
- [PlatformDispatcher API](https://api.flutter.dev/flutter/dart-ui/PlatformDispatcher-class.html)
- [FlutterView API](https://api.flutter.dev/flutter/dart-ui/FlutterView-class.html)
- [Flutter Windows Embedder API](https://api.flutter.dev/windows-embedder/classflutter_1_1_flutter_windows_engine.html)
- [Multiple Flutter Screens/Views](https://docs.flutter.dev/add-to-app/multiple-flutters)
- [Flutter 3.24 Release Notes](https://docs.flutter.dev/release/release-notes/release-notes-3.24.0)

### GitHub Issues (flutter/flutter)
- [Issue #142845 - Multi View for Windows/MacOS](https://github.com/flutter/flutter/issues/142845)
- [Issue #60131 - Migration to multi-window API](https://github.com/flutter/flutter/issues/60131)
- [Issue #30701 - Support multiple windows for desktop](https://github.com/flutter/flutter/issues/30701)
- [Issue #173824 - How to Use Multiple Windows in 3.35](https://github.com/flutter/flutter/issues/173824)
- [Issue #164564 - FlutterEngineRemoveView callback timing](https://github.com/flutter/flutter/issues/164564)
- [Issue #146248 - FlutterEngineAddView platform thread blocking](https://github.com/flutter/flutter/issues/146248)

### GitHub Pull Requests (flutter/flutter)
- [PR #168728 - Multi-window support (engine)](https://github.com/flutter/flutter/pull/168728)
- [PR #173715 - Win32 regular windows implementation](https://github.com/flutter/flutter/pull/173715)
- [PR #164353 - Fix window creation callback](https://github.com/flutter/flutter/pull/164353)
- [PR #164571 - Only call removeview callback when raster done](https://github.com/flutter/flutter/pull/164571)

### GitHub Pull Requests (flutter/engine)
- [PR #37976 - macOS FlutterEngine multi-view support](https://github.com/flutter/engine/pull/37976)
- [PR #39824 - Unregister FlutterWindowsView on destruction](https://github.com/flutter/engine/pull/39824)
- [PR #52253 - macOS Multiview compositor](https://github.com/flutter/engine/pull/52253)
- [PR #54018 - Linux fl_engine_add/remove_view](https://github.com/flutter/engine/pull/54018)
- [PR #20496 - PlatformDispatcher migration](https://github.com/flutter/engine/pull/20496)
- [PR #39553 - PlatformDispatcher.implicitView](https://github.com/flutter/engine/pull/39553)

### Source Code Files
- [embedder.h](https://github.com/flutter/engine/blob/main/shell/platform/embedder/embedder.h)
- [embedder.cc](https://github.com/flutter/engine/blob/main/shell/platform/embedder/embedder.cc)
- [flutter_windows_engine.h](https://github.com/flutter/engine/blob/main/shell/platform/windows/flutter_windows_engine.h)

### Design Documents
- [Desktop Multi-Window Support (Public)](https://docs.google.com/document/d/11_4wntz_9IJTQOo_Qhp7QF4RfpIMTfVygtOTxQ4OGHY/edit)

### Community Resources
- [desktop_multi_window package](https://pub.dev/packages/desktop_multi_window)
- [David Serrano - How to Use Multiple Windows in Flutter Desktop](https://davidserrano.io/how-to-use-multiple-windows-in-flutter-desktop)
- [Medium - Multi window support with Flutter Desktop](https://medium.com/@alexsinelnikov/multi-window-support-with-flutter-desktop-21f5f9281c7b)
- [Canonical - Bringing multiple windows to Flutter desktop apps](https://ubuntu.com/blog/multiple-window-flutter-desktop)

### Release Announcements
- [Flutter 3.24 Announcement (Medium)](https://medium.com/flutter/flutter-3-24-dart-3-5-204b7d20c45d)
- [What's new in Flutter 3.24 (Medium)](https://medium.com/flutter/whats-new-in-flutter-3-24-6c040f87d1e4)

---

## 12. Glossary

**Embedder API** - C-level API for integrating Flutter engine into native applications

**FlutterEngine** - Core rendering engine that executes Dart code and manages rendering pipeline

**FlutterView** - Represents a rendering surface (window) that displays Flutter content

**FlutterViewController** - Platform-specific controller managing a FlutterView

**PlatformDispatcher** - Dart API singleton managing all views and platform events

**Implicit View** - The default first view (ID 0) created automatically, cannot be removed

**View ID** - Unique int64_t identifier for each view (0 = implicit view)

**Shared Engine Architecture** - Single Flutter engine managing multiple views (official design)

**Separate Engine Architecture** - Multiple Flutter engines, one per window (current spike implementation)

**Raster Thread** - Background thread handling rendering and compositing

**Platform Thread** - Main UI thread handling user input and window events

**RegularWindowController** - Experimental Dart API for window creation (Win32)

**WindowBindingHandler** - Native interface for platform-specific window operations

---

## Appendix A: Testing Checklist

When official API becomes stable, verify:

- [ ] Can create windows from Dart without custom engine build
- [ ] PlatformDispatcher.views.length reflects actual window count
- [ ] State sharing works between windows (test with Provider/Riverpod)
- [ ] Window close doesn't affect other windows
- [ ] Memory usage is ~40-80 MB lower per additional window vs separate engines
- [ ] All plugins work correctly across multiple views
- [ ] Hot reload preserves all windows
- [ ] Window positioning and sizing APIs work as expected
- [ ] Fullscreen toggle works per-window
- [ ] Main window close can close all windows (configurable)

---

## Appendix B: Migration Code Example

**Before (Separate Engines - Current):**
```cpp
// windows/runner/main.cpp
auto window1 = std::make_unique<FlutterWindow>(project);
window1->Create(L"Window 1", origin, Size(800, 600));

auto window2 = std::make_unique<FlutterWindow>(project);
window2->Create(L"Window 2", Point(830, 10), Size(800, 600));
```

**After (Shared Engine - Future):**
```dart
// main.dart
import 'package:flutter/material.dart';
import 'dart:ui' as ui;

void main() async {
  // Main window (implicit view)
  runApp(MainApp());

  // Create additional window
  final viewId = await ui.PlatformDispatcher.instance.createView(
    configuration: ui.ViewConfiguration(
      title: 'Window 2',
      geometry: Rect.fromLTWH(830, 10, 800, 600),
    ),
  );

  // Both windows share same Dart state!
}
```

---

**Document Version:** 1.0
**Last Updated:** 2025-11-28
**Next Review:** When Flutter announces stable multi-window API
