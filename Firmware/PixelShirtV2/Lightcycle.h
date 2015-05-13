#ifndef _LIGHTCYCLE_H_
#define _LIGHTCYCLE_H_

#include "ArduinoGame.h"

class Lightcycle : public ArduinoGame
{
 public:
  Lightcycle();
  ~Lightcycle() {};
  void UpdatePhysics( );
  void ResetGame( uint8_t isInit,
                  uint8_t whoWon);
  void ProcessInput( int32_t p1ax,
                     int32_t p1ay, int8_t p1b0, int8_t p1b1,
                     int8_t p1b2, int8_t p1b3, int32_t p2ax, int32_t p2ay, int8_t p2b0, int8_t p2b1,
                     int8_t p2b2, int8_t p2b3);
 private:
  int8_t p1pos[2], p2pos[2];
  uint8_t p1dir, p2dir;
  uint8_t movementTimer;
  uint8_t showingWinningLEDs;
  uint8_t started;
};

#endif
