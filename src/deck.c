#include "deck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//deck.c
/** Creates an empty deck. */
Deck* deck_create_empty(void) {
    Deck* d = (Deck*)malloc(sizeof(Deck));
    if (d == NULL) {
        perror("Failed to allocate memory for deck");
        return NULL; // Memory allocation failed
    }
    d->head = NULL;
    d->size = 0;
    return d;
}

/** Frees all memory allocated to the deck. */
void deck_free(Deck* d) {
    if (d == NULL) return;
    Node* current = d->head;
    Node* next;
    while (current != NULL) {
        Node* next = current->next;
        card_free(current->card);
        free(current);
        current = next;
    }
    free(d);
}

/** Loads a deck from a file, validating 52 cards. */
int deck_load_from_file(Deck* d, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) perror("Failed to open file");
    if (fp == NULL) return -1; // File not found

    char line[5];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) != 3) continue;
        enum Rank r;
        switch(line[0]) {
            case 'A': r = RANK_A; break;
            case '2': r = RANK_2; break;
            case '3': r = RANK_3; break;
            case '4': r = RANK_4; break;
            case '5': r = RANK_5; break;
            case '6': r = RANK_6; break;
            case '7': r = RANK_7; break;
            case '8': r = RANK_8; break;
            case '9': r = RANK_9; break;
            case 'T': r = RANK_10; break;
            case 'J': r = RANK_J; break;
            case 'Q': r = RANK_Q; break;
            case 'K': r = RANK_K; break;
            default: continue;
        }
        enum Suit s;
        switch(line[1]) {
            case 'C': s = CLUBS; break;
            case 'D': s = DIAMONDS; break;
            case 'H': s = HEARTS; break;
            case 'S': s = SPADES; break;
            default: continue;
        }
        Card* c = card_create(r, s, true);
        if (c == NULL) return -1;
        if (c == NULL) {
          fclose(fp);
          return -1;
        }
        Node* newNode = (Node*)malloc(sizeof(Node));
        if (newNode == NULL) {
          card_free(c);
          return -1;
        }
        newNode->card = c;
        newNode->next = d->head;
        d->head = newNode;
        d->size++;
    }

    fclose(fp);

    if (d->size != 52) {
        fprintf(stderr,"deck_load_from_file: Not enough cards in file.\n");
        deck_free(d);
        return -1;
    }
    return 0;
}

int deck_save_to_file(const Deck* d, const char* filename) {
    /** Saves the deck to a file */
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) perror("Failed to open file");
    if (fp == NULL) return -1; // File not found
    Node* current = d->head;
    while (current != NULL) {
        char* card_str = card_to_string(current->card);
        if(card_str == NULL){
            return -1;
        }
        fprintf(fp, "%s\n", card_str);
        free(card_str);
        current = current->next;
    }
    fclose(fp);
    return 0;
}

/** Interleaves the deck with a given split. */
void deck_shuffle_interleave(Deck* d, size_t split) {
    if (d == NULL || d->size <= 1 || split == 0 || split >= d->size) return;

    Node* head = d->head;
    Node* current = head;
    for (size_t i = 1; i < split; i++) {
        current = current->next;
    }

    Node* secondHalf = current->next;
    current->next = NULL;

    Node* firstHalf = head;

    Node* temp1 = NULL;
    Node* temp2 = NULL;

    while (firstHalf != NULL && secondHalf != NULL) {
        temp1 = firstHalf->next;
        temp2 = secondHalf->next;
        firstHalf->next = secondHalf;
        if (temp1 != NULL) {
            secondHalf->next = temp1;
        } else {
            secondHalf->next = NULL;
        }
        firstHalf = temp1;
        secondHalf = temp2;
    }

    d->head = head;
}

/** Randomly shuffles the deck. */
void deck_shuffle_random(Deck* d) {
  if(d == NULL || d->size <= 1) return;
  
  Card* card_array[d->size];
  Node* curr = d->head;
  size_t i = 0;

  while(curr != NULL){
    card_array[i++] = curr->card;
    curr = curr->next;
  }
  
  srand(time(NULL));
  for(i=d->size-1; i>0; i--){
    size_t j = rand() % (i+1);
    Card* temp = card_array[i];
    card_array[i] = card_array[j];
    card_array[j] = temp;
  }
  
  Node* new_head = (Node*) malloc(sizeof(Node));
  new_head->card = card_array[0];
  new_head->next = NULL;
  d->head = new_head;
  for (i=1; i<d->size; i++){
    Node* new_node = (Node*) malloc(sizeof(Node));
    new_node->card = card_array[i];
    new_node->next = d->head;
    d->head = new_node;
  }
}

/** Prints the deck for the SW command. */
void deck_print(const Deck* d) {
    Node* current = d->head;
    while(current != NULL){
      char* card_str = card_to_string(current->card);
      if(card_str != NULL){
        printf("%s ", card_str);
        free(card_str);
      }
      current = current->next;
    }
    printf("\n");
}