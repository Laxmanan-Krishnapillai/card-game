#include "../include/card.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

Card *card_create(Rank rank, Suit suit) {
    Card *card = (Card *)malloc(sizeof(Card));
    if (card == NULL) {
        return NULL;
    }
    card->rank = rank;
    card->suit = suit;
    card->next = NULL;
    return card;
}

void card_free(Card *card) {
    if (card != NULL) {
        free(card);
    }
}

char *card_to_string(const Card *card) {
    if (card == NULL) {
        return NULL;
    }
    
    if (!card->face_up) {
        char *str = (char *)malloc(sizeof(char) * 4);
        if (str == NULL) {
            return NULL;
        }
        sprintf(str, "[ ]");
        return str;
    }

    char rank_char;
    if (card->rank >= RANK_2 && card->rank <= RANK_10) {
        rank_char = '0' + card->rank ;
        
    }
    else {
        switch (card->rank) {
            case RANK_2 : rank_char = '2'; break;
            case RANK_3 : rank_char = '3'; break;
            case RANK_4 : rank_char = '4'; break;
            case RANK_5 : rank_char = '5'; break;
            case RANK_6 : rank_char = '6'; break;
            case RANK_7 : rank_char = '7'; break;
            case RANK_8 : rank_char = '8'; break;
            case RANK_9 : rank_char = '9'; break;
            case RANK_10 : rank_char = 't'; break;
            case RANK_ACE: rank_char = 'A'; break;
            case RANK_JACK: rank_char = 'J'; break;
            case RANK_QUEEN: rank_char = 'Q'; break;
            case RANK_KING: rank_char = 'K'; break;
        }
    }

    char suit_char;
    switch (card->suit) {
        case SUIT_CLUB: suit_char = 'C'; break;
        case SUIT_DIAMOND: suit_char = 'D'; break;
        case SUIT_HEART: suit_char = 'H'; break;
        case SUIT_SPADE: suit_char = 'S'; break;
    }

    char *str = (char *)malloc(sizeof(char) * 3);
    if (str == NULL) {
        return NULL;
    }
    sprintf(str, "%c%c", rank_char, suit_char);
    return str;
}