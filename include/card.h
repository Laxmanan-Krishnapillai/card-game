#ifndef CARD_H
#define CARD_H

// Enumerations for card suit and rank
#include <stdbool.h>

typedef enum {
    SUIT_CLUB, SUIT_DIAMOND, SUIT_HEART, SUIT_SPADE
} Suit;

typedef enum {
    RANK_ACE = 1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_9, RANK_10, RANK_JACK, RANK_QUEEN, RANK_KING
} Rank;

// Card struct definition
typedef struct Card {
    Rank rank;
    Suit suit;
    struct Card *next;
} Card;

// Function prototype
Card *card_create(Rank rank, Suit suit);
void card_free(Card *card);
char* card_to_string(const Card* c);
#endif