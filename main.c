/* File: main.c 
 * Author: K.McGregor, M.Underwood, E.Ford
 * Date: 2026/04/13
 * Description: This file contains the main program for running the solar planetarium simulation. 
 */


#include "main.h"


/* function: CircleFunction
 * description: function that draws circles in relation to x,y placement on the screen with radius and color 
 * @param: SDL renderer,
 * @param: x coordinate of circle 
 * @param: y coordinate of circle
 * @param: radius of circle 
 * @return: void 
 * */
void CircleFunction(SDL_Renderer *render, int x, int y, int r,
                    SDL_Color color) {
  SDL_SetRenderDrawColor(render, color.r, color.g, color.b, 255); // set color 
  /*draw circle*/
  for (int i = -r; i <= r; i++) {
    for (int j = -r; j <= r; j++) {
      if (i * i + j * j <= r * r) {
        SDL_RenderDrawPoint(render, x + i, j + y);
      }
    }
  }
}


/* function: redraw
 * description: function that clears the screen and renders it again with new information 
 * @param SDL renderer, 
 * @param: font, 
 * @param: the current date, 
 * @param: the simulation future date, 
 * @param: simulation past date, 
 * @param: text width 
 * @return: void 
 * */
void redraw(SDL_Renderer *renderer, TTF_Font *font, const char *current,
               const char *future, const char *past, int textWidth) {
                 
  int horizontalLines[] = {270, 540, 810};
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer); // clears screen 

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // draw color to white 

  SDL_RenderDrawLine(renderer, drawWidth, 1, drawWidth, HEIGHT); // draw screen sidebars

  /*draw number of horizontal lines on side bar*/ 
  for (int i = 0; i < 3; i++) {
    SDL_RenderDrawLine(renderer, drawWidth, horizontalLines[i], WIDTH,
                       horizontalLines[i]);
  }
  /*draw the sun: yellow cicle in centre*/ 
  CircleFunction(renderer, center[0], center[1], 25,
                 (SDL_Color){255, 255, 0, 255});
  RenderText(renderer, font, current, future, past, textWidth); // render text

  return;
}


/* function: planetDraw
 * description: calculates the position of the planet and sends it to function to be drawn 
 * @param SDL renderer, 
 * @param: planet array, 
 * @param: x coordinate of planet
 * @param: y coordinate of planet
 * @return: void
 * */
void planetDraw(SDL_Renderer *render, Planet p, int x, int y) {

  int cx = x + cos(p.angle) * p.distanceFromSun; // calculates horizontal (left to right) movement of planet 
  int cy = y - sin(p.angle) * p.distanceFromSun; // calculates vertical (top to bottom) movement of planet 

  CircleFunction(render, cx, cy, p.size, p.color); // draws the planet with new location 
  
  return; 
}


/* function: openSerial 
 * description: connects to the serial port
 * @param void
 * @return: void 
 * */
int openSerial() {
  int serial_port;

  if ((serial_port = serialOpen("/dev/ttyACM0", BAUDRATE)) < 0) {
    return 1; // did not open port 
  }
  if (wiringPiSetup() == -1) {
    return 1; //wiringPi failed 
  }

  return serial_port;
}


/* function: InputThread
 * description: thread to interpret keyboard presses 
 * @param: boolean that controls program running 
 * @return: integer to indicate thread closure 
 * */
int InputThread(void *data) {
  SDL_Event event; 
  volatile bool *running = (volatile bool *)data; // boolean that stops program 
  while (*running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) { // window closed 
        *running = false; // stop program
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_q ||
            event.key.keysym.sym == SDLK_ESCAPE) { //if q or escape pressed
          *running = false; //stop program 
        }
      }
    }

  }

  return 0; // close thread
}

/* function: serialRead
 * description: thread to read the serial input from the arduino 
 * @param: serial port identifier
 * @return: integer to indicate thread state 
 * */
int serialRead(void *data) {
  int serial_port = *(int *)data; //serial port identifier

  char buf[100]; //buffer for serial inputs 
  int bufIndex = 0;
  int tokenCount = 0;

  while (running) { // is the program still running 
    if (!serialDataAvail(serial_port)) { // no serial data available
      SDL_Delay(1);
      continue;
    }

    bufIndex = 0;
    tokenCount = 0;
    while (serialDataAvail(serial_port) > 0) {
      char c = serialGetchar(serial_port); //gather serial data into buffer
      if (c == '\n')
        break;
      buf[bufIndex++] = c;
    }
    buf[bufIndex] = '\0'; // null terminate buffer array 
    char *tokens[20];

    char *token;

    token = strtok(buf, "!"); // parse buffer by exclamation points

    while (token != NULL) {
      tokens[tokenCount] = token;
      tokenCount++;
      token = strtok(NULL, "!");
    }
    if (tokenCount >= 2) { // 2 tokens = past and future date 
      if (tokenCount == 3) { // 3 tokens = past date, future date, start button 
        button = true; //indicate simulation should run 
      }
      //convert readings to integers
      int pastDays = atoi(tokens[0]);
      int futureDays = atoi(tokens[1]);
      
      //access mutex for updating past and future date 
      SDL_LockMutex(inputsMTX);
      sharedInputs.minusDays = pastDays;
      sharedInputs.plusDays = futureDays;
      SDL_UnlockMutex(inputsMTX);
    }

    tcflush(serial_port, TCIOFLUSH);
    tokenCount = 0;
  }
  return 0;
}


/* function: planetUpdate
 * description: update the position of the planets
 * @param: SDL renderer, 
 * @param: planets array, 
 * @param: # of planets, 
 * @param: date of display, 
 * @param: center point of planet
 * @return: void
 * */
void planetUpdate(SDL_Renderer *renderer, Planet planets[], int planetCount,
                  long long day, int centerX, int centerY) {
  for (int i = 0; i < planetCount; i++) {
    double angularVel = (2.0 * PI) / planets[i].speed; //calculate angular velocity of planet 
    double angleCalc = fmod(
        planets[i].baseAngle + day * angularVel * SPEED_MULTIPLIER, 2 * PI); // calculate rads of planet movement from base angle 
    if (angleCalc < 0) { //if rads is less than 0, add 2pi
      angleCalc += 2.0 * PI;
    }
    planets[i].angle = angleCalc; //update planet angle for simulation date 

    planetDraw(renderer, planets[i], centerX, centerY); //draw planet in new location 
  }
  return;
}


/* function: main
 * description: runs the main program for the solar planetarium simulation 
 * @param void
 * @return: integer to indicate error or success 
 * */
int main() {
  char current[25]; //array for current date 
  char future[25]; //array for future date 
  char past[25]; //array for past date 
  char simulated[25]; //array for simulated date
  int plusDays = 0; //variable for days in future 
  int minusDays = 0; // variable for days in past 
  center[0] = drawWidth / 2; //screen configuration 
  center[1] = HEIGHT / 2; //screen configuration 

  /* open serial communication*/
  int serial_port = openSerial();
  if (serial_port == 1) {
    return 1;
  }

  /*begin SDL program*/ 
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL Init Failed: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  
  SDL_Window *window = SDL_CreateWindow("Planetarium", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WIDTH,
                                        HEIGHT, // Window width and height.
                                        SDL_WINDOW_FULLSCREEN);

  if (TTF_Init() < 0) {
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

  /*create mutex for display date information*/ 
  inputsMTX = SDL_CreateMutex();
  
  /*create thread for reading serial inputs*/ 
  SDL_Thread *serialID =
      SDL_CreateThread(serialRead, "SerialReading", (void *)&serial_port);
  
  /*time calculations: date from J200 epoch, date from linux epoch*/ 
  time_t currentDate;
  time(&currentDate);
  long long timeSinceJ200Epoch = (long long)currentDate - LINEPOCH_J200; // calculate seconds since j2000 epoch 
  long long timeSinceLinuxEpoch = (long long)currentDate; // calculate seconds since linux epoch 
  int daysSinceLinEpoch = timeSinceLinuxEpoch / SECONDS_IN_DAY;
  int daysSinceEpoch = timeSinceJ200Epoch / SECONDS_IN_DAY;

  TTF_Font *font = TTF_OpenFont(MY_FONT, 24); // font for display 

  /*create thread for detecting quit key*/ 
  SDL_Thread *threadID =
      SDL_CreateThread(InputThread, "Listener", (void *)&running);

  while (running) { // running bool determined by InputThread 

    int pastDayRead, futureDayRead;

    /*access mutex for +/- date range*/ 
    SDL_LockMutex(inputsMTX);
    pastDayRead = sharedInputs.minusDays;
    futureDayRead = sharedInputs.plusDays;
    SDL_UnlockMutex(inputsMTX);

    minusDays = pastDayRead; // # of days in past
    plusDays = futureDayRead; // # of days in future 

    /*update display*/ 
    timeCalculation(currentDate, current, future, past, plusDays, minusDays);
    redraw(renderer, font, current, future, past, textWidth);
    RenderText(renderer, font, current, future, past, textWidth);

    /*run simulation*/ 
    if (button == true) {
      int start = -minusDays;
      int finish = +plusDays;

      while (start < finish) { // entire date range 

        long long simulatedDay = daysSinceLinEpoch + start;
        long long simulatedSeconds = simulatedDay * SECONDS_IN_DAY; // day to be displayed in simulation
        
        
        long long actualDate = daysSinceEpoch +start; // day to use for planet rendering (from J200) 

        /*redraw display with simulated date, new planet positions*/ 
        redraw(renderer, font, current, future, past, textWidth); 
        formatDate(simulatedSeconds, simulated);
        simulatedDate(renderer, font, simulated, textWidth);
        planetUpdate(renderer, planets, PLANETS, actualDate, center[0],
                     center[1]); 
                     
        if (!running) { // quit key has been pressed
          start = finish;
          break;
        }

        start++; // increment date in range 

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
      }

      tcflush(serial_port, TCIOFLUSH); 

      button = false; //simulation has been completed, reset button indication 
      
    } else if (button == false) { // redraw display of current date
      redraw(renderer, font, current, future, past, textWidth); 
      planetUpdate(renderer, planets, PLANETS, daysSinceEpoch, center[0],
                   center[1]);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(1);
  }
  /* close program completely */ 
  SDL_DestroyMutex(inputsMTX);
  SDL_WaitThread(serialID, NULL);
  SDL_WaitThread(threadID, NULL);
  tcflush(serial_port, TCIOFLUSH);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  TTF_CloseFont(font);
  TTF_Quit();

  return 0;
}
