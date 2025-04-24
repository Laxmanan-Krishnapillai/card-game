#include "ui_gui.h"
#include <stdio.h>

static SDL_Window*    window   = NULL;
static SDL_Renderer*  renderer = NULL;

int gui_init(void) {
    // TODO: initialize SDL, create window and renderer
    return 0;
}

static void gui_render_board(const GameState* state) {
    // TODO: draw cards, columns, foundations using SDL textures
}

static void gui_handle_event(GameState* state, SDL_Event* e, int* quit) {
    // TODO: handle mouse/keyboard input, translate to game_move or quit
}

void gui_main_loop(GameState* state) {
    int quit = 0;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else {
                gui_handle_event(state, &e, &quit);
            }
        }
        SDL_RenderClear(renderer);
        gui_render_board(state);
        SDL_RenderPresent(renderer);
        // TODO: cap frame-rate if desired
    }
}

void gui_destroy(void) {
    // TODO: destroy renderer, window and quit SDL
}