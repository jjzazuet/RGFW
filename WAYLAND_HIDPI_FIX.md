# RGFW Wayland HiDPI Scale Reporting Fix

## Problem
RGFW on Wayland does not report or track HiDPI scale factors. This causes:
- Swapchain/framebuffer to be created at logical size instead of physical pixel size
- Blurry rendering on HiDPI displays
- `RGFW_scaleUpdatedCallback` is never called on Wayland
- Applications cannot correctly size their framebuffers

## Root Cause
1. `RGFW_window` structure has no `scaleX` or `scaleY` fields (unlike Windows/macOS)
2. Wayland fractional-scale-v1 protocol is not implemented
3. No output scale listener is registered
4. Window creation doesn't query initial scale from compositor

## Solution Overview
Need to implement proper Wayland scale detection using two protocols:

### 1. Preferred Buffer Scale (wp_viewporter)
- Part of core wayland-protocols
- Provides integer scales (1x, 2x, 3x, etc.)
- Compositor tells client preferred scale via `wl_surface` events
- Simple but only supports integer scales

### 2. Fractional Scale v1 (wp_fractional_scale_manager_v1)
- Part of wayland-protocols staging
- Provides precise fractional scales (1.25x, 1.5x, 2.0x, etc.)
- More accurate for modern HiDPI displays
- Available in recent compositors (Mutter 43+, wlroots 0.16+)

## Implementation Plan

### Step 1: Add scale fields to RGFW_window
```c
struct RGFW_window {
    RGFW_window_src src;
    RGFW_windowInternal internal;
    void* userPtr;
    i32 x, y, w, h;
    float scaleX, scaleY;  // ADD THESE
};
```

### Step 2: Add fractional-scale-v1 protocol
Need to generate protocol files:
```bash
wayland-scanner client-header \
  /usr/share/wayland-protocols/staging/fractional-scale/fractional-scale-v1.xml \
  fractional-scale-v1.h
  
wayland-scanner private-code \
  /usr/share/wayland-protocols/staging/fractional-scale/fractional-scale-v1.xml \
  fractional-scale-v1.c
```

### Step 3: Track fractional scale manager in RGFW_info
```c
struct RGFW_info {
    // ... existing fields ...
    #ifdef RGFW_WAYLAND
    struct wp_fractional_scale_manager_v1* fractional_scale_manager;
    #endif
};
```

### Step 4: Add scale object to RGFW_window_src
```c
#ifdef RGFW_WAYLAND
typedef struct RGFW_window_src {
    // ... existing fields ...
    struct wp_fractional_scale_v1* fractional_scale;
    float pending_scale;  // Scale from fractional_scale events
};
#endif
```

### Step 5: Implement scale listener
```c
static void fractional_scale_preferred_scale(
    void* data,
    struct wp_fractional_scale_v1* fractional_scale,
    uint32_t scale_fixed_point)
{
    RGFW_window* win = (RGFW_window*)data;
    // Convert from fixed-point (scale * 120) to float
    float scale = scale_fixed_point / 120.0f;
    
    if (win->scaleX != scale || win->scaleY != scale) {
        win->scaleX = win->scaleY = scale;
        RGFW_scaleUpdatedCallback(win, scale, scale);
    }
}

static const struct wp_fractional_scale_v1_listener fractional_scale_listener = {
    .preferred_scale = fractional_scale_preferred_scale,
};
```

### Step 6: Register scale object during window creation
```c
// In RGFW_createWindowPlatform, after wl_surface creation:
if (_RGFW->fractional_scale_manager) {
    win->src.fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(
        _RGFW->fractional_scale_manager,
        win->src.surface
    );
    if (win->src.fractional_scale) {
        wp_fractional_scale_v1_add_listener(
            win->src.fractional_scale,
            &fractional_scale_listener,
            win
        );
    }
}
```

### Step 7: Initialize scale fields
```c
// Set default scale to 1.0
win->scaleX = 1.0f;
win->scaleY = 1.0f;
```

### Step 8: Clean up on window close
```c
if (win->src.fractional_scale) {
    wp_fractional_scale_v1_destroy(win->src.fractional_scale);
    win->src.fractional_scale = NULL;
}
```

### Step 9: Register fractional scale manager in registry listener
```c
// In registry_global handler:
else if (strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
    _RGFW->fractional_scale_manager = wl_registry_bind(
        registry,
        name,
        &wp_fractional_scale_manager_v1_interface,
        1
    );
}
```

## Testing
1. Create window on HiDPI display (3x scale)
2. Verify `win->scaleX` and `win->scaleY` are set to 3.0
3. Verify `RGFW_scaleUpdatedCallback` is called with (3.0, 3.0)
4. Move window between displays with different scales
5. Verify callback fires on scale changes

## Benefits
- Applications can create properly sized swapchains (width * scaleX, height * scaleY)
- No more blurry rendering on HiDPI displays
- Compatible with GLFW's approach (`glfwGetWindowContentScale`)
- Works on all modern Wayland compositors
