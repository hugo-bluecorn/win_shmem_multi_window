# C++ Style Guide Summary - Google vs Flutter Windows Adaptations

**Created:** 2025-11-28

This document summarizes the key differences between the Google C++ Style Guide and our Flutter Windows project adaptations.

---

## Key Differences from Google C++ Style Guide

### 1. Naming Conventions

| Element | Google C++ Guide | Flutter Windows Adaptation |
|---------|------------------|----------------------------|
| **Member variables** | `snake_case_` with trailing underscore | ✅ **Same** - `snake_case_` with trailing underscore |
| **Functions/Methods** | `PascalCase` or `SnakeCase` (both used) | ✅ **Standardized to PascalCase** only (matches existing Flutter Windows code) |
| **Files** | `snake_case.h` / `snake_case.cc` | ✅ **Adapted** - `snake_case.h` / `snake_case.cpp` (using .cpp not .cc) |
| **Constants** | `kPascalCase` | ✅ **Same** - `kPascalCase` with `k` prefix |

**Why:** Flutter Windows runner code consistently uses PascalCase for methods (e.g., `CreateAndShow()`, `GetHandle()`), so we standardized to this pattern for consistency with the existing codebase.

### 2. Namespaces

| Aspect | Google C++ Guide | Flutter Windows Adaptation |
|--------|------------------|----------------------------|
| **Namespace usage** | Encouraged, use nested namespaces | ✅ **Adapted** - Prefer no namespace for runner code |
| **Rationale** | Avoids naming conflicts | Matches existing Flutter Windows runner style |

**Why:** The existing Flutter Windows runner code (`flutter_window.h`, `win32_window.h`, etc.) does not use namespaces. We follow this pattern for consistency.

### 3. File Extensions

| File Type | Google C++ Guide | Flutter Windows Adaptation |
|-----------|------------------|----------------------------|
| **Implementation** | `.cc` preferred | ✅ **Changed to `.cpp`** |
| **Headers** | `.h` | ✅ **Same** - `.h` |

**Why:** Flutter Windows projects use `.cpp` (MSVC standard), not `.cc` (Google/Unix style).

### 4. Include Guards

| Aspect | Google C++ Guide | Flutter Windows Adaptation |
|--------|------------------|----------------------------|
| **Format** | `PROJECT_PATH_FILE_H_` | ✅ **Adapted** - `RUNNER_FILENAME_H_` |
| **Example** | `FOO_BAR_BAZ_H_` | `RUNNER_SHARED_MEMORY_MANAGER_H_` |

**Why:** Flutter Windows runner code uses `RUNNER_` prefix since all native code is in `windows/runner/` directory.

### 5. Windows API Conventions (New Section)

**Added sections not in Google C++ Guide:**
- Handle naming (`_handle_` suffix for Windows handles)
- Error checking with `GetLastError()`
- Wide string conversion for Windows APIs
- Atomic operations (`InterlockedIncrement`/`Decrement`)
- `volatile` keyword for shared memory

**Why:** Google C++ Guide is platform-agnostic. We added Windows-specific patterns essential for this project.

### 6. Smart Pointers vs Windows Handles

| Aspect | Google C++ Guide | Flutter Windows Adaptation |
|--------|------------------|----------------------------|
| **Ownership** | Use `std::unique_ptr` for ownership | ✅ **Adapted** - Use RAII for Windows handles, not smart pointers |
| **Handles** | Not covered | `HANDLE` is not a pointer - use custom RAII wrapper |

**Why:** Windows `HANDLE` types (returned by `CreateFileMapping`, `CreateEvent`, etc.) are opaque types, not pointers. Smart pointers don't work with them. Use RAII pattern with `CloseHandle()` in destructor.

### 7. Line Length

| Aspect | Google C++ Guide | Flutter Windows Adaptation |
|--------|------------------|----------------------------|
| **Max length** | 80 characters (soft limit) | ✅ **Same** - 80 characters (strictly enforced) |

**Why:** Consistent with Google guide and Flutter Dart code conventions.

### 8. Indentation

| Aspect | Google C++ Guide | Flutter Windows Adaptation |
|--------|------------------|----------------------------|
| **Spaces** | 2 spaces | ✅ **Same** - 2 spaces |
| **Tabs** | Never | ✅ **Same** - Never use tabs |

**Why:** Matches both Google guide and existing Flutter Windows code.

### 9. Bracing Style

| Aspect | Google C++ Guide | Flutter Windows Adaptation |
|--------|------------------|----------------------------|
| **Opening brace** | Same line | ✅ **Same** - Same line |
| **Function braces** | Same line preferred | ✅ **Same** - Always same line |

**Why:** Fully compatible with Google style.

---

## New Sections Added (Not in Google C++ Guide)

### 1. Windows API Conventions
Comprehensive guidance on:
- `HANDLE` naming and management
- `GetLastError()` error checking patterns
- String conversion (`std::string` → `std::wstring`)
- Atomic operations (`InterlockedIncrement`, `InterlockedDecrement`)
- `volatile` keyword usage for shared memory

**Rationale:** Essential for Windows shared memory implementation.

### 2. RAII for Windows Resources
Specific patterns for:
- Shared memory handles (`CreateFileMapping`, `MapViewOfFile`)
- Event handles (`CreateEvent`, `SetEvent`)
- Proper cleanup in destructors

**Rationale:** Windows API requires manual resource cleanup, RAII ensures safety.

### 3. Thread Safety Patterns
Guidance on:
- When to use `Interlocked*` functions vs mutexes
- `volatile` for shared memory access
- Event-based synchronization

**Rationale:** Core to the multi-window shared memory architecture.

---

## What We Kept from Google C++ Guide

✅ **Class declaration order** (public → private, constructors → methods → members)
✅ **Member initialization lists** preferred over assignments
✅ **`const` correctness** enforced
✅ **References for non-null parameters**, pointers for optional
✅ **`nullptr` not `NULL`**
✅ **Avoid global mutable state**
✅ **Short functions** (under 40 lines)
✅ **Meaningful names** over comments
✅ **RAII pattern** for resource management

---

## Summary

**Compatibility:** ~90% compatible with Google C++ Style Guide

**Key Adaptations:**
1. **Function naming:** Standardized to `PascalCase` (Flutter Windows pattern)
2. **File extension:** `.cpp` instead of `.cc` (MSVC standard)
3. **Namespace:** Prefer none for runner code (existing pattern)
4. **Include guards:** `RUNNER_` prefix (Flutter Windows pattern)
5. **Windows API:** Added comprehensive Windows-specific guidance

**Philosophy:** Follow Google C++ Style Guide principles while matching existing Flutter Windows codebase conventions for consistency.

---

**Version:** 1.0
**Last Updated:** 2025-11-28
