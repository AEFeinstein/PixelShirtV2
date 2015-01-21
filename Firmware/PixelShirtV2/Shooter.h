#ifndef _SHOOTER_H_
#define _SHOOTER_H_

#include "ArduinoGame.h"

#define NUM_BULLETS 2

typedef struct {
  int16_t position;
  int8_t bullets[NUM_BULLETS][2];
  uint8_t shotClock;
} Player;

class Shooter : public ArduinoGame
{
 public:
  Shooter() {};
  ~Shooter() {};
  void UpdatePhysics( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void ResetGame( uint8_t field[BOARD_SIZE][BOARD_SIZE][3], uint8_t isInit,
                  uint8_t whoWon);
  void ProcessInput( uint8_t field[BOARD_SIZE][BOARD_SIZE][3], int32_t p1ax,
                     int32_t p1ay, int8_t p1b0, int8_t p1b1,
                     int8_t p1b2, int8_t p1b3, int32_t p2ax, int32_t p2ay, int8_t p2b0, int8_t p2b1,
                     int8_t p2b2, int8_t p2b3);
 private:
  void ShiftBoard(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void DropBoard(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void ClearPlayers(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  uint8_t DrawPlayers(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void GameOver(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void SpawnWave(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void KillEnemy(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  uint8_t NumEnemies(uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  uint8_t activeDirection;
  uint8_t moveTimer;
  Player p1;
  Player p2;
  uint16_t score;
  uint8_t currentMovementSpeed;
  uint8_t resetTimer;
};

#endif
