#include "Tetris.h"

#define WALL_COLOR  0x000040

Tetris::Tetris()
{
  multiplayer = 1;
  ResetGame(1, 0);
}

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
    NewActiveTetromino( 1); // ignore return, it will always be placed
    NewActiveTetromino( 0); // ignore return, it will always be placed
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

void Tetris::UpdatePhysics( )
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

void Tetris::ProcessInput(
  
  int32_t p1ax,
  int32_t p1ay,
  int8_t p1bl,
  int8_t p1br,
  int8_t p1bu,
  int8_t p1bd,
  int32_t p2ax,
  int32_t p2ay,
  int8_t p2bl,
  int8_t p2br,
  int8_t p2bu,
  int8_t p2bd)
{
  int32_t ax, ay;
  int8_t br, bl, bu;

  if(p1bl && p2bl) {
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

  if(gameOver) {
    if(multiplayer) {
      if(p1bd && p2bd) {
        ResetGame(0, 0);
      }
    }
    else {
      if(p1bd) {
        ResetGame(0, 0);
      }
    }
    return;
  }

  if(multiplayer) {
    if(leadPlayer%2) {
      ax = p1ax;
      ay = p1ay;
      bu = p2bu;
      bl = p2bl;
      br = p2br;
    }
    else {
      ax = p2ax;
      ay = p2ay;
      bu = p1bu;
      bl = p1bl;
      br = p1br;
    }
  }
  else {
    ax = p1ax;
    ay = p1ay;
    bu = p1bu;
    bl = p1bl;
    br = p1br;
  }

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

  if(bu && !hardDroppedYet) {
    while(DropActiveTetromino()) {
      ;
    }
    hardDroppedYet = 1;
  }
  else if (!bu) {
    hardDroppedYet = 0;
  }

  if(ay > 768 && !turboMode) {
    turboMode = 1;
    dropTimer = 0;
  }
  else if(ay <= 768) {
    turboMode = 0;
  }

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
    // reset the timer
    cursorTimer = 0;
  }
}

uint8_t Tetris::NewActiveTetromino(uint8_t isFirst)
{
  uint8_t i;

  if(isFirst) {
    nextType = (tetromino)random(7);
    uint8_t r = random(0xFF);
    uint8_t g = random(0xFF);
    uint8_t b = random(0xFF);
    uint16_t sum = (r + g + b);
    r = (0x40 * r) / sum;
    g = (0x40 * g) / sum;
    b = (0x40 * b) / sum;
    nextTetrominoColor = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    return 0;
  }
  else {
    activeType = nextType;
    nextType = (tetromino)random(7);

    activeTetrominoColor = nextTetrominoColor;
    uint8_t r = random(0xFF);
    uint8_t g = random(0xFF);
    uint8_t b = random(0xFF);
    uint16_t sum = (r + g + b);
    r = (0x40 * r) / sum;
    g = (0x40 * g) / sum;
    b = (0x40 * b) / sum;
    nextTetrominoColor = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
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

void Tetris::ClearActiveTetromino( )
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

void Tetris::DrawActiveTetromino( )
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

void Tetris::ClearNextTetromino( )
{
  uint8_t i;
  for(i=0; i < 4; i++) {
    SetPixel(nextTetromino[i][X], nextTetromino[i][Y], EMPTY_COLOR);
  }
}

void Tetris::DrawNextTetromino( )
{
  uint8_t i;
  for(i=0; i < 4; i++) {
    SetPixel(nextTetromino[i][X], nextTetromino[i][Y], nextTetrominoColor);
  }
}

/**
 * Returns 1 if the piece dropped or 0 if the piece was blocked
 */
uint8_t Tetris::DropActiveTetromino( )
{
  uint8_t i, j, k, rowFull, isBlocked = 0, rc = 0;
  ClearActiveTetromino();

  // Check to see if the tetromino can be moved down
  for(i=0; i < 4; i++) {
    if(activeTetromino[i][Y] + activeOffset[Y] + 1 == BOARD_SIZE || // floor
        IsPixelLit(activeTetromino[i][X] + activeOffset[X],
                   activeTetromino[i][Y] + activeOffset[Y] +
                   1)) { // used to be ==2, set piece TOTO PROBMEL
      isBlocked = 1;
    }
  }

  // If the piece is not blocked, move it downward
  if(!isBlocked) {
    activeOffset[Y]++;
    DrawActiveTetromino();
    return 1;
  }
  // Otherwise set the piece in the  clear lines, and try to spawn a new piece
  else {
    leadPlayer = (leadPlayer+1)%2; // whenever a piece is set, swap the controls
    // Set the piece
    for(i=0; i < 4; i++) {
      SetPixel(activeTetromino[i][X] + activeOffset[X],
               activeTetromino[i][Y] + activeOffset[Y], activeTetrominoColor);
    }

    // check for rows to clear, clear them
    for(i=0; i < BOARD_SIZE; i++) {
      rowFull = 1;
      for(j=0; j < 10; j++) {
        if(!IsPixelLit(j + 3, i)) {
          rowFull = 0;
        }
      }
      if(rowFull) {
        rc++;
        // drop row
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

    // if a row was cleared, add to the score, draw it, and check for victory
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
      gameOver = 1; // the new tetromino spawned and intersected
    }
    DrawActiveTetromino();
    DrawNextTetromino();
    return 0;
  }
}

void Tetris::SlideActiveTetromino( 
                                   int8_t direction)
{
  uint8_t i;

  ClearActiveTetromino();

  // Check to see if the tetromino can be moved laterally
  for(i=0; i < 4; i++) {
    // 3 is a wall, 2 is a set piece
    if(IsPixelLit(activeTetromino[i][X] + activeOffset[X] + direction,
                  activeTetromino[i][Y] + activeOffset[Y]) ) { /* Used to be == 2 or 3 */
      DrawActiveTetromino();
      return; // can't slide :(
    }
  }

  // If the piece is not blocked, move it downward
  activeOffset[X] += direction;
  DrawActiveTetromino();
}

void Tetris::RotateActiveTetromino( 
                                    int8_t direction)
{
  uint8_t newRotation[4][2];
  uint8_t i;
  uint8_t oldRotation = activeRotation;

  ClearActiveTetromino();

  switch(activeType) {
    case O_TET:
      // no rotation
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
                  newRotation[i][Y] + activeOffset[Y])
        || // /* Used to be == 2 (piece) or 3 (wall) */
        newRotation[i][Y] + activeOffset[Y] < 0  ||
        newRotation[i][Y] + activeOffset[Y] > 15) {
      activeRotation = oldRotation; // undo
      DrawActiveTetromino();
      return; // can't rotate :(
    }
  }

  // rotate the tetromino
  for(i=0; i < 4; i++) {
    activeTetromino[i][X] = newRotation[i][X];
    activeTetromino[i][Y] = newRotation[i][Y];
  }
  DrawActiveTetromino();
}


