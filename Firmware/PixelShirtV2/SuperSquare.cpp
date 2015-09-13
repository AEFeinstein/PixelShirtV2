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

#include "SuperSquare.h"

#define CENTER_COLOR 0x004000
#define LINE_COLOR   0x400000

/**
 * Default constructor. Resets the game with the init flag & no winner
 */
SuperSquare::SuperSquare(void)
{
  ResetGame(1, 0);
}

/**
 * This clears the screen, updates the positions of the lines, generate
 * new lines if it's time, and checks for collisions between lines and
 * the player.
 */
void SuperSquare::UpdatePhysics(void)
{
  int16_t i;

  /* This timer is for in between games. Decrement it if it's active */
  if(resetTimer > 0) {
    resetTimer--;
    if(resetTimer == 0) {
      ResetGame(0, 0);
    }
    return;
  }

  /* Clear all line drawings */
  for(i = 0; i < NUM_LINES; i++) {
    DrawLine(lines[i], EMPTY_COLOR);
  }

  /* Update line positions */
  for(i = 0; i < NUM_LINES; i++) {
    if(lines[i].position >= 0) {
      lines[i].position += lines[i].velocity;
    }
    /* Clear the line if it intersects with the center square */
    if(lines[i].position >= (7 * LINE_SPEED_SCALER)) {
      INVALIDATE_LINE(lines[i]);
    }
  }

  /* Generate all lines */
  newLineTimer--;
  if(newLineTimer == 0) {
    newLineTimer = (5 * LINE_SPEED_SCALER) / currentVelocity;

    /* How many lines to generate? */
    switch(linesDrawn / 6) {
      case 0: {
          AddLine(random(4), currentVelocity);
          break;
        }
      case 1: {
          uint8_t newLinePosition = random(4);
          uint8_t otherNewLinePosition = (newLinePosition + (random(3) + 1)) % 4;
          AddLine(newLinePosition, currentVelocity);
          AddLine(otherNewLinePosition, currentVelocity);
          break;
        }
      default: {
          uint8_t newLinePosition = random(4);
          for(i = 0; i < 3; i++) {
            AddLine(newLinePosition, currentVelocity);
            newLinePosition = (newLinePosition + 1) % 4;
          }
          break;
        }
    }

    currentVelocity += 2;
    if(currentVelocity > 42) {
      currentVelocity = 42;
    }

    linesDrawn++;
  }

  /* Draw all lines */
  for(i = 0; i < NUM_LINES; i++) {
    /* If there is a collision, display the score and start the reset timer */
    if(FALSE == DrawLine(lines[i], LINE_COLOR)) {

      /* Clear all lines */
      for(i = 0; i < NUM_LINES; i++) {
        INVALIDATE_LINE(lines[i]);
      }

      DisplayScore(score, SCORE_COLOR);
      resetTimer = IRQ_HZ * 5;
    }
  }
  score++;
}

/**
 * Clear the display, clear all lines, and draw the center square the
 * player rotates around
 *
 * @param isInit Unused, nothing special happens on initialization
 * @param whoWon Unused, it's a one player game
 */
void SuperSquare::ResetGame(
  __attribute__((unused)) uint8_t isInit,
  __attribute__((unused)) uint8_t whoWon)
{
  int16_t i, j;
  playerPosition = 0;
  currentVelocity = 10;
  newLineTimer = (7 * LINE_SPEED_SCALER) / currentVelocity;
  resetTimer = 0;
  score = 0;
  linesDrawn = 0;

  /* Clear the board */
  for(i = 0; i < BOARD_SIZE; i++) {
    for(j = 0; j < BOARD_SIZE; j++) {
      SetPixel(i, j, EMPTY_COLOR);
    }
  }

  /* Draw the center square */
  SetPixel(7, 7, CENTER_COLOR);
  SetPixel(7, 8, CENTER_COLOR);
  SetPixel(8, 7, CENTER_COLOR);
  SetPixel(8, 8, CENTER_COLOR);

  /* Clear all lines */
  for(i = 0; i < NUM_LINES; i++) {
    INVALIDATE_LINE(lines[i]);
  }
}

/**
 * Moves the player pixel around the center square according to the p1 joystick.
 * Checks for collisions with lines when the player moves
 *
 * @param p1 The 32 bits of player 1 input, to be masked into the joystick
 * @param p2 Unused, it's a one player game
 */
void SuperSquare::ProcessInput(int32_t p1, __attribute__((unused)) int32_t p2)
{
  uint8_t newPos[2];
  uint8_t i;

  if(resetTimer > 0) {
    return;
  }

  /* Clear the old player pixel */
  PlacePlayerPixel(newPos);
  SetPixel(newPos[0], newPos[1], EMPTY_COLOR);

  /* Update the player position */
  playerPosition = (playerPosition + (((GET_X_AXIS(p1)) - 512) / 25));
  if(playerPosition < 0) {
    playerPosition += 360;
  }
  else if(playerPosition > 359) {
    playerPosition -= 360;
  }

  /* Draw the new player pixel */
  PlacePlayerPixel(newPos);

  if(IsPixelLit(newPos[0], newPos[1])) {
    /* Clear all lines */
    for(i = 0; i < NUM_LINES; i++) {
      INVALIDATE_LINE(lines[i]);
    }

    DisplayScore(score, SCORE_COLOR);
    resetTimer = IRQ_HZ * 5;
  }
  else {
    SetPixel(newPos[0], newPos[1], P1_COLOR);
  }
}

/**
 * Given the player's angle around the center square, return the closest pixel.
 * Lookups are faster than math
 *
 * @param position The angle of the player, in degrees, around the center square
 */
void SuperSquare::PlacePlayerPixel(uint8_t position[])
{
  switch(playerPosition / 30) {
    case 0:
      position[0] = 8;
      position[1] = 6;
      break;
    case 1:
      position[0] = 9;
      position[1] = 6;
      break;
    case 2:
      position[0] = 9;
      position[1] = 7;
      break;
    case 3:
      position[0] = 9;
      position[1] = 8;
      break;
    case 4:
      position[0] = 9;
      position[1] = 9;
      break;
    case 5:
      position[0] = 8;
      position[1] = 9;
      break;
    case 6:
      position[0] = 7;
      position[1] = 9;
      break;
    case 7:
      position[0] = 6;
      position[1] = 9;
      break;
    case 8:
      position[0] = 6;
      position[1] = 8;
      break;
    case 9:
      position[0] = 6;
      position[1] = 7;
      break;
    case 10:
      position[0] = 6;
      position[1] = 6;
      break;
    case 11:
      position[0] = 7;
      position[1] = 6;
      break;
  }
}

/**
 * Given a line struct and color, draw it on the display, and check for
 * collisions
 *
 * @param line  The line to draw (position)
 * @param rgb The color of the line to draw
 * @return    FALSE if there was a collision, TRUE if there wasn't
 */
uint8_t SuperSquare::DrawLine( Line line, uint32_t rgb)
{
  int16_t i;

  /* Make sure the line is valid */
  if(!IS_LINE_VALID(line)) {
    return TRUE;
  }

  /* Scale position to board size */
  int16_t position = line.position / LINE_SPEED_SCALER;

  /* Draw the line */
  switch(line.direction) {
    case UP: {
        for(i = position + 1; i < (BOARD_SIZE - position); i++) {
          if(GetPixel(i, 15-position) == P1_COLOR && rgb > 0) {
            return FALSE;
          }
          SetPixel(i, 15-position, rgb);
        }
        break;
      }
    case DOWN: {
        for(i = position; i < (BOARD_SIZE - 1 - position); i++) {
          if(GetPixel(i, position) == P1_COLOR && rgb > 0) {
            return FALSE;
          }
          SetPixel(i, position, rgb);
        }
        break;
      }
    case LEFT: {
        for(i = position; i < (BOARD_SIZE - 1 - position); i++) {
          if(GetPixel(15 - position, i) == P1_COLOR && rgb > 0) {
            return FALSE;
          }
          SetPixel(15 - position, i, rgb);
        }
        break;
      }
    case RIGHT: {
        for(i = position + 1; i < (BOARD_SIZE - position); i++) {
          if(GetPixel(position, i) == P1_COLOR && rgb > 0) {
            return FALSE;
          }
          SetPixel(position, i, rgb);
        }
        break;
      }
  }
  return TRUE;
}

/**
 * Given a direction and initial velocity, add a line. This doesn't draw
 * anything, but stores the line in memory
 *
 * @param direction The initial direction of the line
 * @param velocity  The initial velocity of the line
 */
void SuperSquare::AddLine(uint8_t direction, uint16_t velocity)
{
  int16_t i;
  for(i = 0; i < NUM_LINES; i++) {
    if(!IS_LINE_VALID(lines[i])) {
      lines[i].direction = 1 << direction;
      lines[i].position = 0;
      lines[i].velocity = velocity;
      return;
    }
  }
}

