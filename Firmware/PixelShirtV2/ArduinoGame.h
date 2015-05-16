#ifndef _ARDUINO_GAME_H_
#define _ARDUINO_GAME_H_

#include <stdint.h>
#include "Arduino.h"

#define BOARD_SIZE  16 /* In pixels */
#define IRQ_HZ      32

#define FALSE 0
#define TRUE 1

#define X 0
#define Y 1

#define DEAD_ZONE 15

#define WRAP(x,y) ( ((x) < 0) ? ((x) + (y)) : ( ((x) >= (y)) ? ((x) - (y)) : (x) ) )
#define CLAMP(x,y) ((x) > (y) ? (y) : ((x) < 0 ? 0 : (x)))

#define RANDOM_PIN   4  // Analog, not connected

#define EMPTY_COLOR 0x000000
#define SCORE_COLOR 0x004000
#define P1_COLOR    0x000040
#define P2_COLOR    0x400000

/*
 * Lookup tables!
 */
static const int8_t cos32[360] = {
  32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 28, 28, 28,
  27, 27, 27, 27, 26, 26, 26, 25, 25, 25, 24, 24, 23, 23, 23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 16, 15, 15, 14, 14, 13, 13, 12,
  11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -6, -6, -7, -7, -8, -8, -9, -9, -10, -10, -11,
  -11, -12, -13, -13, -14, -14, -15, -15, -16, -16, -16, -17, -17, -18, -18, -19, -19, -20, -20, -21, -21, -21, -22, -22, -23, -23, -23, -24, -24, -25,
  -25, -25, -26, -26, -26, -27, -27, -27, -27, -28, -28, -28, -29, -29, -29, -29, -29, -30, -30, -30, -30, -30, -31, -31, -31, -31, -31, -31, -31, -32,
  -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -31, -31, -31, -31, -31, -31, -31, -30, -30, -30,
  -30, -30, -29, -29, -29, -29, -29, -28, -28, -28, -27, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -23, -23, -23, -22, -22, -21, -21, -21,
  -20, -20, -19, -19, -18, -18, -17, -17, -16, -16, -16, -15, -15, -14, -14, -13, -13, -12, -11, -11, -10, -10, -9, -9, -8, -8, -7, -7, -6, -6, -5, -4,
  -4, -3, -3, -2, -2, -1, -1, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 13, 13, 14, 14, 15, 15, 16, 16, 16, 17, 17, 18,
  18, 19, 19, 20, 20, 21, 21, 21, 22, 22, 23, 23, 23, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30,
  31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
};
static const int8_t sin32[360] = {
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 13, 13, 14, 14, 15, 15, 16, 16, 16, 17, 17, 18, 18,
  19, 19, 20, 20, 21, 21, 21, 22, 22, 23, 23, 23, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31,
  31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30,
  30, 29, 29, 29, 29, 29, 28, 28, 28, 27, 27, 27, 27, 26, 26, 26, 25, 25, 25, 24, 24, 23, 23, 23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
  16, 16, 15, 15, 14, 14, 13, 13, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -6, -6,
  -7, -7, -8, -8, -9, -9, -10, -10, -11, -11, -12, -13, -13, -14, -14, -15, -15, -16, -16, -16, -17, -17, -18, -18, -19, -19, -20, -20, -21, -21, -21,
  -22, -22, -23, -23, -23, -24, -24, -25, -25, -25, -26, -26, -26, -27, -27, -27, -27, -28, -28, -28, -29, -29, -29, -29, -29, -30, -30, -30, -30, -30,
  -31, -31, -31, -31, -31, -31, -31, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -31, -31,
  -31, -31, -31, -31, -31, -30, -30, -30, -30, -30, -29, -29, -29, -29, -29, -28, -28, -28, -27, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24,
  -23, -23, -23, -22, -22, -21, -21, -21, -20, -20, -19, -19, -18, -18, -17, -17, -16, -16, -16, -15, -15, -14, -14, -13, -13, -12, -11, -11, -10, -10,
  -9, -9, -8, -8, -7, -7, -6, -6, -5, -4, -4, -3, -3, -2, -2, -1, -1
};

class ArduinoGame
{
 public:
  ArduinoGame() {};
  virtual ~ArduinoGame() {};
  virtual void UpdatePhysics( ) = 0;
  virtual void ResetGame(
    uint8_t isInit, uint8_t whoWon) = 0;
  virtual void ProcessInput(
    int32_t p1ax, int32_t p1ay, int8_t p1b0,
    int8_t p1b1, int8_t p1b2, int8_t p1b3, int32_t p2ax, int32_t p2ay, int8_t p2b0,
    int8_t p2b1, int8_t p2b2,
    int8_t p2b3) = 0;
};


/* Function prototype */
void SetPixel(int8_t y, int8_t x, uint32_t val);
uint32_t GetPixel(int8_t x, int8_t y);
void DisplayScore( uint16_t score, uint32_t rgb);
void DrawNumber( uint8_t number, uint8_t offsetX, uint8_t offsetY,
                 uint32_t rgb);

#define IsPixelLit(x,y) GetPixel(x, y)

/* NRF controller reading */
#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08
#define STICK 0x10

/* Bitfield definitions for a uint32_t which olds all input state */
#define BUTTON_MASK  0x0000001F
#define X_AXIS_MASK  0x00007FE0
#define X_AXIS_SHIFT 5
#define Y_AXIS_MASK  0x01FF8000
#define Y_AXIS_SHIFT 15
#define PLAYER_MASK  0x06000000ul
#define PLAYER_SHIFT 25

/* Getter and setter for 5 bit field which holds button presses */
#define SET_BUTTONS(js, b) { \
    (js) &= ~BUTTON_MASK; \
    (js) |= (b); \
  }

#define GET_BUTTONS(js) ((js) & BUTTON_MASK)

/* Getter and setter for 10 bit field which holds the X axis ADC value */
#define SET_X_AXIS(js, x) { \
    (js) &= ~X_AXIS_MASK; \
    (js) |= ((x) << X_AXIS_SHIFT); \
  }

#define GET_X_AXIS(js) (((js) & X_AXIS_MASK) >> X_AXIS_SHIFT)

/* Getter and setter for 10 bit field which holds the Y axis ADC value */
#define SET_Y_AXIS(js, y) { \
    (js) &= ~Y_AXIS_MASK; \
    (js) |= ((y) << Y_AXIS_SHIFT); \
  }

#define GET_Y_AXIS(js) (((js) & Y_AXIS_MASK) >> Y_AXIS_SHIFT)

/* Getter and setter for 2 bit field which holds the player value */
#define SET_PLAYER(js, pl) { \
    (js) &= ~PLAYER_MASK; \
    (js) |= (((uint32_t)pl) << PLAYER_SHIFT); \
  }

#define GET_PLAYER(js) (((js) & PLAYER_MASK) >> PLAYER_SHIFT)

#endif
