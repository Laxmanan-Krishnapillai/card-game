#ifndef UI_GUI_H
#define UI_GUI_H

#include "game.h"
#include <SDL2/SDL.h>

// Initialize SDL, create window & renderer
// Returns 0 on success, non-zero on failure
int gui_init(void);

// Main GUI loop: render the board, handle events, update GameState
void gui_main_loop(GameState* state);

// Clean up SDL resources
void gui_destroy(void);

#endif

