# Tig v1.2.3
Objective:
> Polish Hot reloading and make it bullet proof and stable [DONE]

# Hot Reloading Architecture

To make hot reloading completely bulletproof, stable, and capable of handling changes to structures, enums, and functions on-the-fly (even during infinite loops), we split the compiled code cleanly between the Host Executable and the Hot Library (DLL/SO).

## The Split Strategy

1. **Host Executable (The Driver)**:
   - Contains the `main` function (the entry point and main loop).
   - Declares function pointer variables for all other functions.
   - Generates reload stubs for all other functions.
   - Keeps track of the current DLL version and performs the `reload_check()` on every stub call.

2. **Hot Library (The Engine)**:
   - Contains all struct (`strun`) and `enum` layout definitions.
   - Contains all function bodies **except** `main`.
   - Exports all functions using `__declspec(dllexport)` on Windows.

## Why this is Bulletproof

- **No Trapped Loops**: Because `main` runs on the host, the infinite loop is outside the DLL. When `main` calls any function (e.g. `do_work()`), it crosses the host stub boundary, triggering a `reload_check()`.
- **Dynamic Updates**: If any function, `strun`, or `enum` is updated, recompiling the DLL creates a new DLL version. The very next call from `main` to any stub unloads the old DLL, loads the new DLL, and calls the updated code.
- **Consistent Layouts**: Because all code (except `main`) resides inside the DLL, any changed `strun` or `enum` layout is completely consistent across all DLL functions.

# New TODOs

## 1. Export all DLL functions
Ensure all functions emitted into the DLL are marked with `__declspec(dllexport)` on Windows so that the host can find them using `GetProcAddress`.

## 2. Compile `main` exclusively in the Host
In `emit_hot_split()`, do not emit `main` into the DLL. Instead, emit the full body of `main` directly in the host executable.

## 3. Host stubs for all other functions
Generate stubs and function pointers in the host for all functions except `main`.

## 4. Automatic Cleanup of Old Versions
When a new DLL version is loaded, clean up/delete the old version files to keep the workspace clean.
Hot reloading is meant for development time, not production use.


