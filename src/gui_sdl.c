/* gui_sdl.c – SDL2 graphical front‑end for Yukon Solitaire
 * --------------------------------------------------------
 *  compile:   gcc gui_sdl.c -o yukon_gui `sdl2-config --cflags --libs` -std=c11
 */
#define YUKON_ENGINE_IMPLEMENTATION
#include "yukon_engine.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define CARD_W 60
#define CARD_H 80
#define GAP_X 20
#define GAP_Y 25
#define ORIGIN_X 20
#define ORIGIN_Y 40
#define ENTRY_H 30

static SDL_Window *win;
static SDL_Renderer *rnd;
static SDL_Texture *fontTex = NULL; /* optional bitmap font */
static TTF_Font *font;
static void blit_text(int x, int y, const char *s, SDL_Color c) {
  if (!s || !*s)
    return; /* ←── NEW */
  SDL_Surface *surf = TTF_RenderUTF8_Blended(font, s, c);
  SDL_Texture *tex = SDL_CreateTextureFromSurface(rnd, surf);
  SDL_Rect dst = {x, y, surf->w, surf->h};
  SDL_RenderCopy(rnd, tex, NULL, &dst);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(surf);
}

static void draw_card(int x, int y, const Card *c) {
  SDL_Rect rect = {x, y, CARD_W, CARD_H};
  if (!c) {
    SDL_SetRenderDrawColor(rnd, 32, 32, 32, 255);
    SDL_RenderFillRect(rnd, &rect);
    return;
  }
  if (c->face_up) {
    SDL_SetRenderDrawColor(rnd, 240, 240, 240, 255);
  } else {
    SDL_SetRenderDrawColor(rnd, 64, 64, 128, 255);
  }
  SDL_RenderFillRect(rnd, &rect);
  SDL_SetRenderDrawColor(rnd, 0, 0, 0, 255);
  SDL_RenderDrawRect(rnd, &rect);
  if (c && c->face_up) {
    char txt[3] = {engine_rank_ch(c->rank), engine_suit_ch(c->suit), 0};
    SDL_Color col = {0, 0, 0, 255};
    if (c->suit == HEARTS || c->suit == DIAMONDS)
      col = (SDL_Color){200, 0, 0, 255};
    blit_text(x + 4, y + 4, txt, col);
  }
  // (void)txt;
  // (void)p;
}

static void render_game(const Game *g, const char *entryText, bool showDeck) {
  SDL_SetRenderDrawColor(rnd, 0, 128, 0, 255); /* table green */
  SDL_RenderClear(rnd);

  /* tableau */
  for (int col = 0; col < 7; ++col) {
    int h = engine_tableau_height(g, col);
    for (int row = 0; row < h; ++row) {
      const Card *c = engine_tableau_card(g, col, row);
      int x = ORIGIN_X + col * (CARD_W + GAP_X);
      int y = ORIGIN_Y + row * GAP_Y;
      draw_card(x, y, c);
    }
  }
  /* foundations (vertical) */
  for (int f = 0; f < 4; ++f) {
    const Card *c = engine_foundation_top(g, f);
    int x = ORIGIN_X + 7 * (CARD_W + GAP_X);
    int y = ORIGIN_Y + f * (CARD_H + 10);
    draw_card(x, y, c);
  }
  if (showDeck && g->deck) {
    int col = 0, row = 0;
    for (const Card *c = g->deck; c; c = c->next) {
      int x = ORIGIN_X + col * (CARD_W + GAP_X);
      int y = ORIGIN_Y + row * (CARD_H + 10);

      Card tmp = *c;   /* make a stack copy */
      tmp.face_up = 1; /* force face-up just for this frame */
      draw_card(x, y, &tmp);

      if (++col == 7) {
        col = 0;
        ++row;
      }
    }
  }
  /* entry field */
  SDL_Rect entryRect = {ORIGIN_X, 600, 700, ENTRY_H};
  SDL_SetRenderDrawColor(rnd, 255, 255, 255, 255);
  SDL_RenderFillRect(rnd, &entryRect);
  SDL_SetRenderDrawColor(rnd, 0, 0, 0, 255);
  SDL_RenderDrawRect(rnd, &entryRect);
  /* TODO: render entry text – requires SDL_ttf; omitted so it compiles without
   * extra deps */
  SDL_Color black = {0, 0, 0, 255};
  blit_text(entryRect.x + 6, entryRect.y + 6, entryText, black);

  SDL_RenderPresent(rnd);
  (void)entryText;
}

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL: %s\n", SDL_GetError());
    return 1;
  }

  if (TTF_Init() != 0) {
    fprintf(stderr, "SDL_ttf: %s\n", TTF_GetError());
    return 1;
  }

  font = TTF_OpenFont("OpenSans-Regular.ttf", 16);
  if (!font) {
    fprintf(stderr, "OpenFont: %s\n", TTF_GetError());
    return 1;
  }

  win = SDL_CreateWindow("Yukon Solitaire", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 960, 720, SDL_WINDOW_SHOWN);
  rnd = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_StartTextInput();

  Game g;
  engine_init(&g);
  char entry[128] = "";
  size_t len = 0;
  bool showDeck = false;
  int running = 1;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_QUIT:
        running = 0;
        break;
      case SDL_TEXTINPUT:
        if (len + strlen(e.text.text) < sizeof entry - 1) {
          strcat(entry, e.text.text);
          len = strlen(entry);
        }
        break;
      case SDL_KEYDOWN:
        if (e.key.keysym.sym == SDLK_BACKSPACE && len > 0) {
          entry[--len] = '\0';
        } else if (e.key.keysym.sym == SDLK_RETURN) {
          /* ---- NEW: canonicalise to upper-case ---- */
          char upper[128];
          for (size_t i = 0; i <= len; ++i)
            upper[i] = toupper((unsigned char)entry[i]);

          engine_execute(&g, upper); /* old: engine_execute(&g, entry); */
          bool deck_view = (strcmp(upper, "SW") == 0 && g.phase == STARTUP);

          /* optional: dump engine message to stdout for debugging */
          printf("Message:%s\n", g.message);

          if (!strcmp(upper, "QQ"))
            running = 0;
          if (!strcmp(upper, "SW") && g.phase == STARTUP &&
              !strcmp(g.message, "(ignored in engine)"))
            showDeck = true; /* deck present */
          else
            showDeck = false; /* either not SW or engine rejected it */
          entry[0] = '\0';
          len = 0; /* clear the entry box */
        } else if (e.key.keysym.sym == SDLK_ESCAPE) {
          running = 0;
        }
        break;
      }
    }
    render_game(&g, entry, showDeck);
  }
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();
  return 0;
}
