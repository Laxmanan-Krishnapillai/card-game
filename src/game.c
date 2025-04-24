#include "game.h"
#include "deck.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



// Helper function to get the column index from a column string
static int get_column_index(const char *column_str) {
    if (strlen(column_str) != 2 || column_str[0] != 'C' || column_str[1] < '1' || column_str[1] > '7') {
        return -1;
    }
    return column_str[1] - '1';
}

// Helper function to get the foundation index from a foundation string
static int get_foundation_index(const char *foundation_str) {
    if (strlen(foundation_str) != 2 || foundation_str[0] != 'F' || foundation_str[1] < '1' || foundation_str[1] > '4') {
        return -1;
    }
    return foundation_str[1] - '1';
}

// Helper function to check if a card can be moved onto another card in a column
static bool can_move_onto_column(const Card *src, const Card *dst) {
    if (dst == NULL) return true;
    if (!dst->face_up) return false;
    return src->rank == dst->rank - 1 && (src->suit == SUIT_CLUB || src->suit == SUIT_SPADE) != (dst->suit == SUIT_CLUB || dst->suit == SUIT_SPADE);
}

// Helper function to check if a card can be moved onto a foundation
static bool can_move_onto_foundation(const Card *src, const Card *dst) {
    if (dst == NULL) {
        return src->rank == RANK_ACE;
    }
    return src->rank == dst->rank + 1 && src->suit == dst->suit;
}

GameState *game_create(Deck *initial_deck) {
    GameState *g = (GameState *)malloc(sizeof(GameState));
    if (g == NULL) return NULL;

    g->deck = *initial_deck;

    for (int i = 0; i < 7; i++) {
        g->columns[i].head = NULL;
        g->columns[i].size = 0;
    }
    for (int i = 0; i < 4; i++) {
        g->foundations[i].head = NULL;
        g->foundations[i].size = 0;
    }
    g->in_play = false;
    return g;
}

void game_free(GameState *g) {
    if (g == NULL) return;
    deck_free(&g->deck);

    for (int i = 0; i < 7; i++) {
        deck_free(&g->columns[i]);
    }
    for (int i = 0; i < 4; i++) {
        deck_free(&g->foundations[i]);
    }
    free(g);
}

void game_deal(GameState *g) {
    int column_sizes[] = {1, 6, 7, 8, 7, 6, 5};
    int column_index = 0;
    int num_face_down = 0;
    Node* current = g->deck.head;
    while (current != NULL){
        current->card->face_up = false;
        current = current->next;
    }
    current = g->deck.head;
    
    while(current != NULL){
        Node* temp = current;
        current = current->next;
        temp->next = NULL;
        
        if(column_index == 0 || column_index > num_face_down){
            temp->card->face_up = true;
            num_face_down++;
        }

        if (g->columns[column_index].head == NULL){
            g->columns[column_index].head = temp;
        } else {
            Node* end = g->columns[column_index].head;
            while(end->next != NULL){
                end = end->next;
            }
            end->next = temp;
        }
        g->columns[column_index].size++;
        
        column_index++;
        if(column_index > 6){
            column_index = 0;
            
        }
    }
    g->in_play = true;
}

bool game_can_move(const GameState *g, const char *from, const char *to) {
    int from_col_index = get_column_index(from);
    int to_col_index = get_column_index(to);
    int from_foundation_index = get_foundation_index(from);
    int to_foundation_index = get_foundation_index(to);
    
    Card* src = NULL;
    Card* dst = NULL;

    if(from_col_index != -1){
      if (g->columns[from_col_index].head == NULL) return false;
        Node* current = g->columns[from_col_index].head;
        while (current->next != NULL) {
          current = current->next;
        }
        src = current->card;
    } else if(from_foundation_index != -1) {
        if (g->foundations[from_foundation_index].head == NULL) return false;
        Node* current = g->foundations[from_foundation_index].head;
        while (current->next != NULL) {
          current = current->next;
        }
        src = current->card;
    }else{
      return false;
    }

    if (to_col_index != -1){
        if(g->columns[to_col_index].head == NULL){
           dst = NULL;
        }else{
            Node* current = g->columns[to_col_index].head;
            while (current->next != NULL) {
                current = current->next;
            }
            dst = current->card;
        }
        return can_move_onto_column(src, dst);
    }else if(to_foundation_index != -1){
        if(g->foundations[to_foundation_index].head == NULL){
           dst = NULL;
        }else{
            Node* current = g->foundations[to_foundation_index].head;
            while (current->next != NULL) {
                current = current->next;
            }
            dst = current->card;
        }
        return can_move_onto_foundation(src, dst);
    }

    return false;
}

int game_move(GameState *g, const char *from, const char *to) {
  if (!game_can_move(g, from, to)) return -1;

  int from_col_index = get_column_index(from);
  int to_col_index = get_column_index(to);
  int from_foundation_index = get_foundation_index(from);
  int to_foundation_index = get_foundation_index(to);

  if (from_col_index != -1 && to_col_index != -1) {
    Node* last_node = g->columns[from_col_index].head;
    if(last_node == NULL) return -1;
    Node* pre_last_node = NULL;
    while(last_node->next != NULL){
        pre_last_node = last_node;
        last_node = last_node->next;
    }

    if(pre_last_node != NULL){
        pre_last_node->next = NULL;
    }else{
        g->columns[from_col_index].head = NULL;
    }
    
    last_node->next = g->columns[to_col_index].head;
    g->columns[to_col_index].head = last_node;

  } else if(from_col_index != -1 && to_foundation_index != -1) {
    // Move from column to foundation
  } else if(from_foundation_index != -1 && to_col_index != -1){
      // Move from foundation to colum
  }
    return 0;
}

void game_flip(GameState *g) {}

void game_print_state(const GameState *g) {}