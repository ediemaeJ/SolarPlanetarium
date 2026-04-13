#include "render_text.h"


 SDL_Color white = {255, 255, 255, 255};


// Internal helper — renders a single string at (x, y), no manual surface/texture management needed
static void drawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, int wrapWidth) {
    SDL_Surface *surface = wrapWidth > 0
        ? TTF_RenderText_Blended_Wrapped(font, text, color, wrapWidth)
        : TTF_RenderText_Solid(font, text, color);

    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect rect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *current, const char *future, const char *past, int textWidth) {
            drawText(renderer, font, "Press Q or Escape to Exit",          50,        50,  white, 0);
    drawText(renderer, font, "The current date is: ",              textWidth, 50,  white, 305);
      drawText(renderer, font, "Simulation End Date:", textWidth, 320, white, 305);
       drawText(renderer, font, "Simulation Start Date:", textWidth, 590, white, 305);
    drawText(renderer, font, current,                              textWidth, 185, white, 0);
    drawText(renderer, font, future,                               textWidth, 455, white, 0);
    drawText(renderer, font, past,                                 textWidth, 725, white, 0);
}

void simulatedDate(SDL_Renderer *renderer, TTF_Font *font, const char *simulated, int textWidth) {
    drawText(renderer, font, simulated,                        textWidth, 995, white, 0);
}





















