#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "ArduinoGame.h"

#define CLOCKWISE 1
#define COUNTERCLOCKWISE -1

#define T_RIGHT 1
#define T_LEFT -1

typedef enum {O_TET, I_TET, S_TET, Z_TET, L_TET, J_TET, T_TET} tetromino;

const uint8_t o_tet[1][4][2] = {
  {{1,1},{1,2},{2,1},{2,2}}, /* square */
};
const uint8_t i_tet[2][4][2] = {
  {{0,1},{1,1},{2,1},{3,1}},
  {{2,0},{2,1},{2,2},{2,3}}
};
const uint8_t s_tet[2][4][2] = {
  {{1,2},{2,2},{2,1},{3,1}},
  {{2,0},{2,1},{3,1},{3,2}}
};
const uint8_t z_tet[2][4][2] = {
  {{1,1},{2,1},{2,2},{3,2}},
  {{3,0},{3,1},{2,1},{2,2}}
};
const uint8_t l_tet[4][4][2] = {
  {{1,1},{2,1},{3,1},{1,2}},
  {{2,0},{2,1},{2,2},{3,2}},
  {{1,1},{2,1},{3,1},{3,0}},
  {{2,0},{2,1},{2,2},{1,0}},
};
const uint8_t j_tet[4][4][2] = {
  {{1,1},{2,1},{3,1},{3,2}},
  {{2,0},{2,1},{2,2},{3,0}},
  {{1,1},{2,1},{3,1},{1,0}},
  {{2,0},{2,1},{2,2},{1,2}},
};
const uint8_t t_tet[4][4][2] = {
  {{1,1},{2,1},{3,1},{2,2}},
  {{2,0},{2,1},{2,2},{3,1}},
  {{1,1},{2,1},{3,1},{2,0}},
  {{2,0},{2,1},{2,2},{1,1}},
};

class Tetris : public ArduinoGame
{
 public:
  Tetris(void);
  ~Tetris(void) {};
  void UpdatePhysics(void);
  void ResetGame(  uint8_t isInit,
                   uint8_t whoWon);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  /* Drawing */
  void ClearActiveTetromino(void);
  void DrawActiveTetromino(void);
  void ClearNextTetromino(void);
  void DrawNextTetromino(void);
  /* Moving */
  uint8_t NewActiveTetromino(uint8_t isFirst);
  void RotateActiveTetromino(int8_t direction);
  uint8_t DropActiveTetromino(void);
  void SlideActiveTetromino(int8_t direction);
  /* Timers */
  uint8_t dropTimer;
  uint8_t rotatedYet;
  uint8_t hardDroppedYet;
  uint8_t cursorTimer;
  uint8_t turboMode;
  /* State */
  int8_t nextTetromino[4][2];
  int8_t activeTetromino[4][2];
  int8_t activeOffset[2];
  uint8_t activeRotation;
  tetromino activeType;
  tetromino nextType;
  uint8_t gameOver;
  uint8_t rowsCleared;
  uint8_t leadPlayer;
  uint8_t multiplayer;
  uint8_t downTimer;
  uint32_t nextTetrominoColor;
  uint32_t activeTetrominoColor;
};

#endif
