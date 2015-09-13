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

#include "ArduinoGame.h"

#include "Pong.h"
#include "Lightcycle.h"
#include "Tetris.h"
#include "SuperSquare.h"
#include "Shooter.h"
#include "ScreenSaver.h"

#include "Adafruit_NeoPixel.h"
#include "nrf24.h"

/* Pixel strip pin connections */
#define NP_PIN_0            11
#define NP_PIN_1            12
#define NP_PIN_2            8
#define NP_PIN_3            9

/* Numbers of pixels per strip */
#define NUMPIXELS      64

/* Neopixel objects */
Adafruit_NeoPixel pixels0 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_0,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_1,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_2,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels3 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_3,
                            NEO_GRB + NEO_KHZ800);

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

/* The LED pin for the heart-beat blink */
#define HEARTBEAT_PIN 13

/* Addresses to talk to the wireless controllers. Hard-coded in the controller
 * firmware too
 */
uint8_t tx_address[5] = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7};
uint8_t rx_address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

/* Variables to store controller state */
uint32_t p1controller;
uint32_t p2controller;

void doEverything(void);

/**
 * The setup function. This is called once before loop() is called infinitely.
 * It sets up the hardware interfaces for driving LEDs and receiving controller
 * data. It also sets up the RNG, the initial game, and resets all state
 * variables.
 */
void setup(void)
{
  /* Set up heart-beat */
  pinMode(HEARTBEAT_PIN, OUTPUT);

  /* Seed the RNG */
  randomSeed(analogRead(RANDOM_PIN));

  /* Initialize the NeoPixel library. */
  pixels0.begin();
  pixels1.begin();
  pixels2.begin();
  pixels3.begin();

  /* Clear Variables */
  uint8_t x, y;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }
  gameSwitchTimer = 0;

  /* Set the current game */
  gameMode = PONG;
  currentGame = &pong;
  currentGame->ResetGame(1,0);

  /* Initialize hardware pins */
  nrf24_init();

  /* Channel #2 , payload length: 5 */
  nrf24_config(2, 4);

  /* Set the device addresses */
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);

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
  uint32_t jsTmp;
  uint32_t currentTime = micros();

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

  /* If there is data from the controllers, get it and store it */
  if (nrf24_dataReady()) {
    nrf24_getData((uint8_t*)&jsTmp);
    switch(GET_PLAYER(jsTmp)) {
      case 0: {
          p1controller = jsTmp;
          break;
        }
      case 1: {
          p2controller = jsTmp;
          break;
        }
    }
  }

  /* Heart-beat */
  if((millis() / 1000) % 2 == 0) {
    digitalWrite(HEARTBEAT_PIN, HIGH);
  }
  else {
    digitalWrite(HEARTBEAT_PIN, LOW);
  }
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
    else {
      /* Process new controller input */
      currentGame->ProcessInput(p1controller, p2controller);

      /* Update the game state */
      currentGame->UpdatePhysics();
    }
  }

  /* Display the frame */
  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
}

/**
 * Draws an up to four digit number in the center of the display
 * with the given color
 *
 * @param score The score to draw, 0 to 9999
 * @param rgb The color to draw the score with
 */
void DisplayScore(uint16_t score, uint32_t rgb)
{
  uint8_t i, j;
  for(i = 0; i < BOARD_SIZE; i++) {
    for(j = 0; j < BOARD_SIZE; j++) {
      SetPixel(i, j, EMPTY_COLOR);
    }
  }

  /* draw "score" on the field */
  if(score > 999) {
    DrawNumber((score / 1000) % 10, 0 , 5, rgb);
  }
  if(score > 99) {
    DrawNumber((score / 100 ) % 10, 4 , 5, rgb);
  }
  if(score > 9) {
    DrawNumber((score / 10  ) % 10, 8 , 5, rgb);
  }
  DrawNumber((score / 1   ) % 10, 12, 5, rgb);
}

/**
 * Draws a single digit on the display in the given color at the given offset
 *
 * @param number  The number to draw, 0-9
 * @param offsetX The X offset where to draw the number
 * @param offsetY The Y offset where to draw the number
 * @param rgb   The color to draw the number with
 */
void DrawNumber(uint8_t number, uint8_t offsetX, uint8_t offsetY, uint32_t rgb)
{
  switch(number) {
    case 0: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 1: {
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 2: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 3: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 4: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 5: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 6: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 7: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 3, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        break;
      }
    case 8: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 9: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
  }
}

/**
 * Sets a pixel in the display at the given position to the given color
 *
 * @param x   The X coordinate of the pixel to set
 * @param y   The Y coordinate of the pixel to set
 * @param rgb The color to set the pixel to
 */
void SetPixel(int8_t x, int8_t y, uint32_t rgb)
{
  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0: {
        pixels0.setPixelColor(index, rgb);
        break;
      }
    case 1: {
        pixels1.setPixelColor(index, rgb);
        break;
      }
    case 2: {
        pixels2.setPixelColor(index, rgb);
        break;
      }
    case 3: {
        pixels3.setPixelColor(index, rgb);
        break;
      }
  }
}

/**
 * Gets a pixel in the display at the given position and returns the color
 *
 * @param x   The X coordinate of the pixel to get
 * @param y   The Y coordinate of the pixel to get
 * @return    The color of the given pixel
 */
uint32_t GetPixel(int8_t x, int8_t y)
{
  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0: {
        return pixels0.getPixelColor(index);
      }
    case 1: {
        return pixels1.getPixelColor(index);
      }
    case 2: {
        return pixels2.getPixelColor(index);
      }
    case 3: {
        return pixels3.getPixelColor(index);
      }
    default: {
        return 0;
      }
  }
}
