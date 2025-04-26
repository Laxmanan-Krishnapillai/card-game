/* gui_sdl.c – SDL2 graphical front‑end for Yukon Solitaire
 * --------------------------------------------------------
 *  compile:   gcc gui_sdl.c -o yukon_gui `sdl2-config --cflags --libs` -std=c11
 */
#define YUKON_ENGINE_IMPLEMENTATION
#include "yukon_engine.h"
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#define CARD_W  60
#define CARD_H  80
#define GAP_X   20
#define GAP_Y   25
#define ORIGIN_X 20
#define ORIGIN_Y 40
#define ENTRY_H  30

static SDL_Window  *win;
static SDL_Renderer*rnd;
static SDL_Texture *fontTex = NULL; /* optional bitmap font */

static void draw_card(int x,int y,const Card*c)
{
    SDL_Rect rect = { x, y, CARD_W, CARD_H };
    if(!c){ SDL_SetRenderDrawColor(rnd, 32,32,32,255); SDL_RenderFillRect(rnd,&rect); return; }
    if(c->face_up){ SDL_SetRenderDrawColor(rnd, 240, 240, 240, 255);} else { SDL_SetRenderDrawColor(rnd, 64,64,128,255);}    
    SDL_RenderFillRect(rnd,&rect);
    SDL_SetRenderDrawColor(rnd, 0,0,0,255);
    SDL_RenderDrawRect(rnd,&rect);
    /* rudimentary rank/suit glyph – top‑left corner */
    char txt[3] = { engine_rank_ch(c->rank), engine_suit_ch(c->suit), 0 };
    SDL_Rect p = { x+4, y+4, 8, 12 }; /* crude 8×12 bitmap using SDL_gfx? skipped */
    /* You can plug in SDL_ttf later; skipped for brevity */
    (void)txt; (void)p;
}

static void render_game(const Game* g,const char* entry)
{
    SDL_SetRenderDrawColor(rnd, 0, 128, 0, 255); /* table green */
    SDL_RenderClear(rnd);

    /* tableau */
    for(int col=0; col<7; ++col){
        int h=engine_tableau_height(g,col);
        for(int row=0; row<h; ++row){
            const Card*c=engine_tableau_card(g,col,row);
            int x = ORIGIN_X + col*(CARD_W+GAP_X);
            int y = ORIGIN_Y + row*GAP_Y;
            draw_card(x,y,c);
        }
    }
    /* foundations (vertical) */
    for(int f=0; f<4; ++f){
        const Card*c=engine_foundation_top(g,f);
        int x = ORIGIN_X + 7*(CARD_W+GAP_X);
        int y = ORIGIN_Y + f*(CARD_H+10);
        draw_card(x,y,c);
    }

    /* entry field */
    SDL_Rect entry = { ORIGIN_X, 600, 700, ENTRY_H };
    SDL_SetRenderDrawColor(rnd, 255,255,255,255); SDL_RenderFillRect(rnd,&entry);
    SDL_SetRenderDrawColor(rnd, 0,0,0,255); SDL_RenderDrawRect(rnd,&entry);
    /* TODO: render entry text – requires SDL_ttf; omitted so it compiles without extra deps */

    SDL_RenderPresent(rnd);
}

int main(int argc,char**argv)
{
    if(SDL_Init(SDL_INIT_VIDEO)!=0){ fprintf(stderr,"SDL: %s\n",SDL_GetError()); return 1; }
    win = SDL_CreateWindow("Yukon Solitaire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 720, SDL_WINDOW_SHOWN);
    rnd = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    SDL_StartTextInput();

    Game g; engine_init(&g);
    char entry[128]=""; size_t len=0;
    int running=1;
    while(running){
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            switch(e.type){
            case SDL_QUIT: running=0; break;
            case SDL_TEXTINPUT:
                if(len+strlen(e.text.text) < sizeof entry-1){ strcat(entry,e.text.text); len=strlen(entry);} break;
            case SDL_KEYDOWN:
                if(e.key.keysym.sym==SDLK_BACKSPACE && len>0){ entry[--len]='\0'; }
                else if(e.key.keysym.sym==SDLK_RETURN){
                    engine_execute(&g,entry);
                    if(!strcmp(entry,"Q")||!strcmp(entry,"QQ")) running=0; /* allow quit */
                    entry[0]='\0'; len=0;
                }
                else if(e.key.keysym.sym==SDLK_ESCAPE){ running=0; }
                break;
            }
        }
        render_game(&g,entry);
    }
    SDL_Quit();
    return 0;
}
