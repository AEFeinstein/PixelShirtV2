#ifndef _LIGHTCYCLE_H_
#define _LIGHTCYCLE_H_

#include "ArduinoGame.h"

class Lightcycle : public ArduinoGame
{
 public:
  Lightcycle(void);
  ~Lightcycle(void) {};
  void UpdatePhysics(void);
  void ResetGame( uint8_t isInit,
                  uint8_t whoWon);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  int8_t p1pos[2], p2pos[2];
  uint8_t p1dir, p2dir;
  uint8_t movementTimer;
  uint8_t showingWinningLEDs;
  uint8_t started;
};

#endif
