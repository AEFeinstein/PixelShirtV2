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
  Shooter();
  ~Shooter() {};
  void UpdatePhysics( );
  void ResetGame(  uint8_t isInit,
                   uint8_t whoWon);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  void ShiftBoard();
  void DropBoard();
  void ClearPlayers();
  uint8_t DrawPlayers();
  void GameOver();
  void SpawnWave();
  void KillEnemy();
  uint8_t NumEnemies();
  uint8_t activeDirection;
  uint8_t moveTimer;
  Player player1;
  Player player2;
  uint16_t score;
  uint8_t currentMovementSpeed;
  uint8_t resetTimer;
};

#endif
