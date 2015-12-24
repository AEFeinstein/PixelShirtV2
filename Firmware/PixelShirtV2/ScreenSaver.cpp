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
#include "PlatformSpecific.h"
#include "ArduinoGame.h"
#include "ScreenSaver.h"

#define SQUARE_WAVE_SIZE  6
uint16_t screensaverTimer = 1;
uint8_t squareWavePoint = 0;
uint16_t colorStep = 0;
#define RED   0
#define GREEN 1
#define BLUE  2
uint32_t getRGB(uint16_t polar, uint8_t brightness);
uint8_t screensaverShiftTimer = IRQ_HZ / 4;

/**
 * If the screensaver is active, it clears the screen, leaves the screensaver,
 * and resets the current game. Otherwise it extends the time until the
 * screensaver activates.
 *
 * @param currentGame The game to restart
 */
void ExitScreensaver(ArduinoGame* currentGame)
{
  if (screensaverTimer == 0) {
    /* Leave the screensaver, clear the field & reset the game */
    uint8_t x, y;
    for (x = 0; x < BOARD_SIZE; x++) {
      for (y = 0; y < BOARD_SIZE; y++) {
        SetPixel(x, y, EMPTY_COLOR);
      }
    }
    currentGame->ResetGame(1, 0);
  }
  screensaverTimer = (IRQ_HZ * 30);
}

/**
 * Decrements the screensaver timer, and enters the screensaver if it hits zero
 */
void HandleScreensaverTimer(void)
{
  if (screensaverTimer > 0) {
    screensaverTimer--;
    if (screensaverTimer == 0) {
      /* Enter the screensaver, clear the field */
      uint8_t x, y;
      for (x = 0; x < BOARD_SIZE; x++) {
        for (y = 0; y < BOARD_SIZE; y++) {
          SetPixel(x, y, EMPTY_COLOR);
        }
      }
    }
  }
}

/**
 * @return  The time until the screensaver activates, without controller input
 */
uint16_t GetScreensaverTimer(void)
{
  return screensaverTimer;
}

/**
 * Draws the screensaver, a scrolling square wave which smoothly changes color
 */
void DisplayScreensaver(void)
{
  /* display screensaver */

  if (screensaverShiftTimer > 0) {
    screensaverShiftTimer--;
  }
  if (screensaverShiftTimer == 0) {
    screensaverShiftTimer = IRQ_HZ / 8;

    /* Shift leftward */
    uint8_t x, y;
    for (x = 0; x < BOARD_SIZE - 1; x++) {
      for (y = 0; y < BOARD_SIZE; y++) {
        SetPixel(x, y, GetPixel(x + 1, y));
      }
    }

    /* Clear rightmost column */
    for (y = 0; y < BOARD_SIZE; y++) {
      SetPixel(BOARD_SIZE - 1, y, EMPTY_COLOR);
    }

    /* Draw new rightmost column */
    if (squareWavePoint < SQUARE_WAVE_SIZE - 2) {
      /* top bar */
      SetPixel(BOARD_SIZE - 1, (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2,
               getRGB(colorStep, 0x40));
    }
    else if (squareWavePoint == SQUARE_WAVE_SIZE - 2) {
      /* falling edge */
      for (y = (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2;
           y <= ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1; y++) {
        SetPixel(BOARD_SIZE - 1, y, getRGB(colorStep, 0x40));
      }
    }
    else if (squareWavePoint < (SQUARE_WAVE_SIZE * 2) - 3) {
      /* bottom bar */
      SetPixel(BOARD_SIZE - 1, ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1,
               getRGB(colorStep, 0x40));
    }
    else if (squareWavePoint == (SQUARE_WAVE_SIZE * 2) - 3) {
      /* rising edge */
      for (y = (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2;
           y <= ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1; y++) {
        SetPixel(BOARD_SIZE - 1, y, getRGB(colorStep, 0x40));
      }
    }
    squareWavePoint = (squareWavePoint + 1) % ((SQUARE_WAVE_SIZE - 1) * 2);
    colorStep = (colorStep + 2) % 360;
  }
}

/**
 * Given a position on the color wheel, and a brightness, return the color
 * at that position
 *
 * @param polar     The position on the color wheel, in degrees
 * @param brightness  The brightness of the color to return
 * @return        A color on the color wheel
 */
uint32_t getRGB(uint16_t polar, uint8_t brightness)
{
  uint32_t r = 0, g = 0, b = 0;

  polar = polar % 360;

  if (300 <= polar || polar <= 60) {
    r = 0xFF;
  }
  else if (60 < polar && polar < 120) {
    /* down slope */
    r = ((120 - polar) * 0xFF) / 60;
  }
  else if (120 <= polar && polar <= 240) {
    r = 0;
  }
  else if (240 < polar && polar < 300) {
    /* up slope */
    r = ((polar - 240) * 0xFF) / 60;
  }

  if (0 < polar && polar < 60) {
    /* up slope */
    g = ((polar) * 0xFF) / 60;
  }
  else if (60 <= polar && polar <= 180) {
    g = 0xFF;
  }
  else if (180 < polar && polar < 240) {
    /* down slope */
    g = ((240 - polar) * 0xFF) / 60;
  }
  else if ((240 <= polar && polar <= 360) || polar == 0) {
    g = 0;
  }

  if (polar <= 120) {
    b = 0;
  }
  else if (120 < polar && polar < 180) {
    /* up slope */
    b = ((polar - 120) * 0xFF) / 60;
  }
  else if (180 <= polar && polar <= 300) {
    b = 0xFF;
  }
  else if (300 < polar && polar < 360) {
    /* down slope */
    b = ((360 - polar) * 0xFF) / 60;
  }

  /* Normalize brightness */
  uint32_t sum = r + g + b;
  r = (((r * brightness)) / sum);
  g = (((g * brightness)) / sum);
  b = (((b * brightness)) / sum);
  return (r << 16) | (g << 8) | (b);
}
