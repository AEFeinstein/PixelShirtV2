#include "SuperSquare.h"

#define CENTER_COLOR 0x004000
#define LINE_COLOR   0x400000

SuperSquare::SuperSquare()
{
  ResetGame(1, 0);
}

void SuperSquare::UpdatePhysics( )
{
  int16_t i;

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

