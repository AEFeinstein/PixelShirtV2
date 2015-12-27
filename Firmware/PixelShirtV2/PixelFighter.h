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

#ifndef _FIGHTER_H_
#define _FIGHTER_H_

typedef enum {
  FACING_RIGHT,
  FACING_LEFT
} direction_t;

class PixelFighter
{
 public:
  PixelFighter() {;}
  ~PixelFighter(void) {};
  void InitFighter(direction_t facing, uint8_t resetHP);
  void ProcessFighterInput(uint32_t input);
  void ManageTimers(uint8_t xBound, uint8_t yBound);
  void DrawFighter(void);

  uint8_t getXPos(void);
  uint8_t getActionHeight(void);

  uint8_t isJumping(void);
  uint8_t isAttacking(void);
  uint8_t isBlocking(void);

  void decrementHP(void);
 private:
  uint8_t xPos;
  int8_t velocity;
  uint8_t yPos;

  uint8_t direction;
  uint8_t sprite;
  uint8_t hitPoints;

  uint8_t danceTimer;
  uint8_t jumpTimer;
  uint8_t actionTimer;
  uint8_t moveTimer;
};

class PixelFighterGame : public ArduinoGame
{
 public:
  PixelFighterGame(void);
  ~PixelFighterGame(void) {};
  void UpdatePhysics(void);
  void ResetGame( uint8_t isInit,
                  uint8_t losers);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  PixelFighter fighterOne;
  PixelFighter fighterTwo;
};

#endif
