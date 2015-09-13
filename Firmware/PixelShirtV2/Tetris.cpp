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

#include "Tetris.h"

#define WALL_COLOR  0x000040

uint32_t tetrominoColors[7] = {
  0x00400000,
  0x00221D00,
  0x000E3100,
  0x00002817,
  0x00001728,
  0x000E0031,
  0x0022001D,
};

/**
 * Default constructor. Reset the game with the init flag, and put it in
 * multiplayer mode
 */
Tetris::Tetris(void)
{
  multiplayer = 1;
  ResetGame(1, 0);
}

/**
 * Clear the display and reset all state & timers. If this is the initial reset,
 * don't start the game yet. Otherwise start the game
 *
 * @param isInit  Whether or not this is the initial reset
 * @param whoWon  Unused, this is a team effort (or single player)
 */
void Tetris::ResetGame(
  uint8_t isInit,
  __attribute__((unused)) uint8_t whoWon)
{
  uint8_t i;
  uint8_t x, y;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }

  if (isInit) {
    gameOver = 1;
  }
  else {
    gameOver = 0;
    NewActiveTetromino( 1); /* ignore return, it will always be placed */
    NewActiveTetromino( 0); /* ignore return, it will always be placed */
    DrawActiveTetromino();
    DrawNextTetromino();
  }
  for(i=0; i < BOARD_SIZE; i++) {
    SetPixel(3,i,WALL_COLOR);
    SetPixel(14,i,WALL_COLOR);
  }

  rotatedYet = 0;
  hardDroppedYet = 0;
  dropTimer = IRQ_HZ/2;
  cursorTimer = 0;
  turboMode = 0;
  rowsCleared = 0;
  leadPlayer = 0;
  downTimer = 0;
}

/**
 * Debounces input with cursorTimer and drops tetrominos with dropTimer
 */
void Tetris::UpdatePhysics(void)
{
  if(gameOver) {
    return;
  }

  if(cursorTimer) {
    cursorTimer--;
  }

  if(dropTimer) {
    dropTimer--;
  }
  else {
    DropActiveTetromino();

    dropTimer = IRQ_HZ/2 - rowsCleared;
    if(turboMode) {
      dropTimer /= 4;
    }
  }
}

/**
 * If both players hold left for two seconds, the game switches modes between
 * single & multiplayer.
 * Both players tapping the down button starts the game.
 * Left and right on the analog stick moves the tetromino laterally. Tilting
 * down drops it faster.
 * The left and right buttons rotate the tetromino, and the up button drops it.
 *
 * @param p1  Player 1's input, to be masked
 * @param p2  Player 1's input, to be masked
 */
void Tetris::ProcessInput(int32_t p1, int32_t p2)
{
  int32_t ax, ay;
  int8_t br, bl, bu;

  /* If both controllers hold left for two seconds, switch game mode */
  if((GET_BUTTONS(p1) & LEFT) && (GET_BUTTONS(p2) & LEFT)) {
    downTimer++;
  }
  else {
    downTimer = 0;
  }
  if(downTimer == IRQ_HZ * 2) {
    multiplayer = (multiplayer+1)%2;
    ResetGame(1, 0);
    return;
  }

  /* If the game is over, and active players tap down, the game starts */
  if(gameOver) {
    if(multiplayer) {
      if((GET_BUTTONS(p1) & DOWN) && (GET_BUTTONS(p2) & DOWN)) {
        ResetGame(0, 0);
      }
    }
    else {
      if((GET_BUTTONS(p1) & DOWN)) {
        ResetGame(0, 0);
      }
    }
    return;
  }

  /* Get the input, depending on game mode */
  if(multiplayer) {
    if(leadPlayer%2) {
      ax = (GET_X_AXIS(p1));
      ay = (GET_Y_AXIS(p1));
      bu = (GET_BUTTONS(p2) & UP);
      bl = (GET_BUTTONS(p2) & LEFT);
      br = (GET_BUTTONS(p2) & RIGHT);
    }
    else {
      ax = (GET_X_AXIS(p2));
      ay = (GET_Y_AXIS(p2));
      bu = (GET_BUTTONS(p1) & UP);
      bl = (GET_BUTTONS(p1) & LEFT);
      br = (GET_BUTTONS(p1) & RIGHT);
    }
  }
  else {
    ax = (GET_X_AXIS(p1));
    ay = (GET_Y_AXIS(p1));
    bu = (GET_BUTTONS(p1) & UP);
    bl = (GET_BUTTONS(p1) & LEFT);
    br = (GET_BUTTONS(p1) & RIGHT);
  }

  /* Rotations */
  if(bl && !rotatedYet) {
    RotateActiveTetromino( CLOCKWISE);
    rotatedYet = 1;
  }
  else if(br && !rotatedYet) {
    RotateActiveTetromino( COUNTERCLOCKWISE);
    rotatedYet = 1;
  }
  else if(br == 0 && bl == 0) {
    rotatedYet = 0;
  }

  /* Auto-drop */
  if(bu && !hardDroppedYet) {
    while(DropActiveTetromino()) {
      ;
    }
    hardDroppedYet = 1;
  }
  else if (!bu) {
    hardDroppedYet = 0;
  }

  /* Fast-drop */
  if(ay > 768 && !turboMode) {
    turboMode = 1;
    dropTimer = 0;
  }
  else if(ay <= 768) {
    turboMode = 0;
  }

  /* Lateral movement, with debounce */
  if(ax > 768) {
    if(cursorTimer == 0) {
      SlideActiveTetromino( T_RIGHT);
      cursorTimer = IRQ_HZ/8;
    }
  }
  else if(ax < 256) {
    if(cursorTimer == 0) {
      SlideActiveTetromino( T_LEFT);
      cursorTimer = IRQ_HZ/8;
    }
  }
  else {
    /* reset the timer */
    cursorTimer = 0;
  }
}

/**
 * Moves the next tetromino onto the playing field and generates a new random
 * tetromino to be dropped next. If this is the first piece, it doesn't
 * move the next piece on to the field, since there is no next yet.
 * When the tetromino is placed on the field, collisions are checked.
 *
 * @param isFirst If this is the first tetromino generated in the game
 * @return      1 if the tetromino collided (game over), 0 otherwise
 */
uint8_t Tetris::NewActiveTetromino(uint8_t isFirst)
{
  uint8_t i;

  if(isFirst) {
    nextType = (tetromino)random(7);
    nextTetrominoColor = tetrominoColors[nextType];
    return 0;
  }
  else {
    activeType = nextType;
    nextType = (tetromino)random(7);

    activeTetrominoColor = nextTetrominoColor;
    nextTetrominoColor = tetrominoColors[nextType];
  }

  switch(nextType) {
    case O_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = o_tet[0][i][X] - 1;
        nextTetromino[i][Y] = o_tet[0][i][Y] - 1;
      }
      break;
    case I_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = i_tet[1][i][X] - 2;
        nextTetromino[i][Y] = i_tet[1][i][Y];
      }
      break;
    case S_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = s_tet[1][i][X] - 2;
        nextTetromino[i][Y] = s_tet[1][i][Y];
      }
      break;
    case Z_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = z_tet[1][i][X] - 2;
        nextTetromino[i][Y] = z_tet[1][i][Y];
      }
      break;
    case L_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = l_tet[1][i][X] - 2;
        nextTetromino[i][Y] = l_tet[1][i][Y];
      }
      break;
    case J_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = j_tet[1][i][X] - 2;
        nextTetromino[i][Y] = j_tet[1][i][Y];
      }
      break;
    case T_TET:
      for(i=0; i < 4; i++) {
        nextTetromino[i][X] = t_tet[1][i][X] - 2;
        nextTetromino[i][Y] = t_tet[1][i][Y];
      }
      break;
  }

  switch(activeType) {
    case O_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = o_tet[0][i][X];
        activeTetromino[i][Y] = o_tet[0][i][Y];
      }
      break;
    case I_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = i_tet[0][i][X];
        activeTetromino[i][Y] = i_tet[0][i][Y];
      }
      break;
    case S_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = s_tet[0][i][X];
        activeTetromino[i][Y] = s_tet[0][i][Y];
      }
      break;
    case Z_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = z_tet[0][i][X];
        activeTetromino[i][Y] = z_tet[0][i][Y];
      }
      break;
    case L_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = l_tet[0][i][X];
        activeTetromino[i][Y] = l_tet[0][i][Y];
      }
      break;
    case J_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = j_tet[0][i][X];
        activeTetromino[i][Y] = j_tet[0][i][Y];
      }
      break;
    case T_TET:
      for(i=0; i < 4; i++) {
        activeTetromino[i][X] = t_tet[0][i][X];
        activeTetromino[i][Y] = t_tet[0][i][Y];
      }
      break;
  }

  activeOffset[X] = 7;
  activeOffset[Y] = -2;
  activeRotation = 0;

  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] >= 0
        && IsPixelLit(activeTetromino[i][X] + activeOffset[X],
                      activeTetromino[i][Y] + activeOffset[Y])) {
      return 1;
    }
  }
  return 0;
}

/**
 * Removes the active tetromino piece from the playing field.
 * This is called before dropping, rotating, or sliding a piece.
 */
void Tetris::ClearActiveTetromino(void)
{
  uint8_t i;
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] >= 0) {
      SetPixel(
        activeTetromino[i][X] + activeOffset[X],
        activeTetromino[i][Y] + activeOffset[Y],
        EMPTY_COLOR);
    }
  }
}

/**
 * Draws the active tetromino on the playing field.
 * This is called after dropping, rotating, or sliding a piece.
 */
void Tetris::DrawActiveTetromino(void)
{
  uint8_t i;
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] >= 0) {
      SetPixel(
        activeTetromino[i][X] + activeOffset[X],
        activeTetromino[i][Y] + activeOffset[Y],
        activeTetrominoColor);
    }
  }
}

/**
 * Clears the next tetromino, to the left of the playing field.
 */
void Tetris::ClearNextTetromino(void)
{
  uint8_t i;
  for(i=0; i < 4; i++) {
    SetPixel(nextTetromino[i][X], nextTetromino[i][Y], EMPTY_COLOR);
  }
}

/**
 * Draws the next tetromino, to the left of the playing field
 */
void Tetris::DrawNextTetromino(void)
{
  uint8_t i;
  for(i=0; i < 4; i++) {
    SetPixel(nextTetromino[i][X], nextTetromino[i][Y], nextTetrominoColor);
  }
}

/**
 * If the active tetromino can be dropped one row, it is. Otherwise it is
 * locked in place, any full rows are cleared, score is incremented as
 * appropriate, and a new active & next tetromino are drawn.
 *
 * @return 1 if the piece dropped or 0 if the piece was blocked
 */
uint8_t Tetris::DropActiveTetromino(void)
{
  uint8_t i, j, k, rowFull, isBlocked = 0, rc = 0;
  ClearActiveTetromino();

  /* Check to see if the tetromino can be moved down */
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] + 1 == BOARD_SIZE || /* floor */
        IsPixelLit(activeTetromino[i][X] + activeOffset[X],
                   activeTetromino[i][Y] + activeOffset[Y] +
                   1)) {
      isBlocked = 1;
    }
  }

  /* If the piece is not blocked, move it downward */
  if(!isBlocked) {
    activeOffset[Y]++;
    DrawActiveTetromino();
    return 1;
  }
  /* Otherwise set the piece in the clear lines, and try to spawn a new piece */
  else {
    /* whenever a piece is set, swap the controls */
    leadPlayer = (leadPlayer+1)%2;
    /* Set the piece */
    for(i=0; i < 4; i++) {
      SetPixel(activeTetromino[i][X] + activeOffset[X],
               activeTetromino[i][Y] + activeOffset[Y], activeTetrominoColor);
    }

    /* check for rows to clear, clear them */
    for(i=0; i < BOARD_SIZE; i++) {
      rowFull = 1;
      for(j=0; j < 10; j++) {
        if(!IsPixelLit(j + 3, i)) {
          rowFull = 0;
        }
      }
      if(rowFull) {
        rc++;
        /* drop row */
        for(j = i; j > 0; j--) {
          for(k=4; k < 14; k++) {
            SetPixel(k, j, GetPixel(k, j-1));
          }
        }
        for(k=4; k < 14; k++) {
          SetPixel(k, 0,EMPTY_COLOR);
        }
      }
    }

    /* if a row was cleared, add to the score, draw it, and check for victory */
    if(rc) {
      rowsCleared += rc;

      for(i=0; i < rowsCleared; i++) {
        if(BOARD_SIZE-1-i >= 0) {
          SetPixel(BOARD_SIZE-1, BOARD_SIZE-1-i, SCORE_COLOR);
        }
      }

      if(rowsCleared >= 16) {
        gameOver = 1;
        return 0;
      }
    }

    ClearNextTetromino();
    if(NewActiveTetromino( 0)) {
      gameOver = 1; /* the new tetromino spawned and intersected */
    }
    DrawActiveTetromino();
    DrawNextTetromino();
    return 0;
  }
}

/**
 * Moves the active tetromino one pixel left or right, as long as nothing
 * is blocking the movement
 *
 * @param direction The direction to move. T_RIGHT == +1, T_LEFT = -1
 */
void Tetris::SlideActiveTetromino(
  int8_t direction)
{
  uint8_t i;

  ClearActiveTetromino();

  /* Check to see if the tetromino can be moved laterally */
  for(i=0; i < 4; i++) {
    /* 3 is a wall, 2 is a set piece */
    if(IsPixelLit(activeTetromino[i][X] + activeOffset[X] + direction,
                  activeTetromino[i][Y] + activeOffset[Y]) ) {
      DrawActiveTetromino();
      return; /* can't slide :( */
    }
  }

  /* If the piece is not blocked, move it downward */
  activeOffset[X] += direction;
  DrawActiveTetromino();
}

/**
 * Rotates the active tetromino using a lookup table of tetromino orientations
 * If the piece cannot be rotated (up against a wall, etc), the function does
 * nothing
 *
 * @param direction The direction to rotate, CLOCKWISE == 1,
 *                  COUNTERCLOCKWISE == -1
 */
void Tetris::RotateActiveTetromino(
  int8_t direction)
{
  uint8_t newRotation[4][2];
  uint8_t i;
  uint8_t oldRotation = activeRotation;

  ClearActiveTetromino();

  switch(activeType) {
    case O_TET:
      /* no rotation */
      DrawActiveTetromino();
      return;
    case I_TET:
      activeRotation = WRAP(activeRotation + direction, 2);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = i_tet[activeRotation][i][X];
        newRotation[i][Y] = i_tet[activeRotation][i][Y];
      }
      break;
    case S_TET:
      activeRotation = WRAP(activeRotation + direction, 2);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = s_tet[activeRotation][i][X];
        newRotation[i][Y] = s_tet[activeRotation][i][Y];
      }
      break;
    case Z_TET:
      activeRotation = WRAP(activeRotation + direction, 2);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = z_tet[activeRotation][i][X];
        newRotation[i][Y] = z_tet[activeRotation][i][Y];
      }
      break;
    case L_TET:
      activeRotation = WRAP(activeRotation + direction, 4);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = l_tet[activeRotation][i][X];
        newRotation[i][Y] = l_tet[activeRotation][i][Y];
      }
      break;
    case J_TET:
      activeRotation = WRAP(activeRotation + direction, 4);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = j_tet[activeRotation][i][X];
        newRotation[i][Y] = j_tet[activeRotation][i][Y];
      }
      break;
    case T_TET:
      activeRotation = WRAP(activeRotation + direction, 4);
      for(i=0; i < 4; i++) {
        newRotation[i][X] = t_tet[activeRotation][i][X];
        newRotation[i][Y] = t_tet[activeRotation][i][Y];
      }
      break;
  }

  for(i=0; i < 4; i++) {
    if(IsPixelLit(newRotation[i][X] + activeOffset[X],
                  newRotation[i][Y] + activeOffset[Y]) ||
        newRotation[i][Y] + activeOffset[Y] < 0  ||
        newRotation[i][Y] + activeOffset[Y] > 15) {
      activeRotation = oldRotation; /* undo */
      DrawActiveTetromino();
      return; /* can't rotate :( */
    }
  }

  /* rotate the tetromino */
  for(i=0; i < 4; i++) {
    activeTetromino[i][X] = newRotation[i][X];
    activeTetromino[i][Y] = newRotation[i][Y];
  }
  DrawActiveTetromino();
}
