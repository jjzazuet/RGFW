# HiDPI Scale Detection Test Results

## Test Program
`libdecor_multi_test` - Tests HiDPI scale reporting on Wayland

## How to Run

### From GNOME Terminal (where GTK decorations work):
```bash
cd /home/jjzazuet/code/RGFW/examples/wayland_decoration_test
./test_hidpi.sh
```

Or directly:
```bash
./libdecor_multi_test
```

## Expected Output on HiDPI Display (3x scaling)

```
=== RGFW Multi-Window Test ===
This test creates and destroys 3 windows sequentially.
It validates that libdecor/GTK state doesn't corrupt between windows.


=== Creating window 1 ===
Window 1 created successfully!
Initial scale: 3.00x x 3.00x
Running for 2 seconds...
Closing window 1...
Final scale: 3.00x x 3.00x
Window 1 closed.

... (repeat for windows 2 and 3)
```

## What the Scale Callback Shows

When the scale update callback fires, you'll see:
```
🔍 HiDPI Scale Update: 3.00x (width) x 3.00x (height)
   Window size: 640x480 logical pixels
   Expected buffer size: 1920x1440 physical pixels
```

This means:
- **Logical size**: What the application sees (640x480)
- **Physical size**: What the GPU should render (1920x1440)
- **Scale factor**: 3.0x (because 1920/640 = 3.0)

## Testing Scale Changes

To test dynamic scale updates:

1. **Multi-monitor setup** with different DPI:
   - Create a window on your HiDPI display (3x scale)
   - Drag it to a normal DPI display (1x scale)
   - You should see: `🔍 HiDPI Scale Update: 1.00x (width) x 1.00x (height)`

2. **GNOME display settings**:
   - Change display scaling in Settings → Displays
   - Existing windows should receive scale update callbacks

## What This Fixes

### Before (without wp_fractional_scale_v1):
- `win->scaleX` and `win->scaleY` always return **1.0**
- Applications must manually query `wl_output` scale (integer only)
- Vulkan swapchains created with wrong size → blurry rendering
- Example: 640x480 logical window on 3x display renders 640x480 pixels (stretched to 1920x1440)

### After (with wp_fractional_scale_v1):
- `win->scaleX` and `win->scaleY` return **actual scale** (e.g., 3.0)
- Fractional scales supported (e.g., 1.25x, 1.5x, 2.0x, 3.0x)
- Applications can create correctly sized swapchains
- Example: 640x480 logical window on 3x display renders 1920x1440 pixels (sharp!)

## Integration with Vulkan Swapchain

```c
RGFW_window* win = RGFW_createWindow("Test", 0, 0, 640, 480, 0);

// Calculate physical dimensions for swapchain
uint32_t swapchain_width = (uint32_t)(win->w * win->scaleX);   // 1920
uint32_t swapchain_height = (uint32_t)(win->h * win->scaleY);  // 1440

// Use wp_viewport to decouple buffer size from surface size
if (win->src.viewport) {
    wp_viewport_set_destination(win->src.viewport, win->w, win->h);
}

// Create Vulkan swapchain with physical dimensions
VkSwapchainCreateInfoKHR swapchain_info = {
    .imageExtent = {swapchain_width, swapchain_height},
    // ...
};
```

The `wp_viewport` protocol tells the compositor:
- "I'm giving you a 1920x1440 buffer"
- "Please display it as a 640x480 surface"
- Compositor handles the scaling perfectly

## Troubleshooting

### Scale always shows 1.00x:
- Compositor doesn't support `wp_fractional_scale_v1` protocol
- Check: `wayland-info | grep fractional_scale`
- GNOME 44+ and KDE Plasma 5.27+ support it

### No scale update callbacks:
- Normal if you don't move windows between displays
- The callback only fires when scale actually changes

### Compilation errors about protocols:
- Make sure protocol headers are generated:
  ```bash
  ls -la fractional-scale-v1.h viewporter.h
  ```
- Should see both files in the test directory

## Next Steps

Once this C test works correctly:
1. Rebuild Java native library with updated RGFW
2. Update Java FFI bindings to expose `scaleX`/`scaleY`
3. Modify `UlRgfwContext` to handle `onScaleUpdated` callback
4. Update `RzSwapchainTest` to use HiDPI-aware swapchain creation
5. All Java Vulkan demos should render sharply on HiDPI displays!
