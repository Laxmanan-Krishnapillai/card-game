/* tests.c – builds against the single‑header engine */
#define YUKON_ENGINE_IMPLEMENTATION
#include "yukon_engine.h"
#include <assert.h>
int main(void) {
  Game g;
  engine_init(&g);
  engine_execute(&g, "LD");                  /* ordered deck */
  assert(engine_tableau_height(&g, 0) == 0); /* still STARTUP */

  engine_execute(&g, "SI 26"); /* perfect interleave */
  engine_execute(&g, "P");     /* deal */
  static int exp[7] = {1, 6, 7, 8, 9, 10, 11};
  for (int i = 0; i < 7; ++i)
    assert(engine_tableau_height(&g, i) == exp[i]);

  /* bottom card C3 should be face‑up */
  const Card *b = engine_tableau_card(&g, 2, exp[2] - 1);
  assert(b && b->face_up);

  engine_execute(&g, "Q"); /* back to STARTUP */
  engine_execute(&g, "P"); /* deal again */
  for (int i = 0; i < 7; ++i)
    assert(engine_tableau_height(&g, i) == exp[i]);
  return 0;
}