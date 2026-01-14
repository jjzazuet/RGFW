# RGFW Wayland Decoration Diagnostic Results

## Test Environment
- Compositor: GNOME/Mutter on Wayland
- RGFW Version: 1.8.1
- Test: `/home/jjzazuet/code/RGFW/examples/wayland_decoration_test/decoration_test`

## Findings

### Root Cause Identified
**GNOME/Mutter does NOT advertise the `zxdg_decoration_manager_v1` protocol**

From Wayland protocol debug output, the compositor advertises these globals:
```
wl_compositor, wl_shm, zxdg_output_manager_v1, wl_data_device_manager,
xdg_wm_base, zwp_relative_pointer_manager_v1, zwp_pointer_constraints_v1,
etc...
```

**MISSING**: `zxdg_decoration_manager_v1` (the xdg-decoration protocol for server-side decorations)

### What This Means
1. GNOME/Mutter doesn't support the xdg-decoration protocol
2. Applications MUST use client-side decorations (CSD) on GNOME
3. RGFW correctly detects this and falls back to libdecor (lines 8755-8780 in RGFW.h)
4. **BUT**: RGFW's libdecor fallback is broken - the callbacks are stubbed as `{0}` (NULL)

### The Bug in RGFW.h (lines 8762-8767)
```c
static struct libdecor_frame_interface frameInterface = {0}; /*= {
    RGFW_wl_handle_configure,
    RGFW_wl_handle_close,
    RGFW_wl_handle_commit,
    RGFW_wl_handle_dismiss_popup,
};*/
```

The callbacks are commented out, causing:
- No window configuration (size/state changes don't work)
- No close handling
- No frame rendering
- **Result**: Windows appear as empty space with no decorations

## Required Fix
Implement the libdecor callback functions:
1. `RGFW_wl_libdecor_configure` - Handle window resize/state
2. `RGFW_wl_libdecor_close` - Handle close button
3. `RGFW_wl_libdecor_commit` - Commit frame changes

These callbacks will provide proper client-side decorations (title bar, buttons, borders).
