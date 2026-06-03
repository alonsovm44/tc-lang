# Hot Reload Proof of Concept

## Overview

This is a proof of concept implementation of hot reload functionality for Tight-C. Hot reload allows you to modify code and recompile shared libraries without restarting the main executable.

## How It Works

### Architecture

The hot reload system uses versioned shared libraries to avoid file locking issues on Windows:

```
hotlib_1.dll  (initial version)
hotlib_2.dll  (first rebuild)
hotlib_3.dll  (second rebuild)
...
hotlib.version (contains current version number)
```

### Components

1. **Host Executable** (`hotfn.exe`)
   ( compile with `./tcc samples/hot.tc -c hotfn`)
   
   - Contains the main function and non-hot code
   - Uses function pointers to call hot functions
   - Monitors the version file for changes
   - Dynamically loads/unloads versioned DLLs

2. **Hot Library** (`hotlib_N.dll`)
   - Contains only the hot functions
   - Compiled as a shared library
   - Each rebuild creates a new version

3. **Version File** (`hotlib.version`)
   - Simple text file containing the current version number
   - Incremented on each rebuild
   - Read by the host to detect new versions

## Usage

### Initial Compilation

```bash
tcc hot.tc -H hotlib -c hotfn
```

This creates:
- `hotlib_1.dll` (version 1 of the hot library)
- `hotlib.version` (contains "1")
- `hotfn.exe` (the host executable)

### Running the Application

```bash
./hotfn.exe
```

The application will run in a loop, printing the result of the hot function every 2 seconds.

### Hot Reloading

While the application is running, modify the source code (e.g., change `ret x + y` to `ret x + y + 10`), then rebuild:

```bash
tcc hot.tc -H hotlib --hot-rebuild
```

This creates:
- `hotlib_2.dll` (version 2 of the hot library)
- Updates `hotlib.version` to "2"

The running application will automatically detect the version change on the next hot function call, unload the old DLL, load the new one, and start using the updated code.

## Implementation Details

### Compiler Changes

1. **Lexer/Parser**: Added `hot` keyword to mark functions for hot reloading
2. **AST**: Added `is_hot` flag to function declarations
3. **Emitter**: Split output into host code and hot library code
4. **Main**: Added `-H <libname>` and `--hot-rebuild` CLI flags

### Generated Code

#### Host Code (`__tcc_host.c`)

```c
#include <windows.h>
static HMODULE hot_lib_handle = NULL;

static void load_hot_functions(void) {
    // Read current version
    int version = 1;
    FILE *vf = fopen("hotlib.version", "r");
    if (vf) {
        fscanf(vf, "%d", &version);
        fclose(vf);
    }
    static int loaded_version = 0;
    if (version == loaded_version) return;
    loaded_version = version;
    
    // Unload old library
    if (hot_lib_handle) FreeLibrary(hot_lib_handle);
    
    // Load new versioned library
    char dll_path[256];
    snprintf(dll_path, sizeof(dll_path), "hotlib_%d.dll", version);
    hot_lib_handle = LoadLibraryA(dll_path);
    
    // Resolve function pointers
    hot_add = (void*)GetProcAddress(hot_lib_handle, "add");
}

static void reload_hot_if_changed(void) {
    load_hot_functions();
}

// Hot function stub
int32_t add(int32_t x, int32_t y) {
    reload_hot_if_changed();
    return hot_add(x, y);
}
```

#### Hot Library (`hotlib.c`)

```c
int32_t add(int32_t x, int32_t y) {
    return (x + y + 10);
}
```

### Windows DLL Locking Solution

Windows locks loaded DLL files, preventing them from being overwritten. The standard solution used by game engines and live coding systems is to use versioned filenames:

- Each rebuild creates a new file (e.g., `hotlib_1.dll`, `hotlib_2.dll`)
- The old DLL remains loaded until the next hot function call
- The host unloads the old DLL and loads the new one
- Old DLL files can be cleaned up later if needed

This approach completely avoids file locking issues and is the industry standard for hot reload on Windows.

## CLI Flags

- `-H <libname>`: Enable hot reload mode, specify the library name
- `--hot-rebuild`: Rebuild only the hot library (for running applications)
- `-t, --temp`: Keep temporary .c files for debugging

## Test Case

The test program (`hot.tc`) demonstrates hot reload:

```tightc
use "stdlib/io.tc"

extern "C" {
    i32 fn Sleep: u32 ms {}
}

hot fn i32 add: i32 x, i32 y {
    ret x + y + 10
}

fn void main: {
    loop {
        i32 result = add(3, 4)
        printi(result)
        Sleep(2000)
    }
}
```

Running this prints `17` every 2 seconds. If you change `ret x + y + 10` to `ret x + y + 20` and rebuild, the output will change to `24` without restarting the application.

## Demo Output

```
C:\Users\diego\.projects\langs\tidy>.\hotfn
7
7
7
17
17
17
^C
```

The output changed from `7` to `17` after rebuilding the hot library while the application was running.

## Limitations

- Only functions marked with `hot` can be hot-reloaded
- Global state in hot libraries is not preserved across reloads
- The version file must be in the same directory as the executable
- Old DLL versions accumulate and are not automatically cleaned up

## Future Improvements

- Automatic cleanup of old DLL versions
- Support for hot-reloading global state
- Better error handling and recovery
- Support for hot-reloading data structures
