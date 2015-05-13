#ifndef _SUPER_SQUARE_H_
#define _SUPER_SQUARE_H_

#include "ArduinoGame.h"

#define NUM_LINES 6
#define LINE_SPEED_SCALER 128

typedef struct {
  int16_t position;
  int16_t velocity;
  uint8_t direction;
} Line;

#define INVALIDATE_LINE(x) ((x).position = -1)
#define IS_LINE_VALID(x) ((x).position >= 0)

class SuperSquare : public ArduinoGame
{
 public:
  SuperSquare();
  ~SuperSquare() {};
  void UpdatePhysics( );
  void ResetGame(  uint8_t isInit,
                  uint8_t whoWon);
  void ProcessInput(  int32_t p1ax,
                     int32_t p1ay, int8_t p1b0, int8_t p1b1,
                     int8_t p1b2, int8_t p1b3, int32_t p2ax, int32_t p2ay, int8_t p2b0, int8_t p2b1,
                     int8_t p2b2, int8_t p2b3);
 private:
  void PlacePlayerPixel(uint8_t position[]);
  uint8_t DrawLine( Line line, uint32_t rgb);
  void AddLine(uint8_t direction, uint16_t velocity);

  int16_t playerPosition;
  int16_t currentVelocity;
  uint16_t score;
  uint8_t newLineTimer;
  uint8_t resetTimer;
  Line lines[NUM_LINES];
  uint8_t linesDrawn;
};

#endif
