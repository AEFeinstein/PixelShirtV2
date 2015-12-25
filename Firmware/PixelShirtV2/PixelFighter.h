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

class PixelFighterGame : public ArduinoGame
{
 public:
  PixelFighterGame(void);
  ~PixelFighterGame(void) {};
  void UpdatePhysics(void);
  void ResetGame( uint8_t isInit,
                  uint8_t losers);
  void ProcessInput( int32_t p1, int32_t p2);
};

class PixelFighter
{
 public:
  PixelFighter(uint8_t facing);
  ~PixelFighter(void) {};
  void DrawFighter(void);
  void ManageTimers(uint8_t xBound, uint8_t yBound);
  void ProcessFighterInput(uint32_t input);
  void InitFighter(uint8_t facing);
  uint8_t getXPos(void);
  uint8_t isJumping(void);
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

#endif