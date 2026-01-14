// libdecor_test.c - Test libdecor client-side decorations
// This test creates a window with libdecor decorations enabled
// and displays it for visual inspection
//
// Build with:
//   gcc -o libdecor_test libdecor_test.c \
//       -I../../ -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon -lEGL -lGL -ldecor-0 \
//       ../../xdg-shell.c ../../xdg-decoration-unstable-v1.c ../../relative-pointer-unstable-v1.c \
//       ../../pointer-constraints-unstable-v1.c ../../xdg-output-unstable-v1.c \
//       ../../xdg-toplevel-icon-v1.c ../../viewporter.c ../../fractional-scale-v1.c

#define RGFW_IMPLEMENTATION
#define RGFW_WAYLAND
#define RGFW_LIBDECOR
#define RGFW_DEBUG
#include "../../RGFW.h"

#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("=== RGFW libdecor Decoration Test ===\n\n");
    printf("This test will display a window for 10 seconds.\n");
    printf("You should see:\n");
    printf("  - Title bar with 'libdecor Test Window' text\n");
    printf("  - Close/Minimize/Maximize buttons\n");
    printf("  - Resizable borders\n\n");
    
    printf("Creating window...\n");
    
    // Create window - RGFW will fall back to libdecor since GNOME doesn't support xdg-decoration
    RGFW_window* win = RGFW_createWindow("libdecor Test Window", 100, 100, 800, 600, 0);
    
    if (!win) {
        fprintf(stderr, "ERROR: Failed to create window\n");
        return 1;
    }
    
    printf("Window created successfully!\n");
    printf("Running for 10 seconds... Please check if decorations are visible.\n\n");
    
    RGFW_event event;
    int frames = 0;
    int resize_count = 0;
    
    for (int i = 0; i < 600; i++) { // 10 seconds at ~60 FPS
        while (RGFW_window_checkEvent(win, &event)) {
            if (event.type == RGFW_windowResized) {
                printf("Window resized to: %dx%d\n", win->w, win->h);
                resize_count++;
            } else if (event.type == RGFW_quit) {
                printf("Close button clicked!\n");
                goto cleanup;
            }
        }
        
        frames++;
        
        // Test programmatic resize at 3 seconds
        if (frames == 180) {
            printf("\nTesting programmatic resize to 1000x700...\n");
            RGFW_window_resize(win, 1000, 700);
        }
        
        usleep(16666); // ~60 FPS
    }
    
cleanup:
    printf("\n--- Test Summary ---\n");
    printf("Total frames: %d\n", frames);
    printf("Resize events received: %d\n", resize_count);
    printf("\nDid you see decorations with title bar and buttons? (You should have!)\n");
    printf("Did programmatic resize work? (Window should have grown at ~3 seconds)\n\n");
    
    RGFW_window_close(win);
    return 0;
}
