#ifndef _RENDER_TEXT_H
#define _RENDER_TEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 



void renderTextStatic(SDL_Renderer *renderer, TTF_Font *font, int textWidth);

void RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *current, const char *future, const char *past, int textWidth);

void simulatedDate (SDL_Renderer *renderer, TTF_Font *font,const char *simulated, int textWidth);
#endif
