// simple_test.c - Minimal libdecor test
#define RGFW_IMPLEMENTATION
#define RGFW_WAYLAND
#define RGFW_LIBDECOR
#include "../../RGFW.h"

#include <stdio.h>

int main(void) {
    printf("Creating window...\n");
    fflush(stdout);
    
    RGFW_window* win = RGFW_createWindow("Test", 100, 100, 800, 600, 0);
    
    if (!win) {
        printf("ERROR: Failed to create window\n");
        return 1;
    }
    
    printf("Window created! Address: %p\n", (void*)win);
    printf("xdg_surface: %p\n", (void*)win->src.xdg_surface);
    printf("xdg_toplevel: %p\n", (void*)win->src.xdg_toplevel);
    fflush(stdout);
    
    printf("Sleeping for 3 seconds...\n");
    sleep(3);
    
    printf("Closing window...\n");
    RGFW_window_close(win);
    printf("Done\n");
    
    return 0;
}
