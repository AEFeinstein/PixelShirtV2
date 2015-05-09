#include "ArduinoGame.h"
#include "ScreenSaver.h"

#define SQUARE_WAVE_SIZE  6
uint16_t screensaverTimer = 1;
uint8_t squareWavePoint = 0;
uint8_t colorStep = 0;
#define RED   0
#define GREEN 1
#define BLUE  2
uint8_t getIntensity(uint8_t step, uint8_t color);
uint8_t screensaverShiftTimer = IRQ_HZ / 4;

void ExitScreensaver(ArduinoGame * currentGame) {
  if (screensaverTimer == 0) {
    /* Leave the screensaver, clear the field & reset the game */
    uint8_t x, y;
    for (x = 0; x < BOARD_SIZE; x++) {
      for (y = 0; y < BOARD_SIZE; y++) {
        SetPixel(x, y, 0, 0, 0);
      }
    }
    currentGame->ResetGame(1, 0);
  }
  screensaverTimer = (IRQ_HZ * 30);
}

void HandleScreensaverTimer() {
  if (screensaverTimer > 0) {
    screensaverTimer--;
    if (screensaverTimer == 0) {
      /* Enter the screensaver, clear the field */
      uint8_t x, y;
      for (x = 0; x < BOARD_SIZE; x++) {
        for (y = 0; y < BOARD_SIZE; y++) {
          SetPixel(x, y, 0, 0, 0);
        }
      }
    }
  }
}

uint16_t GetScreensaverTimer() {
  return screensaverTimer;
}

void DisplayScreensaver() {
  /* display screensaver */

  if (screensaverShiftTimer > 0) {
    screensaverShiftTimer--;
  }
  if (screensaverShiftTimer == 0) {
    screensaverShiftTimer = IRQ_HZ / 4;

    /* Shift leftward */
    uint8_t x, y;
    for (x = 0; x < BOARD_SIZE - 1; x++) {
      for (y = 0; y < BOARD_SIZE; y++) {
        SetPixel(y, x, GetPixel(y, x + 1));
      }
    }

    /* Clear rightmost column */
    for (y = 0; y < BOARD_SIZE; y++) {
      SetPixel(BOARD_SIZE - 1, y, 0, 0, 0);
    }

    /* Draw new rightmost column */
    if (squareWavePoint < SQUARE_WAVE_SIZE - 2) {
      /* top bar */
      SetPixel(BOARD_SIZE - 1, (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2,
               getIntensity(colorStep, RED), getIntensity(colorStep, GREEN),
               getIntensity(colorStep, BLUE));
    }
    else if (squareWavePoint == SQUARE_WAVE_SIZE - 2) {
      /* falling edge */
      for (y = (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2;
           y <= ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1; y++) {
        SetPixel(BOARD_SIZE - 1, y, getIntensity(colorStep, RED),
                 getIntensity(colorStep, GREEN), getIntensity(colorStep, BLUE));
      }
    }
    else if (squareWavePoint < (SQUARE_WAVE_SIZE * 2) - 3) {
      /* bottom bar */
      SetPixel(BOARD_SIZE - 1, ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1,
               getIntensity(colorStep, RED), getIntensity(colorStep, GREEN),
               getIntensity(colorStep, BLUE));
    }
    else if (squareWavePoint == (SQUARE_WAVE_SIZE * 2) - 3) {
      /* rising edge */
      for (y = (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2;
           y <= ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1; y++) {
        SetPixel(BOARD_SIZE - 1, y, getIntensity(colorStep, RED),
                 getIntensity(colorStep, GREEN), getIntensity(colorStep, BLUE));
      }
    }
    squareWavePoint = (squareWavePoint + 1) % ((SQUARE_WAVE_SIZE - 1) * 2);
    colorStep = (colorStep + 1) % 24;
  }
}

uint8_t getIntensity(uint8_t step, uint8_t color)
{
  switch (color) {
    case RED:
      if (step < 8) {
        /* rise */
        return (0x01 << (step));
      }
      else if (step < 16) {
        /* fall */
        return (0x80 >> (step - 8));
      }
      return 0;
    case GREEN:
      if (step >= 16) {
        /* fall */
        return (0x80 >> (step - 16));
      }
      else if (step >= 8) {
        /* rise */
        return (0x01 << (step - 8));
      }
      return 0;
    case BLUE:
      if (step < 8) {
        /* fall */
        return (0x80 >> step);
      }
      else if (step >= 16) {
        /* rise */
        return (0x01 << (step - 16));
      }
      return 0;
  }
  return 0;
}