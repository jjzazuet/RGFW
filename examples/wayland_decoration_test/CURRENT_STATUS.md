# RGFW Wayland Decorations - Current Status

## ✅ MAJOR SUCCESS: Decorations Are Working!

The libdecor integration is now functional! Windows appear with proper client-side decorations (title bar, buttons, borders).

## Current Issue: Crash on Mouse Hover

**Symptom**: Program segfaults when mouse enters the window decoration frame (before clicking any buttons).

**What This Means**: The core integration is correct, but one of the libdecor callbacks is being triggered during mouse events and has a bug.

## Changes Made (Latest)

### 1. Fixed Window Creation Architecture (CRITICAL)
Restructured `RGFW_createWindowPlatform()` to:
- Check for `decoration_manager` BEFORE creating xdg surfaces
- If using libdecor path, let libdecor create xdg_surface/xdg_toplevel
- Extract xdg surfaces from libdecor frame using:
  - `libdecor_frame_get_xdg_surface()`
  - `libdecor_frame_get_xdg_toplevel()`
- Early return when using libdecor to skip manual xdg-shell creation

### 2. Added Safety Checks to libdecor Callbacks
- Null pointer checks for win, frame, configuration
- Fallback to current window size if configuration doesn't specify size
- Null check before calling `libdecor_state_free()`
- Surface validity check in commit callback

### 3. Added decorFrame Field
Added `struct libdecor_frame* decorFrame` to RGFW_window_src to store the frame reference.

## Debugging Setup

Created VSCode launch configuration at `/home/jjzazuet/code/RGFW/.vscode/launch.json`:
- Debugger: GDB
- Program: `libdecor_test`
- Catches SIGSEGV (segmentation faults)
- Built with `-g -O0` for debug symbols

## Next Steps

1. **Use IntelliJ Debugger MCP** to debug the crash:
   - Set breakpoint in `RGFW_wl_libdecor_configure`
   - Run until crash
   - Inspect stack trace and variables

2. **Likely Culprits**:
   - `libdecor_configuration_get_content_size()` might return false but still modify width/height
   - `libdecor_configuration_get_window_state()` might access invalid memory
   - `libdecor_frame_commit()` might need different parameters
   - The `commit` callback might be called with NULL surface

3. **Test Case**: Just hover mouse over window decorations - no clicking needed to reproduce

## What's Confirmed Working

- ✅ Window creation with libdecor
- ✅ Decorations appear (title bar, buttons, borders)
- ✅ xdg_surface and xdg_toplevel correctly populated
- ✅ No protocol errors (`xdg_wm_base::get_xdg_surface already requested` is fixed!)
- ✅ Window displays for full 3 seconds in simple_test
- ✅ Programmatic resize fix (separate, already working)

## Files Modified

- `/home/jjzazuet/code/RGFW/RGFW.h` - All fixes
- `/home/jjzazuet/code/RGFW/.vscode/launch.json` - Debug configuration

You're SO close! The decorations work - we just need to fix one callback bug.
