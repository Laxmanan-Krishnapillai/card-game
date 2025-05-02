/* tui.c – text-mode Yukon Solitaire using yukon_engine.h */
#define YUKON_ENGINE_IMPLEMENTATION /* generate engine once here */
#include "yukon_engine.h"
#include <stdio.h>
#include <string.h>

/* ── helpers ─────────────────────────────────────────────────────────── */
static void render_terminal(const Game *g) {
  printf("C1\tC2\tC3\tC4\tC5\tC6\tC7\tF\n");
  int rows = 0;
  for (int i = 0; i < 7; ++i) {
    int h = engine_tableau_height(g, i);
    if (h > rows)
      rows = h;
  }
  if (rows < 4)
    rows = 4; /* room for F1‒F4 */

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < 7; ++c) {
      const Card *card = engine_tableau_card(g, c, r);
      if (card && card->face_up)
        printf("%c%c", engine_rank_ch(card->rank), engine_suit_ch(card->suit));
      else if (card)
        printf("[]");
      else
        printf("  ");
      printf("\t");
    }
    if (r < 4) {
      const Card *f = engine_foundation_top(g, r);
      if (f)
        printf("%c%c", engine_rank_ch(f->rank), engine_suit_ch(f->suit));
      else
        printf("[]");
      printf(" F%d", r + 1);
    }
    putchar('\n');
  }
  printf("\nLAST Command:%s\nMessage:%s\nINPUT > ", g->last_cmd, g->message);
}

static void show_deck(const Game *g) {
  puts("C1\tC2\tC3\tC4\tC5\tC6\tC7\n");
  const Card *c = g->deck;
  int col = 0;
  while (c) {
    printf("%c%c\t", engine_rank_ch(c->rank), engine_suit_ch(c->suit));
    if (++col == 7) {
      putchar('\n');
      col = 0;
    }
    c = c->next;
  }
  puts("");
}

/* ── main loop ───────────────────────────────────────────────────────── */
int main(void) {
  Game g;
  engine_init(&g);
  char buf[128];
  puts("--- Yukon Solitaire (terminal) ---  [type QQ to quit]\n");

  while (1) {
    if (g.phase == STARTUP)
      printf("INPUT > ");
    if (!fgets(buf, sizeof buf, stdin))
      break;
    buf[strcspn(buf, "\n")] = '\0';
    if (!*buf)
      continue; /* ignore blank line */

    engine_execute(&g, buf);
    if (!strcmp(buf, "QQ"))
      break; /* exit after processing */

    /* front‑end output logic */
    if (!strcmp(buf, "SW") && g.phase == STARTUP) {
      if (!strcmp(g.message, "(ignored in engine)"))
        show_deck(&g); /* deck present → show it */
      else
        printf("Message:%s\n", g.message); /* pass the error through */
    } else if (g.phase == PLAY) {
      render_terminal(&g);
    } else {
      printf("Message:%s\n", g.message);
    }
  }
  return 0;
}
