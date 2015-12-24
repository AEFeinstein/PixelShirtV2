/* Copyright 2015 Adam Feinstein
 *
 * This file is part of PixelShirtV2.
 *
 * PixelShirtV2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PixelShirtV2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with PixelShirtV2.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "ArduinoGame.h"

#include "Pong.h"
#include "Lightcycle.h"
#include "Tetris.h"
#include "SuperSquare.h"
#include "Shooter.h"
#include "ScreenSaver.h"
#include "ScoreDisplay.h"

#include "PlatformSpecific.h"
#include "PixelShirtV2.h"

/* Manage the Games */
#define NUM_GAMES 5

/* The different game IDs */
#define SUPER_SQUARE 0
#define PONG 1
#define LIGHTCYCLE 2
#define SHOOTER 3
#define TETRIS 4

/* The current game's ID */
uint8_t gameMode;

/* A timer for game switching. It counts how long both "up" buttons are
 * pressed
 */
uint8_t gameSwitchTimer;

/* A pointer to the active game */
ArduinoGame* currentGame;

/* The collection of games */
SuperSquare superSquare;
Pong pong;
Lightcycle lightcycle;
Shooter shooter;
Tetris tetris;

/* To periodically call the main function in the loop */
uint32_t microsCnt = 0;
uint32_t microsLast = 0;

/* Variables to store controller state */
uint32_t p1controller;
uint32_t p2controller;

/* Prototypes */
void doEverything(void);

/**
 * The setup function. This is called once before loop() is called infinitely.
 * It sets up the hardware interfaces for driving LEDs and receiving controller
 * data. It also sets up the RNG, the initial game, and resets all state
 * variables.
 */
void setup(void)
{
  initializeHardware();

  /* Clear Variables */
  uint8_t x, y;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }
  gameSwitchTimer = 0;

  /* Set the current game */
  gameMode = LIGHTCYCLE;
  currentGame = &lightcycle;
  currentGame->ResetGame(1,0);

  /* Initialize controller state */
  p1controller = 0;
  p2controller = 0;
  SET_X_AXIS(p1controller, 512);
  SET_Y_AXIS(p1controller, 512);
  SET_X_AXIS(p2controller, 512);
  SET_Y_AXIS(p2controller, 512);
}

/**
 * This function is called within an infinite loop. It calls doEverything() at
 * 32Hz and is always listening for controller input data. It also blinks
 * a heart-beat LED.
 */
void loop(void)
{
  uint32_t currentTime = getMicroseconds();

  /* Run the code at 32Hz (every 31250 microseconds)
   * Only add to the counter if it doesn't roll over
   */
  if(currentTime > microsLast) {
    microsCnt += (currentTime - microsLast);
  }
  if(microsCnt >= 31250) {
    microsCnt = 0;
    doEverything();
  }
  microsLast = currentTime;

  readJoystickData();
}

/**
 * This is called at 32Hz. It handles displaying the screen saver, switching
 * between game modes, sending controller input and update physics requests
 * to the active game, and actually pushing the pixel data out to the display.
 */
void doEverything(void)
{
  /* If both up buttons are held, maybe the game mode is being changed */
  if((GET_BUTTONS(p1controller) & UP) && (GET_BUTTONS(p2controller) & UP)) {
    gameSwitchTimer++;
  }
  else {
    gameSwitchTimer = 0;
  }

  /* If there is any input, exit the screen saver */
  if((GET_BUTTONS(p1controller) & UP) ||
      (GET_BUTTONS(p1controller) & DOWN) ||
      (GET_BUTTONS(p1controller) & LEFT) ||
      (GET_BUTTONS(p1controller) & RIGHT) ||
      ((GET_X_AXIS(p1controller)) < 512 - DEAD_ZONE ||
       512 + DEAD_ZONE < (GET_X_AXIS(p1controller))) ||
      ((GET_Y_AXIS(p1controller)) < 512 - DEAD_ZONE ||
       512 + DEAD_ZONE < (GET_Y_AXIS(p1controller))) ||
      (GET_BUTTONS(p2controller) & UP) ||
      (GET_BUTTONS(p2controller) & DOWN) ||
      (GET_BUTTONS(p2controller) & LEFT) ||
      (GET_BUTTONS(p2controller) & RIGHT) ||
      ((GET_X_AXIS(p2controller)) < 512 - DEAD_ZONE ||
       512 + DEAD_ZONE < (GET_X_AXIS(p2controller))) ||
      ((GET_Y_AXIS(p2controller)) < 512 - DEAD_ZONE ||
       512 + DEAD_ZONE < (GET_Y_AXIS(p2controller)))) {
    ExitScreensaver(currentGame);
  }

  /* Decrement the screen saver timer */
  HandleScreensaverTimer();

  if(GetScreensaverTimer() == 0) {
    DisplayScreensaver();
  }
  else {
    /* The game mode is being changed */
    if(gameSwitchTimer == IRQ_HZ * 2) {
      gameSwitchTimer = 0;
      switchGame();
    }
    else {
      /* Process new controller input */
      currentGame->ProcessInput(p1controller, p2controller);

      /* Update the game state */
      currentGame->UpdatePhysics();
    }
  }

  /* Display the frame */
  displayPixels();
}

/**
 * Cycles through the available games.
 * Switches to the next one & resets it
 */
void switchGame(void) {
  gameMode = (gameMode+1)%NUM_GAMES;
  switch(gameMode) {
    case PONG: {
        currentGame = &pong;
        break;
      }
    case LIGHTCYCLE: {
        currentGame = &lightcycle;
        break;
      }
    case TETRIS: {
        currentGame = &tetris;
        break;
      }
    case SUPER_SQUARE: {
        currentGame = &superSquare;
        break;
      }
    case SHOOTER: {
        currentGame = &shooter;
        break;
      }
  }
  currentGame->ResetGame(1,0);
}