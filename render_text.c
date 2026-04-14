/* File: render_text.c 
 * Author: K.McGregor, E.Ford, M.Underwood
 * Date: 2026/04/13
 * Description: This file contains functions for rendering text in the solar planetarium simulation. 
 */

#include "render_text.h"


 SDL_Color white = {255, 255, 255, 255}; // SDL code for white (used for white text) 


/* function: drawText 
 * description: renders a string at (x,y) 
 * @param: renderer, 
 * @param: font for string, 
 * @param: text for string, 
 * @param: x coordinate of text
 * @param: y coordinate of text
 * @param: text color, 
 * @param: text width
 * @return: void
 * */
// Internal helper — renders a single string at (x, y), no manual surface/texture management needed
static void drawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color, int wrapWidth) {
    SDL_Surface *surface = wrapWidth > 0 // is wrap width above zero 
        ? TTF_RenderText_Blended_Wrapped(font, text, color, wrapWidth) // wrap text
        : TTF_RenderText_Solid(font, text, color); // solid text

    if (!surface) return; 

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect rect = {x, y, surface->w, surface->h}; 
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface); 
}

/* function: RenderText
 * description: draw text on side of screen 
 * @param: renderer, 
 * @param: font for text, 
 * @param: current date, 
 * @param: future date, 
 * @param: past date, 
 * @param: text width 
 * @return: void
 * */
void RenderText(SDL_Renderer *renderer, TTF_Font *font, const char *current, const char *future, const char *past, int textWidth) {
    /*drawText(renderer, font, string to display, x coordinate of text, y coordinate of text, text color, wrap width of text) */ 
    drawText(renderer, font, "Press Q or Escape to Exit",          50,        50,  white, 0); // top left of screen 
    drawText(renderer, font, "The current date is: ",              textWidth, 50,  white, 305); //top right of screen 
    drawText(renderer, font, "Simulation End Date:", textWidth, 320, white, 305); // 2nd level down from top right of screen 
    drawText(renderer, font, "Simulation Start Date:", textWidth, 590, white, 305); // 3rd level down from top right of screen 
    drawText(renderer, font, current,                              textWidth, 185, white, 0); // current date (top right box) 
    drawText(renderer, font, future,                               textWidth, 455, white, 0); // future date (mid right box)
    drawText(renderer, font, past,                                 textWidth, 725, white, 0); // past date (2nd to bottom right box)
}

/* function: simulatedDate
 * description: writes the date being simulated in the bottom right box on screen 
 * @param: renderer, 
 * @param: font for text, 
 * @param: date of simulation, 
 * @param: text width 
 * @return: void
 * */
void simulatedDate(SDL_Renderer *renderer, TTF_Font *font, const char *simulated, int textWidth) {
    drawText(renderer, font, simulated,                        textWidth, 995, white, 0);
}





















