#include "main.h"

void CircleFunction(SDL_Renderer *render, int x, int y, int r,
                    SDL_Color color) {
  SDL_SetRenderDrawColor(render, color.r, color.g, color.b, 255);
  for (int i = -r; i <= r; i++) {
    for (int j = -r; j <= r; j++) {
      if (i * i + j * j <= r * r) {
        SDL_RenderDrawPoint(render, x + i, j + y);
      }
    }
  }
}

void clearPrev(SDL_Renderer *renderer, TTF_Font *font, const char *current,
               const char *future, const char *past, int textWidth) {
  int horizontalLines[] = {270, 540, 810};
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_RenderDrawLine(renderer, drawWidth, 1, drawWidth, HEIGHT);

  for (int i = 0; i < 3; i++) {
    SDL_RenderDrawLine(renderer, drawWidth, horizontalLines[i], WIDTH,
                       horizontalLines[i]);
  }
  CircleFunction(renderer, center[0], center[1], 25,
                 (SDL_Color){255, 255, 0, 255});
  RenderText(renderer, font, current, future, past, textWidth);

  return;
}

void planetDraw(SDL_Renderer *render, Planet p, int x, int y) {

  int cx = x + cos(p.angle) * p.distanceFromSun;
  int cy = y - sin(p.angle) * p.distanceFromSun;

  CircleFunction(render, cx, cy, p.size, p.color);
}

int openSerial() {
  int serial_port;

  if ((serial_port = serialOpen("/dev/ttyACM0", 115200)) < 0) {
    return 1;
  }
  if (wiringPiSetup() == -1) {
    return 1;
  }

  return serial_port;
}

int InputThread(void *data) {
  SDL_Event event;
  volatile bool *running = (volatile bool *)data;
  while (*running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        *running = false;
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_q ||
            event.key.keysym.sym == SDLK_ESCAPE) {
          *running = false;
        }
      }
    }

  }

  return 0;
}

int serialRead(void *data) {
  int serial_port = *(int *)data;

  char buf[100];
  int bufIndex = 0;
  int tokenCount = 0;

  while (running) {
    if (!serialDataAvail(serial_port)) {
      SDL_Delay(1);
      continue;
    }

    bufIndex = 0;
    tokenCount = 0;
    while (serialDataAvail(serial_port) > 0) {
      char c = serialGetchar(serial_port);
      if (c == '\n')
        break;
      buf[bufIndex++] = c;
    }
    buf[bufIndex] = '\0';
    char *tokens[20];

    char *token;

    token = strtok(buf, "!");

    while (token != NULL) {
      tokens[tokenCount] = token;
      tokenCount++;
      token = strtok(NULL, "!");
    }
    if (tokenCount >= 2) {
      if (tokenCount == 3) {
        button = true;
      }

      int pastDays = atoi(tokens[0]);
      int futureDays = atoi(tokens[1]);

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

void planetUpdate(SDL_Renderer *renderer, Planet planets[], int planetCount,
                  long long day, int centerX, int centerY) {
  for (int i = 0; i < planetCount; i++) {
    double angularVel = (2.0 * PI) / planets[i].speed;
    double angleCalc = fmod(
        planets[i].baseAngle + day * angularVel * SPEED_MULTIPLIER, 2 * PI);
    if (angleCalc < 0) {
      angleCalc += 2.0 * PI;
    }
    planets[i].angle = angleCalc;

    planetDraw(renderer, planets[i], centerX, centerY);
  }
  return;
}

int main() {
  char current[25];
  char future[25];
  char past[25];
  char simulated[25];
  int plusDays = 0;
  int minusDays = 0;
  center[0] = drawWidth / 2;
  center[1] = HEIGHT / 2;

  int serial_port = openSerial();
  if (serial_port == 1) {
    return 1;
  }


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

  inputsMTX = SDL_CreateMutex();
  SDL_Thread *serialID =
      SDL_CreateThread(serialRead, "SerialReading", (void *)&serial_port);

  time_t currentDate;
  time(&currentDate);
  long long timeSinceEpoch = (long long)currentDate - 946684800;
  long long timeSinceLinuxEpoch = (long long)currentDate;
  int daysSinceLinEpoch = timeSinceLinuxEpoch / SECONDS_IN_DAY;
  int daysSinceEpoch = timeSinceEpoch / SECONDS_IN_DAY;

  TTF_Font *font = TTF_OpenFont(MY_FONT, 24);

  SDL_Thread *threadID =
      SDL_CreateThread(InputThread, "Listener", (void *)&running);

  while (running) {

    int pastDayRead, futureDayRead;

    SDL_LockMutex(inputsMTX);
    pastDayRead = sharedInputs.minusDays;
    futureDayRead = sharedInputs.plusDays;
    SDL_UnlockMutex(inputsMTX);

    minusDays = pastDayRead;
    plusDays = futureDayRead;

    timeCalculation(currentDate, current, future, past, plusDays, minusDays);

    clearPrev(renderer, font, current, future, past, textWidth);

    RenderText(renderer, font, current, future, past, textWidth);

    if (button == true) {
      int start = -minusDays;
      int finish = +plusDays;

      while (start < finish) {

        long long simulatedDay = daysSinceLinEpoch + start;
        long long simulatedSeconds = simulatedDay * SECONDS_IN_DAY;

        clearPrev(renderer, font, current, future, past, textWidth);
        formatDate(simulatedSeconds, simulated);
        simulatedDate(renderer, font, simulated, textWidth);

        planetUpdate(renderer, planets, PLANETS, simulatedDay, center[0],
                     center[1]);
        if (!running) {
          start = finish;
          break;
        }

        start++;

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
      }

      tcflush(serial_port, TCIOFLUSH);

      button = false;
    } else if (button == false) {
      clearPrev(renderer, font, current, future, past, textWidth);
      planetUpdate(renderer, planets, PLANETS, daysSinceEpoch, center[0],
                   center[1]);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(1);
  }

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
