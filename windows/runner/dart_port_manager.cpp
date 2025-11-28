// dart_port_manager.cpp
//
// Implementation of DartPortManager for C++ to Dart communication.

#include "dart_port_manager.h"

#include <algorithm>
#include <iostream>

DartPortManager::DartPortManager() {
  // Constructor initializes members to safe defaults
  // Actual port registration happens via RegisterPort()
}

DartPortManager::~DartPortManager() {
  // Destructor - ports are automatically cleaned up when vector is destroyed
  // No explicit cleanup needed for Dart_Port (they're just int64_t values)
}

bool DartPortManager::RegisterPort(Dart_Port_DL port) {
  std::lock_guard<std::mutex> lock(ports_mutex_);

  ports_.push_back(port);
  std::cout << "Dart port registered: " << port << std::endl;

  return true;
}

bool DartPortManager::UnregisterPort(Dart_Port_DL port) {
  std::lock_guard<std::mutex> lock(ports_mutex_);

  auto it = std::find(ports_.begin(), ports_.end(), port);
  if (it != ports_.end()) {
    ports_.erase(it);
    std::cout << "Dart port unregistered: " << port << std::endl;
    return true;
  }

  return false;
}

void DartPortManager::NotifyWindowCountChanged(LONG new_count) {
  std::lock_guard<std::mutex> lock(ports_mutex_);

  if (ports_.empty()) {
    return;  // No ports registered, nothing to notify
  }

  std::cout << "Notifying " << ports_.size() << " Dart port(s) of window count: "
            << new_count << std::endl;

  // Create Dart_CObject message with int64 value
  Dart_CObject message;
  message.type = Dart_CObject_kInt64;
  message.value.as_int64 = static_cast<int64_t>(new_count);

  // Broadcast to all registered ports
  for (Dart_Port_DL port : ports_) {
    bool result = Dart_PostCObject_DL(port, &message);
    if (result) {
      std::cout << "Posted to Dart port: " << port << std::endl;
    } else {
      std::cerr << "Failed to post to Dart port: " << port << std::endl;
    }
  }
}

// FFI Exports

// Global DartPortManager instance
// Alternative: Could be owned by FlutterWindow and passed via context
static DartPortManager g_dart_port_manager;

extern "C" {

__declspec(dllexport) intptr_t InitDartApiDL(void* data) {
  return Dart_InitializeApiDL(data);
}

__declspec(dllexport) bool RegisterWindowCountPort(Dart_Port_DL port) {
  return g_dart_port_manager.RegisterPort(port);
}

__declspec(dllexport) bool UnregisterWindowCountPort(Dart_Port_DL port) {
  return g_dart_port_manager.UnregisterPort(port);
}

}  // extern "C"
