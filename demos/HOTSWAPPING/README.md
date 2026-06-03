# Global Hot Reload (v1.2.3)

## Overview

This is the documentation for Tight-C's global, bulletproof hot reload functionality. Hot reload allows you to modify functions, structures (`strun`), and enum layouts and recompile them on-the-fly without restarting the main running application.

## How It Works

### Architecture

The hot reload system cleanly splits your application into two parts:

1. **Host Executable** (e.g. `hot_app.exe` on Windows)
   - Contains the `main` driver function and loop compiled directly inside the Host.
   - Declares function pointer variables for all other functions.
   - Generates reload stubs for all other functions.
   - Manages version tracking, loading/unloading DLLs/SOs, and automatic file cleanup.

2. **Hot Library** (e.g. `hotlib_N.dll` on Windows / `hotlib_N.so` on Unix)
   - Contains all function bodies, structs (`strun`), and enums (except `main`).
   - Every function is dynamically exported (with `__declspec(dllexport)` on Windows).
   - Recompiled versions generate uniquely-named files to prevent file locking:
     ```
     hotlib_1.dll  (initial version)
     hotlib_2.dll  (first rebuild)
     hotlib_3.dll  (second rebuild)
     ```

### Components

- **Version File** (`hotlib.version`): A simple text file containing the current version number, incremented on each rebuild.
- **Host Loader** (`load_hot_functions()`): Monitors the version file. When a new version is detected, it unloads the old library, loads the new one, resolves all exported function stubs, and deletes the old library files automatically.

## Usage

### 1. Initial Compilation

```bash
tigc samples/hot.tc -c hot_app -H hotlib
```

This creates:
- `hotlib_1.dll` (version 1 of the hot library)
- `hotlib.version` (containing "1")
- `hot_app.exe` (the host executable)

### 2. Running the Application

```bash
./hot_app
```

The application runs in a loop on the Host, printing results every 2 seconds.

### 3. Hot Reloading Live

While the application is running, modify the source code of any function (e.g., in `samples/hot.tc`, change `ret x + y` to `ret x + y + 10`), then run:

```bash
tigc samples/hot.tc -H hotlib --hot
```

This:
- Recompiles only the hot library as `hotlib_2.dll`
- Updates `hotlib.version` to "2"

On the very next iteration of the loop in `main`, the Host crosses the stub boundaries, detects the version update, dynamically unloads `hotlib_1.dll`, loads `hotlib_2.dll`, links the new stubs, and cleanly deletes `hotlib_1.dll` from the disk!

## Implementation Details

### Host Stub Code Example (`__tcc_host.c`)

```c
#include <windows.h>

// Hot reload function pointer table
int32_t (*hot_add)(int32_t, int32_t);

static HMODULE hot_lib_handle = NULL;
static int current_version = 0;

static void load_hot_functions(int version) {
    if (version == current_version) return;
    int old_version = current_version;
    current_version = version;
    if (hot_lib_handle) FreeLibrary(hot_lib_handle);
    
    char dll_path[256];
    snprintf(dll_path, sizeof(dll_path), ".\\hotlib_%d.dll", version);
    hot_lib_handle = LoadLibraryA(dll_path);
    
    // Clean up old DLL files automatically
    if (old_version > 0) {
        char old_path[256];
        snprintf(old_path, sizeof(old_path), "hotlib_%d.dll", old_version);
        remove(old_path);
    }
    
    // Resolve stubs
    hot_add = (void*)GetProcAddress(hot_lib_handle, "add");
}

static void reload_check(void) {
    int new_ver = get_version();
    if (new_ver != current_version) load_hot_functions(new_ver);
}

// Function stub
int32_t add(int32_t x, int32_t y) {
    reload_check();
    return hot_add(x, y);
}

// Host-driven main driver
int32_t main(void) {
    load_hot_functions(get_version());
    // Loops and execution run natively on the host
    ...
}
```

## CLI Flags

- `-H <libname>`: Enable hot reload mode, specify the shared library name
- `--hot`: Rebuild only the hot library (for running applications)
- `-t, --temp`: Keep temporary `.c` files for debugging

## Advantages of v1.2.3 Architecture

- **No Obsolete Keywords**: No special `hot` keyword needed. Your code is completely clean and identical to production code!
- **Dynamic Structural Changes**: Since structs (`strun`) and enums reside in the DLL, layout updates are fully consistent across all hot functions.
- **Zero Locked Files**: Using versioned libraries avoids all file-locking issues.
- **Workspace Cleanup**: Old DLL/SO files are automatically removed upon reloading, keeping the directory clean.

