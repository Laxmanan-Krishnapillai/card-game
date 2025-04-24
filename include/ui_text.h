#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "game.h"

/**
 * @brief Initializes the startup loop for the text-based UI.
 *        This loop handles commands for loading, shuffling, saving the deck, and starting the game.
 */
void ui_text_startup_loop(void);
/**
 * @brief Initializes the play loop for the text-based UI.
 * @param g The game state.
 */
void ui_text_play_loop(GameState* g);

#endif // UI_TEXT_H
