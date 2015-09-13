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

#include "Lightcycle.h"

/**
 * Default constructor. Resets the game with the
 * initialization flag and no winner
 */
Lightcycle::Lightcycle(void)
{
  ResetGame(1, 0);
}

/**
 * Updates the lightcycle positions twice a second.
 * Checks for collisions.
 */
void Lightcycle::UpdatePhysics(void)
{
  uint8_t losers = 0;
  uint8_t i;

  /* Only update physics twice a second */
  if(movementTimer) {
    movementTimer--;
  }
  else {
    started = 1;
    movementTimer = IRQ_HZ/8;

    if(showingWinningLEDs) {
      for(i=0; i < BOARD_SIZE; i++) {
        SetPixel(0,i, EMPTY_COLOR);
        SetPixel(BOARD_SIZE-1,i, EMPTY_COLOR);
      }
      showingWinningLEDs = 0;
    }
    /* Move the cycles */
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

    /* Check for collisions, add to cycle trails */
    if(IsPixelLit(p1pos[X], p1pos[Y])) {
      losers |= 0x01;
    }
    if(IsPixelLit(p2pos[X], p2pos[Y])) {
      losers |= 0x02;
    }

    SetPixel(p1pos[X], p1pos[Y], P1_COLOR);
    SetPixel(p2pos[X], p2pos[Y], P2_COLOR);

    /* Check for a new round */
    if(losers) {
      ResetGame(0, losers);
    }
  }
}

/**
 * Clears the screen, resets timers, and places cycles at their initial
 * positions. If there was a winner, light some LEDs on their side.
 *
 * @param isInit	1 if this is the initial reset, 0 otherwise
 * @param losers	A bitmask of losers. 0x01 indicates P1 lost,
 * 					0x02 indicates P2 lost
 */
void Lightcycle::ResetGame( __attribute__((unused)) uint8_t isInit,
                            uint8_t losers)
{
  uint8_t i;
  movementTimer = IRQ_HZ*3;
  started = 0;
  /* Clear the field */
  uint8_t x, y;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }

  /* Place P1, P2 */
  p1pos[X] = 3;
  p1pos[Y] = 7;
  p1dir = RIGHT;

  p2pos[X] = 12;
  p2pos[Y] = 8;
  p2dir = LEFT;

  /* Draw the initial cycles */
  SetPixel(p1pos[X], p1pos[Y], P1_COLOR);
  SetPixel(p2pos[X], p2pos[Y], P2_COLOR);

  /* If there was a winner, draw some lights */
  if((losers & 0x01) && (losers & 0x02)) {
    ;/* everyone loses! */
  }
  else if(losers & 0x01) {
    for(i=0; i < BOARD_SIZE; i++) {
      SetPixel(BOARD_SIZE-1, i, P2_COLOR);
      showingWinningLEDs = 1;
    }
  }
  else if(losers & 0x02) {
    for(i=0; i < BOARD_SIZE; i++) {
      SetPixel(0, i, P1_COLOR);
      showingWinningLEDs = 1;
    }
  }
}

/**
 * Process input from the controllers. Only the D-Pad can drive cycles
 *
 * @param p1	The 32 bits of player 1 input, to be masked into buttons
 * @param p2	The 32 bits of player 2 input, to be masked into buttons
 */
void Lightcycle::ProcessInput(int32_t p1, int32_t p2)
{
  if(!started) {
    return;
  }
  /* Switch position based on buttons */
  if((GET_BUTTONS(p1) & UP)) {
    if(p1dir != DOWN) {
      p1dir = UP;
    }
  }
  else if((GET_BUTTONS(p1) & DOWN)) {
    if(p1dir != UP) {
      p1dir = DOWN;
    }
  }
  else if((GET_BUTTONS(p1) & LEFT)) {
    if(p1dir != RIGHT) {
      p1dir = LEFT;
    }
  }
  else if((GET_BUTTONS(p1) & RIGHT)) {
    if(p1dir != LEFT) {
      p1dir = RIGHT;
    }
  }

  if((GET_BUTTONS(p2) & UP)) {
    if(p2dir != DOWN) {
      p2dir = UP;
    }
  }
  else if((GET_BUTTONS(p2) & DOWN)) {
    if(p2dir != UP) {
      p2dir = DOWN;
    }
  }
  else if((GET_BUTTONS(p2) & LEFT)) {
    if(p2dir != RIGHT) {
      p2dir = LEFT;
    }
  }
  else if((GET_BUTTONS(p2) & RIGHT)) {
    if(p2dir != LEFT) {
      p2dir = RIGHT;
    }
  }
}
