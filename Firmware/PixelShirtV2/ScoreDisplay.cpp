/*
 * ScoreDisplay.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: adam
 */

#include <stdint.h>
#include "PlatformSpecific.h"
#include "ArduinoGame.h"
#include "ScoreDisplay.h"

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
