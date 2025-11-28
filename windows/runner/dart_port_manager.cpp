// dart_port_manager.cpp
//
// Implementation of DartPortManager for C++ to Dart communication.

#include "dart_port_manager.h"

#include <iostream>

DartPortManager::DartPortManager() {
  // Constructor initializes members to safe defaults
  // Actual port registration happens via RegisterPort()
  // TODO: In GREEN phase, implement port registration and notification
}

DartPortManager::~DartPortManager() {
  // Destructor - ports are automatically cleaned up when vector is destroyed
  // No explicit cleanup needed for Dart_Port (they're just int64_t values)
}

bool DartPortManager::RegisterPort(Dart_Port port) {
  // TODO: In GREEN phase:
  // 1. Acquire mutex lock (std::lock_guard)
  // 2. Add port to ports_ vector
  // 3. Log registration
  // 4. Return true
  return false;
}

bool DartPortManager::UnregisterPort(Dart_Port port) {
  // TODO: In GREEN phase:
  // 1. Acquire mutex lock (std::lock_guard)
  // 2. Find port in ports_ vector (std::find)
  // 3. Remove port if found (std::remove_if + erase)
  // 4. Log unregistration
  // 5. Return true if found, false otherwise
  return false;
}

void DartPortManager::NotifyWindowCountChanged(LONG new_count) {
  // TODO: In GREEN phase:
  // 1. Acquire mutex lock (std::lock_guard)
  // 2. Create Dart_CObject message with new_count
  // 3. Loop through ports_ vector
  // 4. Call Dart_PostCObject for each port
  // 5. Log successful posts and errors
}

// FFI Exports

// Global DartPortManager instance
// Alternative: Could be owned by FlutterWindow and passed via context
static DartPortManager g_dart_port_manager;

extern "C" {

__declspec(dllexport) bool RegisterWindowCountPort(Dart_Port port) {
  return g_dart_port_manager.RegisterPort(port);
}

__declspec(dllexport) bool UnregisterWindowCountPort(Dart_Port port) {
  return g_dart_port_manager.UnregisterPort(port);
}

}  // extern "C"
