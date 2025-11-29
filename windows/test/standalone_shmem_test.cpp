// Standalone Cross-Process Shared Memory Test
// Compile with: cl standalone_shmem_test.cpp /EHsc
// Run two instances and verify shared memory works

#include <windows.h>
#include <iostream>
#include <cstdlib>

const char* kSharedMemoryName = "Local\\FlutterMultiWindowCounter";
const DWORD kSharedMemorySize = 16;
const DWORD kMagicMarker = 0xDEADBEEF;

struct SharedMemoryData {
  volatile LONG window_count;
  DWORD reserved[3];
};

int main(int argc, char* argv[]) {
    bool waitMode = (argc > 1 && std::string(argv[1]) == "--wait");
    bool checkMode = (argc > 1 && std::string(argv[1]) == "--check");

    std::cout << "=== Standalone Shared Memory Cross-Process Test ===" << std::endl;
    std::cout << "PID: " << GetCurrentProcessId() << std::endl;
    if (waitMode) std::cout << "Mode: WAIT (first instance, will hold memory)" << std::endl;
    if (checkMode) std::cout << "Mode: CHECK (verify sharing and exit)" << std::endl;
    std::cout << std::endl;

    // Create/open shared memory
    std::cout << "[1] Calling CreateFileMappingA('" << kSharedMemoryName << "')" << std::endl;

    HANDLE hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        kSharedMemorySize,
        kSharedMemoryName);

    DWORD lastError = GetLastError();
    bool alreadyExists = (lastError == ERROR_ALREADY_EXISTS);

    std::cout << "    Handle: " << hMap << std::endl;
    std::cout << "    GetLastError(): " << lastError << std::endl;
    std::cout << "    ERROR_ALREADY_EXISTS: " << ERROR_ALREADY_EXISTS << std::endl;
    std::cout << "    Already exists: " << (alreadyExists ? "YES" : "NO") << std::endl;
    std::cout << std::endl;

    if (hMap == nullptr) {
        std::cerr << "FATAL: CreateFileMappingA failed!" << std::endl;
        return 1;
    }

    // Map view
    std::cout << "[2] Calling MapViewOfFile()" << std::endl;
    SharedMemoryData* data = static_cast<SharedMemoryData*>(
        MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, kSharedMemorySize));

    if (data == nullptr) {
        std::cerr << "FATAL: MapViewOfFile failed! Error: " << GetLastError() << std::endl;
        CloseHandle(hMap);
        return 1;
    }
    std::cout << "    Mapped address: " << data << std::endl;
    std::cout << std::endl;

    if (!alreadyExists) {
        // First instance - initialize
        std::cout << "[3] FIRST INSTANCE - Initializing shared memory" << std::endl;
        data->window_count = 0;
        data->reserved[0] = kMagicMarker;
        std::cout << "    Set window_count = 0" << std::endl;
        std::cout << "    Set magic marker = 0x" << std::hex << kMagicMarker << std::dec << std::endl;
    } else {
        // Second+ instance - verify sharing
        std::cout << "[3] SECOND+ INSTANCE - Verifying shared memory" << std::endl;
        std::cout << "    Read window_count = " << data->window_count << std::endl;
        std::cout << "    Read magic marker = 0x" << std::hex << data->reserved[0] << std::dec << std::endl;

        if (data->reserved[0] == kMagicMarker) {
            std::cout << std::endl;
            std::cout << "*** SUCCESS: Magic marker matches! Memory IS shared across processes! ***" << std::endl;
        } else {
            std::cout << std::endl;
            std::cout << "*** FAILURE: Magic marker mismatch! Memory is NOT shared! ***" << std::endl;
            std::cout << "    Expected: 0x" << std::hex << kMagicMarker << std::dec << std::endl;
            std::cout << "    Got:      0x" << std::hex << data->reserved[0] << std::dec << std::endl;
        }
    }

    // Increment window count
    std::cout << std::endl;
    std::cout << "[4] Incrementing window count atomically" << std::endl;
    LONG newCount = InterlockedIncrement(&data->window_count);
    std::cout << "    New window_count = " << newCount << std::endl;

    std::cout << std::endl;

    if (waitMode) {
        std::cout << "Waiting 5 seconds (run --check in another terminal)..." << std::endl;
        Sleep(5000);
    } else if (checkMode) {
        // Just exit immediately after checking
        std::cout << "Check complete. Exiting." << std::endl;
    } else {
        std::cout << "Press Enter to exit (keeping memory mapped)..." << std::endl;
        std::cin.get();
    }

    // Decrement and cleanup
    std::cout << "[5] Decrementing window count and cleaning up" << std::endl;
    LONG finalCount = InterlockedDecrement(&data->window_count);
    std::cout << "    Final window_count = " << finalCount << std::endl;

    UnmapViewOfFile(data);
    CloseHandle(hMap);

    std::cout << "Done." << std::endl;
    return 0;
}
