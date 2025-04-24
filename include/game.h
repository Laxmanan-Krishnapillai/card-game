#ifndef GAME_H_
#define GAME_H_

#include <stdbool.h>
#include <stddef.h>\n\n// Enum for command states in the game\ntypedef enum {\n    COMMAND_INVALID,\n    COMMAND_LD,\n    COMMAND_SW,\n    COMMAND_SI,\n    COMMAND_SR,\n    COMMAND_SD,\n    COMMAND_QQ,\n    COMMAND_P,\n    COMMAND_Q,\n    COMMAND_MOVE\n} CommandState;


typedef struct Node Node;
typedef struct Card Card;

// Deck structure
typedef struct {
  Node* head;
  size_t size;
} Deck;

// GameState structure to hold all game state
typedef struct {
  Deck deck;
  Deck columns[7];
  Deck foundations[4];
  bool in_play;
} GameState;
/**
 * @brief Creates and initializes a new game state.
 * @param initial_deck The initial deck to use for the game.
 * @return A pointer to the newly created GameState.
 */
GameState* game_create(Deck* initial_deck);
/**
 * @brief Frees all memory allocated for a GameState.
 * @param g The GameState to free.
 */
void game_free(GameState* g);
/**
 * @brief Deals the cards from the deck to the columns in the game state.
 * @param g The GameState to deal to.
 */
void game_deal(GameState* g);
/**
 * @brief Checks if a move is valid in the given game state.
 * @param g The GameState to check the move in.
 * @param from The source of the move.
 * @param to The destination of the move.
 * @return true if the move is valid, false otherwise.
 */
bool game_can_move(const GameState* g, const char* from, const char* to);
/**
 * @brief Performs a move in the given game state.
 */
int game_move(GameState* g, const char* from, const char* to);
void game_flip(GameState* g);
void game_print_state(const GameState* g);

#endif