#include "SuperSquare.h"

void SuperSquare::UpdatePhysics( uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  int16_t i;

  if(resetTimer > 0) {
    resetTimer--;
    if(resetTimer == 0) {
      ResetGame(field, 0, 0);
    }
    return;
  }

  /* Clear all line drawings */
  for(i = 0; i < NUM_LINES; i++) {
    DrawLine(field, lines[i], 0,0,0);
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
          AddLine(random(4) + 1, currentVelocity);
          break;
        }
      case 1: {
          uint8_t newLinePosition = random(4);
          uint8_t otherNewLinePosition = (newLinePosition + random(3) + 1) % 4;
          AddLine(newLinePosition + 1, currentVelocity);
          AddLine(otherNewLinePosition + 1, currentVelocity);
          break;
        }
      default: {
          uint8_t newLinePosition = random(4);
          for(i = 0; i < 3; i++) {
            AddLine(newLinePosition + 1, currentVelocity);
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
    if(FALSE == DrawLine(field, lines[i], 0x40,0,0)) {

      /* Clear all lines */
      for(i = 0; i < NUM_LINES; i++) {
        INVALIDATE_LINE(lines[i]);
      }

      DisplayScore(field, score);
      resetTimer = IRQ_HZ * 5;
    }
  }
  score++;
}

void SuperSquare::ResetGame(
  uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
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
      SetPixel(i, j, 0,0,0);
    }
  }

  /* Draw the center square */
  SetPixel(7, 7, 0,0x40,0);
  SetPixel(7, 8, 0,0x40,0);
  SetPixel(8, 7, 0,0x40,0);
  SetPixel(8, 8, 0,0x40,0);

  /* Clear all lines */
  for(i = 0; i < NUM_LINES; i++) {
    INVALIDATE_LINE(lines[i]);
  }
}

void SuperSquare::ProcessInput(
  __attribute__((unused))  uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
  int32_t p1ax,
  __attribute__((unused)) int32_t p1ay,
  __attribute__((unused)) int8_t p1bl,
  __attribute__((unused)) int8_t p1br,
  __attribute__((unused)) int8_t p1bu,
  __attribute__((unused)) int8_t p1bd,
  __attribute__((unused)) int32_t p2ax,
  __attribute__((unused)) int32_t p2ay,
  __attribute__((unused)) int8_t p2bl,
  __attribute__((unused)) int8_t p2br,
  __attribute__((unused)) int8_t p2bu,
  __attribute__((unused)) int8_t p2bd)
{
  uint8_t newPos[2];
  uint8_t i;

  if(resetTimer > 0) {
    return;
  }

  /* Clear the old player pixel */
  PlacePlayerPixel(newPos);
  SetPixel(newPos[0], newPos[1], 0,0,0);

  /* Update the player position */
  playerPosition = (playerPosition + ((p1ax - 512) / 25));
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

    DisplayScore(field, score);
    resetTimer = IRQ_HZ * 5;
  }
  else {
    SetPixel(newPos[0], newPos[1], 0,0,0x40);
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

uint8_t SuperSquare::DrawLine( uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
                               Line line, uint8_t r, uint8_t g, uint8_t b)
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
          if(field[i][15-position][2] == 0x40 && (r|g|b) > 0) {
            return FALSE;
          }
          SetPixel(i, 15-position, r, g, b);
        }
        break;
      }
    case DOWN: {
        for(i = position; i < (BOARD_SIZE - 1 - position); i++) {
          if(field[i][position][2] == 0x40 && (r|g|b) > 0) {
            return FALSE;
          }
          SetPixel(i, position, r, g, b);
        }
        break;
      }
    case LEFT: {
        for(i = position; i < (BOARD_SIZE - 1 - position); i++) {
          if(field[15 - position][i][2] == 0x40 && (r|g|b) > 0) {
            return FALSE;
          }
          SetPixel(15 - position, i, r, g, b);
        }
        break;
      }
    case RIGHT: {
        for(i = position + 1; i < (BOARD_SIZE - position); i++) {
          if(field[position][i][2] == 0x40 && (r|g|b) > 0) {
            return FALSE;
          }
          SetPixel(position, i, r, g, b);
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
      lines[i].direction = direction;
      lines[i].position = 0;
      lines[i].velocity = velocity;
      return;
    }
  }
}

