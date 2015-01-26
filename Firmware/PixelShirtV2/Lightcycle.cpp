#include "Lightcycle.h"

void Lightcycle::UpdatePhysics( uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t losers = 0;
  uint8_t i;

  // Only update physics twice a second
  if(movementTimer) {
    movementTimer--;
  }
  else {
    started = 1;
    movementTimer = IRQ_HZ/8;

    if(showingWinningLEDs) {
      for(i=0; i < BOARD_SIZE; i++) {
        SetPixel(0,i, 0,0,0 );
        SetPixel(BOARD_SIZE-1,i, 0,0,0 );
      }
      showingWinningLEDs = 0;
    }
    // Move the cycles
    switch(p1dir) {
      case UP:
        p1pos[Y] = WRAP(p1pos[Y]-1, BOARD_SIZE);
        break;
      case DOWN:
        p1pos[Y] = WRAP(p1pos[Y]+1, BOARD_SIZE);
        break;
      case LEFT:
        p1pos[X] = WRAP(p1pos[X]-1, BOARD_SIZE);
        break;
      case RIGHT:
        p1pos[X] = WRAP(p1pos[X]+1, BOARD_SIZE);
        break;
    }
    switch(p2dir) {
      case UP:
        p2pos[Y] = WRAP(p2pos[Y]-1, BOARD_SIZE);
        break;
      case DOWN:
        p2pos[Y] = WRAP(p2pos[Y]+1, BOARD_SIZE);
        break;
      case LEFT:
        p2pos[X] = WRAP(p2pos[X]-1, BOARD_SIZE);
        break;
      case RIGHT:
        p2pos[X] = WRAP(p2pos[X]+1, BOARD_SIZE);
        break;
    }

    // Check for collisions, add to cycle trails
    if(IsPixelLit(p1pos[X], p1pos[Y])) {
      losers |= 0x01;
    }
    if(IsPixelLit(p2pos[X], p2pos[Y])) {
      losers |= 0x02;
    }

    SetPixel(p1pos[X], p1pos[Y], 0,0x40,0);
    SetPixel(p2pos[X], p2pos[Y], 0,0,0x40);

    // Check for a new round
    if(losers) {
      ResetGame(field, 0, losers);
    }
  }
}

void Lightcycle::ResetGame( uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
                            __attribute__((unused)) uint8_t isInit,
                            uint8_t whoWon)
{
  uint8_t i;
  movementTimer = IRQ_HZ*3;
  started = 0;
  // Clear the field
  memset((void*)field, 0, sizeof(uint8_t) * BOARD_SIZE * BOARD_SIZE * 3);

  // Place P1, P2
  p1pos[X] = 3;
  p1pos[Y] = 7;
  p1dir = RIGHT;

  p2pos[X] = 12;
  p2pos[Y] = 8;
  p2dir = LEFT;

  // Draw the initial cycles
  SetPixel(p1pos[X], p1pos[Y], 0,0x40,0);
  SetPixel(p2pos[X], p2pos[Y], 0,0,0x40);

  // If there was a winner, draw some lights
  if(whoWon & 0x01 && whoWon & 0x02) {
    ;// everyone loses!
  }
  else if(whoWon & 0x01) {
    for(i=0; i < BOARD_SIZE; i++) {
      SetPixel(BOARD_SIZE-1, i, 0,0,0x40);
      showingWinningLEDs = 1;
    }
  }
  else if(whoWon & 0x02) {
    for(i=0; i < BOARD_SIZE; i++) {
      SetPixel(0, i, 0,0x40,0);
      showingWinningLEDs = 1;
    }
  }
}

void Lightcycle::ProcessInput(
  __attribute__((unused))  uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
  __attribute__((unused)) int32_t p1ax,
  __attribute__((unused)) int32_t p1ay,
  int8_t p1bl,
  int8_t p1br,
  int8_t p1bu,
  int8_t p1bd,
  __attribute__((unused)) int32_t p2ax,
  __attribute__((unused)) int32_t p2ay,
  int8_t p2bl,
  int8_t p2br,
  int8_t p2bu,
  int8_t p2bd)
{
  if(!started) {
    return;
  }
  // Switch position based on buttons
  if(p1bu) {
    if(p1dir != DOWN) {
      p1dir = UP;
    }
  }
  else if(p1bd) {
    if(p1dir != UP) {
      p1dir = DOWN;
    }
  }
  else if(p1bl) {
    if(p1dir != RIGHT) {
      p1dir = LEFT;
    }
  }
  else if(p1br) {
    if(p1dir != LEFT) {
      p1dir = RIGHT;
    }
  }

  if(p2bu) {
    if(p2dir != DOWN) {
      p2dir = UP;
    }
  }
  else if(p2bd) {
    if(p2dir != UP) {
      p2dir = DOWN;
    }
  }
  else if(p2bl) {
    if(p2dir != RIGHT) {
      p2dir = LEFT;
    }
  }
  else if(p2br) {
    if(p2dir != LEFT) {
      p2dir = RIGHT;
    }
  }
}
