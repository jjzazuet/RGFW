// wayland_decoration_test.c - Diagnostic test for RGFW Wayland decorations
// This test prints what decoration mode the compositor returns and displays
// a window for visual inspection to determine if server-side decorations work.
//
// Build with:
//   gcc -o decoration_test wayland_decoration_test.c \
//       -I../../ -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon -lEGL -lGL

#define RGFW_IMPLEMENTATION
#define RGFW_WAYLAND
#define RGFW_DEBUG
#include "../../RGFW.h"

#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("=== RGFW Wayland Decoration Diagnostic Test ===\n\n");
    printf("Creating window with default flags (decorations enabled)...\n");
    
    RGFW_window* win = RGFW_createWindow("Decoration Test", 100, 100, 800, 600, 0);
    
    if (!win) {
        fprintf(stderr, "ERROR: Failed to create window\n");
        return 1;
    }
    
    printf("\n--- Decoration Mode Analysis ---\n");
    printf("Decoration mode received from compositor: %u\n", win->src.decoration_mode);
    printf("  1 = CLIENT_SIDE (compositor expects app to draw decorations)\n");
    printf("  2 = SERVER_SIDE (compositor will draw decorations)\n");
    printf("  0 = NOT YET CONFIGURED (might update after first configure event)\n\n");
    
    if (win->src.decoration_mode == 0) {
        printf("NOTE: Mode is 0 - waiting for configure event...\n");
    } else if (win->src.decoration_mode == 1) {
        printf("DIAGNOSIS: Compositor returned CLIENT_SIDE\n");
        printf("  -> libdecor fallback should be active but may be broken\n");
        printf("  -> Check RGFW.h lines 8398-8421 (libdecor callbacks)\n");
    } else if (win->src.decoration_mode == 2) {
        printf("DIAGNOSIS: Compositor returned SERVER_SIDE\n");
        printf("  -> If you DON'T see decorations, there's a timing/commit issue\n");
        printf("  -> Server-side decorations should be visible\n");
    }
    
    printf("\n--- Visual Inspection ---\n");
    printf("Window will display for 5 seconds.\n");
    printf("LOOK FOR:\n");
    printf("  - Title bar with 'Decoration Test' text\n");
    printf("  - Close/Minimize/Maximize buttons\n");
    printf("  - Resizable borders\n\n");
    
    printf("If you see NO decorations, take note and report:\n");
    printf("  1. Decoration mode from above: %u\n", win->src.decoration_mode);
    printf("  2. Your compositor: GNOME/KDE/Sway/etc.\n");
    printf("  3. Whether window is visible at all\n\n");
    
    printf("Running event loop for 5 seconds...\n");
    
    int frame_count = 0;
    int mode_updated = 0;
    RGFW_event event;
    
    for (int i = 0; i < 300; i++) {
        while (RGFW_window_checkEvent(win, &event)) {
            // Process events
        }
        
        // Check if mode was updated after initial creation
        if (!mode_updated && win->src.decoration_mode != 0 && frame_count > 10) {
            printf("\nUPDATE: Decoration mode is now: %u\n", win->src.decoration_mode);
            mode_updated = 1;
        }
        
        frame_count++;
        usleep(16666); // ~60 FPS
    }
    
    printf("\n--- Test Complete ---\n");
    printf("Final decoration mode: %u\n", win->src.decoration_mode);
    printf("Did you see decorations? (You should have!)\n\n");
    
    RGFW_window_close(win);
    return 0;
}
