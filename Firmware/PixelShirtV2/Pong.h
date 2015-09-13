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
  Pong(void);
  ~Pong(void) {};
  void UpdatePhysics(void);
  void ResetGame(  uint8_t isInit,
                   uint8_t whoWon);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  void DrawField(void);
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




