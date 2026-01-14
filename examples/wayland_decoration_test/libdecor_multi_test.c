// libdecor_multi_test.c - Test multiple window create/destroy cycles
// This tests that libdecor/GTK state doesn't corrupt between windows

#define RGFW_IMPLEMENTATION
#define RGFW_WAYLAND
#define RGFW_LIBDECOR
#define RGFW_DEBUG
#include "../../RGFW.h"

#include <stdio.h>
#include <unistd.h>

int test_window(int window_num) {
    char title[64];
    snprintf(title, sizeof(title), "Test Window %d", window_num);
    
    printf("\n=== Creating window %d ===\n", window_num);
    
    RGFW_window* win = RGFW_createWindow(title, 100 + (window_num * 50), 100 + (window_num * 50), 640, 480, 0);
    
    if (!win) {
        fprintf(stderr, "ERROR: Failed to create window %d\n", window_num);
        return 1;
    }
    
    printf("Window %d created successfully!\n", window_num);
    printf("Running for 2 seconds...\n");
    
    RGFW_event event;
    
    // Run for 2 seconds (~120 frames at 60 FPS)
    for (int i = 0; i < 120; i++) {
        while (RGFW_window_checkEvent(win, &event)) {
            if (event.type == RGFW_quit) {
                printf("Window %d: Close button clicked!\n", window_num);
                goto cleanup;
            }
        }
        usleep(16666); // ~60 FPS
    }
    
cleanup:
    printf("Closing window %d...\n", window_num);
    RGFW_window_close(win);
    printf("Window %d closed.\n", window_num);
    
    return 0;
}

int main(void) {
    printf("=== RGFW Multi-Window Test ===\n");
    printf("This test creates and destroys 3 windows sequentially.\n");
    printf("It validates that libdecor/GTK state doesn't corrupt between windows.\n\n");
    
    for (int i = 1; i <= 3; i++) {
        if (test_window(i) != 0) {
            fprintf(stderr, "Test failed at window %d\n", i);
            return 1;
        }
        
        // Small delay between windows
        printf("\nWaiting 500ms before next window...\n");
        usleep(500000);
    }
    
    printf("\n=== All 3 windows created and destroyed successfully! ===\n");
    return 0;
}
