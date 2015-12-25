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

#ifndef _SHOOTER_H_
#define _SHOOTER_H_

#define NUM_BULLETS 2

typedef struct {
  int16_t position;
  int8_t bullets[NUM_BULLETS][2];
  uint8_t shotClock;
} Player;

class Shooter : public ArduinoGame
{
 public:
  Shooter(void);
  ~Shooter(void) {};
  void UpdatePhysics(void);
  void ResetGame(  uint8_t isInit,
                   uint8_t whoWon);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  void ShiftBoard(void);
  void DropBoard(void);
  void ClearPlayers(void);
  uint8_t DrawPlayers(void);
  void GameOver(void);
  void SpawnWave(void);
  void KillEnemy(void);
  uint8_t NumEnemies(void);
  uint8_t activeDirection;
  uint8_t moveTimer;
  Player player1;
  Player player2;
  uint16_t score;
  uint8_t currentMovementSpeed;
  uint8_t resetTimer;
  uint8_t playerMovementTimer[2];
};

#endif
