/**
 * @file main.c
 * @brief Main entry point for the Yukon Solitaire game.
 *
 * This file contains the main function which initializes and starts the
 * user interface for the text-based Yukon Solitaire game.
 */

#include <stdio.h>
#include "deck.h"
#include "game.h"
#include "ui_text.h"

/**
 * @brief The main function.
 * @return 0 on successful execution.
 */
int main() {
    ui_text_startup_loop();
    return 0;
}