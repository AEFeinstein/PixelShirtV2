#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "ArduinoGame.h"

#define CLOCKWISE 1
#define COUNTERCLOCKWISE -1

#define T_RIGHT 1
#define T_LEFT -1

typedef enum {O_TET, I_TET, S_TET, Z_TET, L_TET, J_TET, T_TET} tetromino;

const uint8_t o_tet[1][4][2] = {
  {{1,1},{1,2},{2,1},{2,2}}, // square
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
  Tetris();
  ~Tetris() {};
  void UpdatePhysics( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void ResetGame( uint8_t field[BOARD_SIZE][BOARD_SIZE][3], uint8_t isInit,
                  uint8_t whoWon);
  void ProcessInput( uint8_t field[BOARD_SIZE][BOARD_SIZE][3], int32_t p1ax,
                     int32_t p1ay, int8_t p1b0, int8_t p1b1,
                     int8_t p1b2, int8_t p1b3, int32_t p2ax, int32_t p2ay, int8_t p2b0, int8_t p2b1,
                     int8_t p2b2, int8_t p2b3);
 private:
  // Drawing
  void ClearActiveTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void DrawActiveTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void ClearNextTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void DrawNextTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  // Moving
  uint8_t NewActiveTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
                              uint8_t isFirst);
  void RotateActiveTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
                              int8_t direction);
  uint8_t DropActiveTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3]);
  void SlideActiveTetromino( uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
                             int8_t direction);
  // Timers
  uint8_t dropTimer;
  uint8_t rotatedYet;
  uint8_t hardDroppedYet;
  uint8_t cursorTimer;
  uint8_t turboMode;
  // State
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
  uint8_t nextTetrominoColor[3];
  uint8_t activeTetrominoColor[3];
};

#endif