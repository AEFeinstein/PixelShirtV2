/*
 * Pong.h
 *
 *  Created on: Oct 27, 2013
 *      Author: adam
 */

#ifndef PONG_H_
#define PONG_H_

#include "ArduinoGame.h"

/*
 * Defines
 */
#define S_M     IRQ_HZ /* Position Multiplier */
#define V_M     IRQ_HZ /* Velocity Multiplier */
#define PADDLE_SIZE (4 * V_M * S_M) /* in sim size */
#define TIME_STEP (S_M/IRQ_HZ) /* (1/32) seconds * 32 multiplier */
#define EXTRA_ROTATION_ON_EDGE  15 /*degrees  */
#define SPEED_LIMIT 1200

/*
 * Class
 */
class Pong : public ArduinoGame
{
 public:
  Pong();
  ~Pong() {};
  void UpdatePhysics( );
  void ResetGame(  uint8_t isInit,
                   uint8_t whoWon);
  void ProcessInput(  int32_t p1ax,
                      int32_t p1ay, int8_t p1b0, int8_t p1b1,
                      int8_t p1b2, int8_t p1b3, int32_t p2ax, int32_t p2ay, int8_t p2b0, int8_t p2b1,
                      int8_t p2b2, int8_t p2b3);
 private:
  void DrawField( );
  void IncreaseSpeed(int16_t speedM);
  void RotateBall(int16_t degree);
  int16_t binPaddleLocation(int32_t loc);
  int16_t paddleLocL;
  int16_t paddleLocR;
  uint16_t restartTimer;
  int16_t ballLoc[2];
  int32_t ballVel[2];
};

#endif /* PONG_H_ */




