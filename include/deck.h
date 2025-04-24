#ifndef DECK_H
#define DECK_H

#include <stddef.h>
#include <stdbool.h>
#include "card.h"


// Define a linked-list node
typedef struct Node {
    Card* card;
    struct Node* next;
} Node;

// Define the Deck structure
typedef struct Deck {
    Node* head;
    size_t size;
} Deck;

// Function prototypes

/** 
 * @brief Create and initialize an empty deck.
 *
 * Allocates memory for a new Deck and initializes it as empty.
 * 
 * @return A pointer to the newly created Deck, or NULL on allocation failure.
 */
Deck* deck_create_empty(void);

/** 
 * @brief Free all memory allocated to the deck and its cards.
 *
 * Iterates through the deck, freeing each card and node. Finally, it frees the deck itself.
 * 
 * @param d A pointer to the deck to be freed.
 */
void deck_free(Deck* d);

/**
 * @brief Load a deck from file.
 * 
 * @param d pointer to the Deck
 * @param filename name of the file
 * @return 0 if successful, -1 if not.
 */
int deck_load_from_file(Deck* d, const char* filename);

/**
 * @brief Saves a deck to a file.
 * 
 * @param d the deck
 * @param filename the name of the file
 * @return 0 if successful, -1 if not.
 */
int deck_save_to_file(const Deck* d, const char* filename);
void deck_shuffle_interleave(Deck* d, size_t split);
void deck_shuffle_random(Deck* d);
void deck_print(const Deck* d);
#endif