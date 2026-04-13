#include "render_text.h"
#include "time_calc.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define PLANETS 8
#define PI 3.1415926535897932384626433832795
#define SPEED_MULTIPLIER 1.0
#define DEG_TO_RAD (PI /180)
#define daysinYear 365
#define LINEPOCH_J200 946684800 // conversion of linux epoch to j2000 epoch for planet calcs
#define MY_FONT "/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf"
#define WIDTH 1920
#define HEIGHT 1080

volatile bool running = true;
int drawHeight = 0;

typedef struct {
  int plusDays; 
  int minusDays; 
} inputs; 

inputs sharedInputs = {0, 0}; 

SDL_mutex *inputsMTX = NULL; 

const int drawWidth = (WIDTH - (WIDTH / 6));

const int textWidth = drawWidth + 10;

int plusCalc, minusCalc;
bool button = false;

int center[2];

typedef struct {
  const char *name;
  double distanceFromSun;
  double angle;
  double speed;
  int size;
  SDL_Color color;
  double baseAngle; 
} Planet;

Planet planets[] = {
    
    {"Mercury", 60, 0, 87.97, 4, {183, 184, 185, 255}, 252.25 *DEG_TO_RAD},
    {"Venus", 90, 0, 224.70, 6, {248, 226, 176, 255},181.98*DEG_TO_RAD},
    {"Earth", 130, 0, 365.25, 6, {0, 100, 255, 255}, 100.46*DEG_TO_RAD},
    {"Mars", 170, 0, 686.98, 5, {173, 98, 60, 255}, 355.43 *DEG_TO_RAD},
    {"Jupiter", 240, 0, 4332.82, 14, {227, 220, 209, 255}, 34.35*DEG_TO_RAD},
    {"Saturn", 310, 0, 10755.70, 12, {206, 184, 184, 255}, 50.09*DEG_TO_RAD},
    {"Uranus", 380, 0, 30687.15, 6.83, {175, 229, 238, 255}, 314.06*DEG_TO_RAD}, //314.06 , 9
    {"Neptune", 450, 0, 60190.03, 9, {91, 93, 223, 255}, 304.35*DEG_TO_RAD}
    
};
