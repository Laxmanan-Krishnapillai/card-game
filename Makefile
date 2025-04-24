CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra -Iinclude
SRC_TEXT := src/card.c src/deck.c src/game.c src/ui_text.c src/main.c
OBJ_TEXT := $(SRC_TEXT:.c=.o)

all: text

text: yukon_text

yukon_text: $(OBJ_TEXT)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f yukon_text src/*.o
	
.PHONY: all text clean