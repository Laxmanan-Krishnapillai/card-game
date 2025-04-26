/*
 * yukon_engine.h — single‑header Yukon Solitaire core
 *
 *   #define YUKON_ENGINE_IMPLEMENTATION
 *   #include "yukon_engine.h"
 *
 * in **exactly one** translation unit to generate the implementation.
 * Any other file should include it without defining the macro.
 *
 * Public API (minimal):
 *   - typedefs  : Card, Game, Suit, Phase
 *   - void  engine_init(Game* g);           // zero‑init
 *   - void  engine_execute(Game* g, const char* line); // run one command
 *   - /* helpers for read‑only drawing
 *     int           engine_tableau_height(const Game*, int col);
 *     const Card*   engine_tableau_card  (const Game*, int col, int row);
 *     const Card*   engine_foundation_top(const Game*, int f);
 *
 * The command set is identical to the PDF (LD, SW, SI, SR, SD, QQ, P, Q,
 * moves).
 */
#ifndef YUKON_ENGINE_H
#define YUKON_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*———————————————————  Types  ———————————————————*/

typedef enum { CLUBS, DIAMONDS, HEARTS, SPADES } Suit;

typedef struct Card {
  int rank;          /* 1 – 13  */
  Suit suit;         /* enum    */
  int face_up;       /* bool    */
  struct Card *next; /* forward list */
} Card;

static inline char engine_rank_ch(int r) { return " A23456789TJQK"[r]; }
static inline char engine_suit_ch(Suit s) { return "CDHS"[s]; }

typedef enum { STARTUP, PLAY } Phase;

typedef struct Game {
  Phase phase;
  Card *deck;
  Card *columns[7];
  Card *foundations[4];
  char last_cmd[128];
  char message[128];
  Card *backup_deck; /* snapshot taken right before first deal */
} Game;

/*———————————————————  API  ———————————————————*/
void engine_init(Game *g);
void engine_execute(Game *g, const char *line); /* run one textual command */

/* read‑only helpers for GUIs */
int engine_tableau_height(const Game *, int col); /* 0‑based col */
const Card *engine_tableau_card(const Game *, int col,
                                int row);               /* row 0 = top */
const Card *engine_foundation_top(const Game *, int f); /* f 0‑3 */

#ifdef __cplusplus
} /* extern C */
#endif

/*———————————————————  IMPLEMENTATION  ———————————————————*/
#ifdef YUKON_ENGINE_IMPLEMENTATION

#define _CRT_SECURE_NO_WARNINGS 1
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*—— helpers ——*/
static Card *_new_card(int r, Suit s, int up) {
  Card *c = (Card *)malloc(sizeof *c);
  c->rank = r;
  c->suit = s;
  c->face_up = up;
  c->next = NULL;
  return c;
}
static void _push(Card **h, Card *c) {
  c->next = *h;
  *h = c;
}
static Card *_pop(Card **h) {
  Card *c = *h;
  if (c) {
    *h = c->next;
    c->next = NULL;
  }
  return c;
}
static void _append(Card **h, Card *c) {
  if (!*h) {
    *h = c;
  } else {
    Card *p = *h;
    while (p->next)
      p = p->next;
    p->next = c;
  }
}
static void _freelist(Card *h) {
  while (h) {
    Card *n = h->next;
    free(h);
    h = n;
  }
}
static int _col_height(Card *c) {
  int n = 0;
  while (c) {
    ++n;
    c = c->next;
  }
  return n;
}
static void _flip_bottom(Card *c) {
  if (!c)
    return;
  while (c->next)
    c = c->next;
  c->face_up = 1;
}

static int _rank_from(char r) {
  if (r == 'A')
    return 1;
  if (r >= '2' && r <= '9')
    return r - '0';
  if (r == 'T')
    return 10;
  if (r == 'J')
    return 11;
  if (r == 'Q')
    return 12;
  if (r == 'K')
    return 13;
  return -1;
}
static int _suit_from(char s) {
  switch (s) {
  case 'C':
    return CLUBS;
  case 'D':
    return DIAMONDS;
  case 'H':
    return HEARTS;
  case 'S':
    return SPADES;
  default:
    return -1;
  }
}

void engine_init(Game *g) {
  memset(g, 0, sizeof *g);
  g->phase = STARTUP;
}

/*———— core command helpers (same logic as previous monolithic file) ————*/
static void _msg(Game *g, const char *m) {
  strncpy(g->message, m, sizeof g->message - 1);
  g->message[sizeof g->message - 1] = '\0';
}

static int _can_place(Card *dest, Card *src) {
  if (!dest)
    return src->rank == 13;
  if (dest->suit == src->suit)
    return 0;
  return dest->rank == src->rank + 1;
}

#include <stdbool.h>

static void _cmd_LD(Game *g, const char *fn) {
  _freelist(g->deck);
  g->deck = NULL;
  if (fn && *fn) {
    FILE *fp = fopen(fn, "r");
    if (!fp) {
      _msg(g, "Error: file not found");
      return;
    }
    int seen[4][14] = {0}, line = 0;
    char buf[16];
    while (fgets(buf, sizeof buf, fp)) {
      if (buf[0] == '\n')
        continue;
      ++line;
      int r = _rank_from(buf[0]);
      int s = _suit_from(buf[1]);
      if (r < 0 || s < 0 || seen[s][r]) {
        fclose(fp);
        _msg(g, "Error: bad deck");
        return;
      }
      seen[s][r] = 1;
      _append(&g->deck, _new_card(r, (Suit)s, 0));
    }
    fclose(fp);
    int count = 0;
    for (Card *c = g->deck; c; c = c->next)
      ++count;
    if (count != 52) {
      _msg(g, "Error: deck size");
      return;
    }
  } else {
    for (int s = 0; s < 4; ++s)
      for (int r = 1; r <= 13; ++r)
        _append(&g->deck, _new_card(r, (Suit)s, 0));
  }
  _msg(g, "OK");
}

static void _interleave_shuffle(Game *g, int split) {
  if (!g->deck) {
    _msg(g, "Error: No deck");
    return;
  }
  int len = 0;
  for (Card *p = g->deck; p; p = p->next)
    ++len;
  if (split <= 0 || split >= len)
    split = rand() % (len - 1) + 1;
  Card *p = g->deck;
  for (int i = 1; i < split; ++i)
    p = p->next;
  Card *pile2 = p->next;
  p->next = NULL;
  Card *pile1 = g->deck;
  Card *shuf = NULL, **tail = &shuf;
  while (pile1 || pile2) {
    if (pile1) {
      *tail = pile1;
      tail = &pile1->next;
      pile1 = *tail;
    }
    if (pile2) {
      *tail = pile2;
      tail = &pile2->next;
      pile2 = *tail;
    }
  }
  g->deck = shuf;
  _msg(g, "OK");
}

static void _random_shuffle(Game *g) {
  if (!g->deck) {
    _msg(g, "Error: No deck");
    return;
  }
  Card *un = g->deck, *sh = NULL;
  srand((unsigned)time(NULL));
  while (un) {
    Card *c = _pop(&un);
    if (!sh)
      sh = c;
    else {
      int span = 0;
      for (Card *q = sh; q; q = q->next)
        ++span;
      int pos = rand() % (span + 1);
      if (pos == 0) {
        c->next = sh;
        sh = c;
      } else {
        Card *q = sh;
        for (int i = 0; i < pos - 1; ++i)
          q = q->next;
        c->next = q->next;
        q->next = c;
      }
    }
  }
  g->deck = sh;
  _msg(g, "OK");
}

static void _save_deck(Game *g, const char *fn) {
  if (!g->deck) {
    _msg(g, "Error: No deck");
    return;
  }
  const char *f = (fn && *fn) ? fn : "cards.txt";
  FILE *fp = fopen(f, "w");
  if (!fp) {
    _msg(g, "Error: cant open");
    return;
  }
  for (Card *c = g->deck; c; c = c->next)
    fprintf(fp, "%c%c\n", engine_rank_ch(c->rank), engine_suit_ch(c->suit));
  fclose(fp);
  _msg(g, "OK");
}

static void _deal(Game *g) {
  /* --- snapshot current deck for later “Q” restore (deep copy) --- */
  if (!g->backup_deck) { /* only take it once */
    Card *src = g->deck;
    Card *dup_head = NULL, **tail = &dup_head;
    while (src) {
      Card *dup = _new_card(src->rank, src->suit, 0); /* face-down copy */
      *tail = dup;
      tail = &dup->next;
      src = src->next;
    }
    g->backup_deck = dup_head;
  }
  static const int layout[7] = {1, 6, 7, 8, 9, 10, 11};
  static const int facedn[7] = {0, 1, 2, 3, 4, 5, 6};
  for (int row = 0; row < 11; ++row) {
    for (int col = 0; col < 7; ++col) {
      if (row < layout[col]) {
        Card *c = _pop(&g->deck);
        if (!c) {
          _msg(g, "Error: not enough");
          return;
        }
        c->face_up = (row >= facedn[col]);
        _append(&g->columns[col], c);
      }
    }
  }
  g->phase = PLAY;
  _msg(g, "OK");
}

/*—————————  moves —————————*/
static bool _seq_move(Game *g, int sc, int rank, Suit suit, int dc) {
  Card *col = g->columns[sc], *prev = NULL;
  while (col && !(col->rank == rank && col->suit == suit && col->face_up)) {
    prev = col;
    col = col->next;
  }
  if (!col) {
    _msg(g, "Error: card not found");
    return false;
  }
  Card *dest = g->columns[dc];
  while (dest && dest->next)
    dest = dest->next;
  if (!_can_place(dest, col)) {
    _msg(g, "Error: bad move");
    return false;
  }
  if (prev)
    prev->next = NULL;
  else
    g->columns[sc] = NULL;
  if (dest)
    dest->next = col;
  else
    g->columns[dc] = col;
  _flip_bottom(g->columns[sc]);
  _msg(g, "OK");
  return true;
}

static bool _col_to_found(Game *g, int sc, int f) {
  Card *c = g->columns[sc], *prev = NULL;
  if (!c) {
    _msg(g, "Error: empty col");
    return false;
  }
  while (c->next) {
    prev = c;
    c = c->next;
  }
  if (!c->face_up) {
    _msg(g, "Error: face down");
    return false;
  }
  Card *top = g->foundations[f];
  if (top) {
    if (top->suit != c->suit || c->rank != top->rank + 1) {
      _msg(g, "Error: bad foundation");
      return false;
    }
  } else if (c->rank != 1) {
    _msg(g, "Error: need Ace");
    return false;
  }
  if (prev)
    prev->next = NULL;
  else
    g->columns[sc] = NULL;
  c->next = top;
  g->foundations[f] = c;
  _flip_bottom(g->columns[sc]);
  _msg(g, "OK");
  return true;
}

static bool _found_to_col(Game *g, int f, int dc) {
  Card *c = g->foundations[f];
  if (!c) {
    _msg(g, "Error: foundation empty");
    return false;
  }
  Card *dest = g->columns[dc];
  while (dest && dest->next)
    dest = dest->next;
  if (!_can_place(dest, c)) {
    _msg(g, "Error: bad move");
    return false;
  }
  g->foundations[f] = c->next;
  c->next = NULL;
  if (dest)
    dest->next = c;
  else
    g->columns[dc] = c;
  _msg(g, "OK");
  return true;
}

/*—————————  engine_execute —————————*/
void engine_execute(Game *g, const char *line) {
  strncpy(g->last_cmd, line, sizeof g->last_cmd - 1);
  g->last_cmd[sizeof g->last_cmd - 1] = '\0';

  char cmd[8];
  if (sscanf(line, "%7s", cmd) != 1)
    return;

  if (g->phase == STARTUP) {
    if (!strcmp(cmd, "LD")) {
      char fn[64] = "";
      sscanf(line, "LD %63s", fn);
      _cmd_LD(g, fn);
      return;
    }
    if (!strcmp(cmd, "SW")) {
      _msg(g, "(ignored in engine)");
      return;
    }
    if (!strcmp(cmd, "SI")) {
      int sp = 0;
      sscanf(line, "SI %d", &sp);
      _interleave_shuffle(g, sp);
      return;
    }
    if (!strcmp(cmd, "SR")) {
      _random_shuffle(g);
      return;
    }
    if (!strcmp(cmd, "SD")) {
      char fn[64] = "";
      sscanf(line, "SD %63s", fn);
      _save_deck(g, fn);
      return;
    }
    if (!strcmp(cmd, "P")) {
      _deal(g);
      return;
    }
    if (!strcmp(cmd, "QQ")) {
      _freelist(g->backup_deck);
      exit(EXIT_SUCCESS);
    }
    _msg(g, "Error: unknown");
    return;
  }
  /*— PLAY —*/
  if (!strcmp(cmd, "Q")) {
    /* free columns and foundations */
    for (int c = 0; c < 7; ++c) {
      _freelist(g->columns[c]);
      g->columns[c] = NULL;
    }
    for (int f = 0; f < 4; ++f) {
      _freelist(g->foundations[f]);
      g->foundations[f] = NULL;
    }

    /* restore the snapshot into g->deck (if it exists) */
    _freelist(g->deck); /* safety: clear any leftover */
    g->deck = g->backup_deck;
    g->backup_deck = NULL; /* one-shot restore */

    g->phase = STARTUP;
    _msg(g, "OK");
    return;
  }

  char src[32], dst[32];
  if (sscanf(line, "%31[^-]->%31s", src, dst) == 2) {
    /* trim spaces */ for (char *p = src; *p; ++p)
      if (isspace((unsigned char)*p))
        *p = '\0';
    if (src[0] == 'F') {
      int f = src[1] - '1';
      int dc = dst[1] - '1';
      _found_to_col(g, f, dc);
      return;
    }
    if (src[0] == 'C') {
      int sc = src[1] - '1';
      if (strchr(src, ':')) {
        char rch, sch;
        sscanf(src, "C%d:%c%c", &sc, &rch, &sch);
        _seq_move(g, sc, _rank_from(rch), (Suit)_suit_from(sch), dst[1] - '1');
      } else {
        if (dst[0] == 'F')
          _col_to_found(g, sc, dst[1] - '1');
        else if (dst[0] == 'C') {
          Card *p = g->columns[sc];
          if (!p) {
            _msg(g, "Error: empty");
            return;
          }
          while (p->next)
            p = p->next;
          _seq_move(g, sc, p->rank, p->suit, dst[1] - '1');
        }
      }
      return;
    }
    _msg(g, "Error: bad move");
    return;
  }
  _msg(g, "Error: bad syntax");
}

/*—————————  read‑only helpers —————————*/
int engine_tableau_height(const Game *g, int col) {
  return _col_height(col >= 0 && col < 7 ? g->columns[col] : NULL);
}

const Card *engine_tableau_card(const Game *g, int col, int row) {
  Card *p = (col >= 0 && col < 7) ? g->columns[col] : NULL;
  for (int i = 0; i < row && p; i++)
    p = p->next;
  return p;
}
const Card *engine_foundation_top(const Game *g, int f) {
  return (f >= 0 && f < 4) ? g->foundations[f] : NULL;
}

#endif /* YUKON_ENGINE_IMPLEMENTATION */

#endif /* YUKON_ENGINE_H */
