# ---- paths ----
SRCDIR := src
BINDIR := bin
TESTSRC := $(SRCDIR)/tests.c
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -I$(SRCDIR)

# ---- SDL detection (only if present) ----
SDL_CONFIG := $(shell command -v sdl2-config 2>/dev/null)
ifeq ($(SDL_CONFIG),)
    $(info *** SDL2 not found – GUI target will be skipped ***)
    SDL_CFLAGS :=
    SDL_LIBS   :=
    GUI_ENABLED := 0
else
    SDL_CFLAGS := $(shell $(SDL_CONFIG) --cflags)
    SDL_LIBS   = $(shell sdl2-config --libs) -lSDL2_ttf    
    GUI_ENABLED := 1
endif

# ---- targets ----
.PHONY: all tui gui test clean
all: tui gui            # builds what is possible

tui: | $(BINDIR)
	$(CC) $(CFLAGS) $(SRCDIR)/tui.c -o $(BINDIR)/tui

ifeq ($(GUI_ENABLED),1)
gui: | $(BINDIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(SRCDIR)/gui_sdl.c -o $(BINDIR)/yukon_gui $(SDL_LIBS)
else
gui:
	@echo \"(skipped) GUI requires SDL2\"
endif

$(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(BINDIR)

# --- unit test ----------------------------------------------------------
test: | $(BINDIR)
	$(CC) $(CFLAGS) -DYUKON_ENGINE_IMPLEMENTATION $(TESTSRC) -o $(BINDIR)/tests
	@echo "Running unit tests…"
	@$(BINDIR)/tests && echo "✓ All tests passed"