#include "flutter_window.h"

#include <iostream>
#include <optional>

#include "dart_port_manager.h"
#include "flutter/generated_plugin_registrant.h"

FlutterWindow::FlutterWindow(const flutter::DartProject& project)
    : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  // Initialize shared memory manager for multi-window synchronization
  shared_memory_manager_ = std::make_unique<SharedMemoryManager>();
  if (!shared_memory_manager_->Initialize()) {
    std::cerr << "Failed to initialize SharedMemoryManager" << std::endl;
    // Continue anyway - shared memory is not critical for basic functionality
  } else {
    // Increment window count to track active windows
    shared_memory_manager_->IncrementWindowCount();
  }

  // Start event listener for window count change notifications
  window_count_listener_ = std::make_unique<WindowCountListener>();

  // Set callback to notify Dart isolates when window count changes.
  // Lambda captures shared_memory_manager_ to read current count.
  window_count_listener_->SetCallback([this](LONG /* placeholder */) {
    // Read current window count from shared memory
    LONG current_count = shared_memory_manager_->GetWindowCount();

    // Update global count for new port registrations
    SetCurrentWindowCount(current_count);

    // Notify all registered Dart isolates (Layer 3)
    GetGlobalDartPortManager().NotifyWindowCountChanged(current_count);
  });

  if (!window_count_listener_->Start()) {
    std::cerr << "Failed to start WindowCountListener" << std::endl;
    // Continue anyway - listener is not critical for basic functionality
  }

  // Update global window count so newly registered Dart ports receive it.
  // This ensures Dart UI gets the current count immediately on registration.
  LONG current_count = shared_memory_manager_->GetWindowCount();
  SetCurrentWindowCount(current_count);

  RECT frame = GetClientArea();

  // The size here must match the window dimensions to avoid unnecessary surface
  // creation / destruction in the startup path.
  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
      frame.right - frame.left, frame.bottom - frame.top, project_);
  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller_->engine() || !flutter_controller_->view()) {
    return false;
  }
  RegisterPlugins(flutter_controller_->engine());
  SetChildContent(flutter_controller_->view()->GetNativeWindow());

  flutter_controller_->engine()->SetNextFrameCallback([&]() {
    this->Show();
  });

  // Flutter can complete the first frame before the "show window" callback is
  // registered. The following call ensures a frame is pending to ensure the
  // window is shown. It is a no-op if the first frame hasn't completed yet.
  flutter_controller_->ForceRedraw();

  return true;
}

void FlutterWindow::OnDestroy() {
  // Decrement window count before destroying
  if (shared_memory_manager_) {
    shared_memory_manager_->DecrementWindowCount();
  }

  // Stop event listener
  if (window_count_listener_) {
    window_count_listener_->Stop();
  }

  if (flutter_controller_) {
    flutter_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

LRESULT
FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) noexcept {
  // Give Flutter, including plugins, an opportunity to handle window messages.
  if (flutter_controller_) {
    std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam,
                                                      lparam);
    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      flutter_controller_->engine()->ReloadSystemFonts();
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}
