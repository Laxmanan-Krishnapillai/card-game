#include "ui_text.h"
#include "game.h"
#include "deck.h"
#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// draw_board: Draws the game board to the console.
static void draw_board(const GameState* g) {
  // Print foundations
    printf("F1\tF2\tF3\tF4\n");
    for (int i = 0; i < 4; i++) {
      if(g->foundations[i].head == NULL){
        printf("[ ]\t");
      }else{
        char* card_str = card_to_string(g->foundations[i].head->card);
        if(card_str != NULL){
          printf("%s\t", card_str);
          free(card_str);
        }
      }
    }
    printf("\n");

    // Print columns
    printf("C1\tC2\tC3\tC4\tC5\tC6\tC7\n");
    int max_cards = 0;
    for(int i = 0; i < 7; i++){
      if(g->columns[i].size > max_cards){
        max_cards = g->columns[i].size;
      }
    }

    for (int i = 0; i < max_cards; i++) {
        for (int j = 0; j < 7; j++) {
            Node* current = g->columns[j].head;
            int count = 0;
            while (current != NULL && count < g->columns[j].size - i - 1 ) {
                current = current->next;
                count++;
            }
            
            if (current == NULL) {
                printf("\t");
            } else {
                char* card_str = card_to_string(current->card);
                if (card_str == NULL) {
                    printf("[!]\t");
                } else {
                    printf("%s\t", card_str);
                    free(card_str);
                }
            }
        }
        printf("\n");
    }
}

// prompt: Displays the last command and message.
static void prompt(const char* last_cmd, const char* msg) {
  printf("Last Command: %s\n", last_cmd);
  printf("Message: %s\n", msg);
}

// handle_startup_cmd: Handles commands during the startup phase.
static void handle_startup_cmd(char* line, Deck* d, bool* quit) {
  char cmd[3];
  char arg[250];

  if (sscanf(line, "%2s %249s", cmd, arg) >= 1) {
    if (strcmp(cmd, "LD") == 0) {
      Deck* temp_deck = deck_create_empty();
      int load_result = deck_load_from_file(temp_deck, arg);
      if (load_result != 0) {
        printf("Error: Failed to load deck from %s\n", arg);
        deck_free(temp_deck);
      } else {
        deck_free(d);
        *d = *temp_deck;
        free(temp_deck);
        printf("Deck loaded from %s\n", arg);
      }
    } else if (strcmp(cmd, "SW") == 0) {
      deck_print(d);
    } else if (strcmp(cmd, "SI") == 0) {
        int split;
        if (sscanf(arg, "%d", &split) == 1) {
            deck_shuffle_interleave(d, split);
            printf("Deck shuffled with interleave and split %d.\n", split);
        } else {
            printf("Error: Invalid split value.\n");
        }
    }else if (strcmp(cmd, "SR") == 0) {
        deck_shuffle_random(d);
        printf("Deck shuffled randomly.\n");
    }else if (strcmp(cmd, "SD") == 0) {
        int save_result = deck_save_to_file(d, arg);
        if (save_result != 0) {
            printf("Error: Failed to save deck to %s\n", arg);
        } else {
            printf("Deck saved to %s\n", arg);
        }
    } else if (strcmp(cmd, "QQ") == 0) {
        *quit = true;
        deck_free(d);
    }else {
      printf("Invalid command.\n");
    }
  } else {
    if (strcmp(line, "QQ") == 0) {
        *quit = true;
        deck_free(d);
    } else {
      printf("Invalid command format.\n");
    }
  }
}

// handle_play_cmd: Handles commands during the play phase.
static void handle_play_cmd(char* line, GameState* g, bool* quit_game, bool* quit_app) {
    char from[5], to[5];
    char last_cmd[256] = "";
    char msg[256] = "";
    
    if (strcmp(line, "Q") == 0) {
        *quit_game = true;
        game_free(g);
    } else if (strcmp(line, "P") == 0) {
        game_flip(g);
        strcpy(last_cmd, "P");
        strcpy(msg, "Cards dealt and flipped.");
    } else if (sscanf(line, "%4s->%4s", from, to) == 2) {
        if(game_can_move(g, from, to)){
          if (game_move(g, from, to) == 0){
              game_flip(g);
              strcpy(last_cmd, line);
              strcpy(msg, "Move successful.");
          }else{
            strcpy(last_cmd, line);
            strcpy(msg, "Invalid move.");
          }
        }else{
            strcpy(last_cmd, line);
            strcpy(msg, "Invalid move.");
        }
    } else {
        strcpy(msg, "Invalid command format.");
        strcpy(last_cmd, line);
    }
    if (strcmp(msg, "") != 0 || strcmp(last_cmd, "") != 0){
      prompt(last_cmd, msg);
    }
}

// ui_text_startup_loop: Manages the startup loop for the text UI.
void ui_text_startup_loop(void) {
  char line[256];
  Deck *d = deck_create_empty();
  bool quit = false;
  while (!quit) {
    printf("Enter command: ");
    if (fgets(line, sizeof(line), stdin) == NULL) {
      break;
    }

    // Remove trailing newline
    size_t ln = strlen(line) - 1;
    if (line[ln] == '\n') {
      line[ln] = '\0';
    }
    if (strcmp(line, "P") == 0) {
      GameState* g = game_create(d);
      if (g) {
        game_deal(g);
        ui_text_play_loop(g);
        game_free(g);
        deck_free(d);
      }
      quit = true;
    }
    handle_startup_cmd(line, d, &quit);
  }
  deck_free(d);
}

// ui_text_play_loop: Manages the play loop for the text UI.
void ui_text_play_loop(GameState* g) {
  char line[256];
  bool quit_game = false;
  bool quit_app = false;
  while (!quit_game && !quit_app) {
    draw_board(g);
    printf("Enter command: ");
    if (fgets(line, sizeof(line), stdin) == NULL) {
      break;
    }
    size_t ln = strlen(line) - 1;
    if (line[ln] == '\n') {
      line[ln] = '\0';
    }
    handle_play_cmd(line, g, &quit_game, &quit_app);
  }
  if (quit_app) {
    exit(0);
  }
}