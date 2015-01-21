#include "Shooter.h"

int8_t shipShape[4][2] = {{-1,BOARD_SIZE-1},{0,BOARD_SIZE-1},{1,BOARD_SIZE-1},{0,BOARD_SIZE-2}};
#define PLAYER_SCALAR 16

void Shooter::UpdatePhysics( uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t i;

  if(resetTimer > 0) {
    resetTimer--;
    DisplayScore(field, score);
    if(resetTimer == 0) {
      ResetGame(field, 0, 0);
    }
  }
  else {
    /* Clear bullets */
    for (i = 0; i < NUM_BULLETS; i++) {
      if(p1.bullets[i][1] != -1 && p1.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(p1.bullets[i][0], p1.bullets[i][1], 0, 0, 0);
      }
      if(p2.bullets[i][1] != -1 && p2.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(p2.bullets[i][0], p2.bullets[i][1], 0, 0, 0);
      }
    }

    /* Should the invaders move? */
    moveTimer--;
    if(moveTimer == 0) {
      moveTimer = currentMovementSpeed;
      ShiftBoard(field);
    }

    /* Cooldown for shooting */
    if(p1.shotClock > 0) {
      p1.shotClock--;
    }
    if(p2.shotClock > 0) {
      p2.shotClock--;
    }

    /* Move bullets */
    for (i = 0; i < NUM_BULLETS; i++) {
      if(p1.bullets[i][1] != -1) {
        /* Move the bullet, if it is == -1, it will invalidate */
        p1.bullets[i][1]--;
      }
      if(p2.bullets[i][1] != -1) {
        /* Move the bullet, if it is == -1, it will invalidate */
        p2.bullets[i][1]--;
      }
    }

    /* Draw bullets, checking for collisions */
    for (i = 0; i < NUM_BULLETS; i++) {
      if(p1.bullets[i][1] != -1) {
        if(field[p1.bullets[i][0]][p1.bullets[i][1]][0] == 0x40) {
          /* KILL SHOT, clear the pixel, invalidate the bullet, increment the score */
          SetPixel(p1.bullets[i][0], p1.bullets[i][1], 0,0,0);
          KillEnemy(field);
          p1.bullets[i][1] = -1;
        }
        else {
          SetPixel(p1.bullets[i][0], p1.bullets[i][1], 0x15, 0x15, 0x15);
        }
      }
      if(p2.bullets[i][1] != -1) {
        if(field[p2.bullets[i][0]][p2.bullets[i][1]][0] == 0x40) {
          /* KILL SHOT, clear the pixel, invalidate the bullet, increment the score */
          SetPixel(p2.bullets[i][0], p2.bullets[i][1], 0,0,0);
          KillEnemy(field);
          p2.bullets[i][1] = -1;
        }
        else {
          SetPixel(p2.bullets[i][0], p2.bullets[i][1], 0x15, 0x15, 0x15);
        }
      }
    }
  }
}

void Shooter::KillEnemy(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  score++;
  if(score == 250) {
    GameOver(field);
  }

  /* For each row killed, make dudes go faster */
  uint8_t enemies = NumEnemies(field);
  if(enemies % 5 == 0 && currentMovementSpeed > 3) {
    currentMovementSpeed--;
  }
  if(enemies == 0) {
    SpawnWave(field);
  }
}

void Shooter::ResetGame(
  uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
  __attribute__((unused)) uint8_t isInit,
  __attribute__((unused)) uint8_t whoWon)
{
  uint8_t i;

  currentMovementSpeed = IRQ_HZ;
  score = 0; /* wave speed depends on score */
  SpawnWave(field);

  moveTimer = IRQ_HZ;
  activeDirection = RIGHT;

  p1.position = PLAYER_SCALAR * 3;
  p2.position = PLAYER_SCALAR * 12;

  p1.shotClock = 0;
  p2.shotClock = 0;

  for(i=0; i < NUM_BULLETS; i++) {
    p1.bullets[i][1] = -1;
    p2.bullets[i][1] = -1;
  }

  if(FALSE == DrawPlayers(field)) {
    GameOver(field);
  }

  resetTimer = 0;
}

void Shooter::SpawnWave(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t x, y;

  for(x=0; x < BOARD_SIZE; x++) {
    for(y=0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, 0, 0, 0);
    }
  }

  for(x = 2; x < BOARD_SIZE - 2; x += 2) {
    for(y = 0; y < BOARD_SIZE - 6; y += 2) {
      SetPixel(x + ((y%4==2) ? 1 : 0),y,0x40, 0x0, 0x0);
    }
  }
}

void Shooter::ProcessInput(
  __attribute__((unused))  uint8_t field[BOARD_SIZE][BOARD_SIZE][3],
  int32_t p1ax,
  __attribute__((unused)) int32_t p1ay,
  __attribute__((unused)) int8_t p1bl,
  __attribute__((unused)) int8_t p1br,
  __attribute__((unused)) int8_t p1bu,
  int8_t p1bd,
  int32_t p2ax,
  __attribute__((unused)) int32_t p2ay,
  __attribute__((unused)) int8_t p2bl,
  __attribute__((unused)) int8_t p2br,
  __attribute__((unused)) int8_t p2bu,
  int8_t p2bd)
{
  uint8_t i;

  if(resetTimer == 0) {
    ClearPlayers(field);

    /* Handle position */
    if(p1ax < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p1ax) {
      p1.position += ((p1ax - 512) / 32);
      if(p1.position < 0) {
        p1.position = 0;
      }
      else if(p1.position > p2.position - PLAYER_SCALAR) {
        p1.position = p2.position - PLAYER_SCALAR;
      }
    }

    if(p2ax < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p2ax) {
      p2.position += ((p2ax - 512) / 32);
      if(p2.position < p1.position + PLAYER_SCALAR) {
        p2.position = p1.position + PLAYER_SCALAR;
      }
      else if(p2.position > PLAYER_SCALAR * (BOARD_SIZE - 1)) {
        p2.position = PLAYER_SCALAR * (BOARD_SIZE - 1);
      }
    }

    if(FALSE == DrawPlayers(field)) {
      GameOver(field);
    }

    /* Handle bullets */
    /* If the player presses the button, and the timer has expired */
    if(p1bd && p1.shotClock == 0) {
      /* Find a place to store the bullet */
      for( i = 0; i < NUM_BULLETS; i++) {
        /* If there is space, store the bullet position and start the clock again */
        if(p1.bullets[i][1] == -1) {
          p1.bullets[i][0] = p1.position / PLAYER_SCALAR;
          p1.bullets[i][1] = BOARD_SIZE - 2;
          p1.shotClock = IRQ_HZ / 4;
          break;
        }
      }
    }

    /* If the player presses the button, and the timer has expired */
    if(p2bd && p2.shotClock == 0) {
      /* Find a place to store the bullet */
      for( i = 0; i < NUM_BULLETS; i++) {
        /* If there is space, store the bullet position and start the clock again */
        if(p2.bullets[i][1] == -1) {
          p2.bullets[i][0] = p2.position / PLAYER_SCALAR;
          p2.bullets[i][1] = BOARD_SIZE - 2;
          p2.shotClock = IRQ_HZ / 4;
          break;
        }
      }
    }
  }
}

void Shooter::ShiftBoard(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t canMove = TRUE;
  uint8_t x, y;

  switch(activeDirection) {
    case LEFT : {
        /* Check for left side boundry condition */
        ClearPlayers(field);
        for(y = 0; y < BOARD_SIZE; y++) {
          if(IsPixelLit(0, y)) {
            canMove = FALSE;
          }
        }
        if(FALSE == DrawPlayers(field)) {
          GameOver(field);
        }

        if(!canMove) {
          activeDirection = RIGHT; /* Reverse direction, for next time */
          ClearPlayers(field);
          DropBoard(field);
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers(field)) {
            GameOver(field);
          }
        }
        else {
          ClearPlayers(field);
          /* Shift the columns leftward */
          for(x = 0; x < BOARD_SIZE - 1; x++) {
            for(y = 0; y < BOARD_SIZE; y++) {
              field[x][y][0] = field[x+1][y][0];
              field[x][y][1] = field[x+1][y][1];
              field[x][y][2] = field[x+1][y][2];
            }
          }
          /* Clear the last column */
          for(y = 0; y < BOARD_SIZE; y++) {
            SetPixel(BOARD_SIZE-1, y, 0, 0, 0);
          }
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers(field)) {
            GameOver(field);
          }
        }
        break;
      }
    case RIGHT : {
        /* Check for right side boundry condition */
        ClearPlayers(field);
        for(y = 0; y < BOARD_SIZE; y++) {
          if(IsPixelLit(BOARD_SIZE - 1, y)) {
            canMove = FALSE;
          }
        }
        if(FALSE == DrawPlayers(field)) {
          GameOver(field);
        }

        if(!canMove) {
          activeDirection = LEFT; /* Reverse direction, for next time */
          ClearPlayers(field);
          DropBoard(field);
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers(field)) {
            GameOver(field);
          }
        }
        else {
          ClearPlayers(field);
          /* Shift the columns rightward */
          for(x = BOARD_SIZE - 1; x > 0; x--) {
            for(y = 0; y < BOARD_SIZE; y++) {
              field[x][y][0] = field[x-1][y][0];
              field[x][y][1] = field[x-1][y][1];
              field[x][y][2] = field[x-1][y][2];
            }
          }
          /* Clear the last column */
          for(y = 0; y < BOARD_SIZE; y++) {
            SetPixel(0, y, 0, 0, 0);
          }
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers(field)) {
            GameOver(field);
          }
        }
        break;
      }
  }
}

void Shooter::DropBoard(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t x, y;
  for(y = BOARD_SIZE - 1; y > 0; y--) {
    for(x = 0; x < BOARD_SIZE; x++) {
      field[x][y][0] = field[x][y-1][0];
      field[x][y][1] = field[x][y-1][1];
      field[x][y][2] = field[x][y-1][2];
    }
  }
  for(x = 0; x < BOARD_SIZE; x++) {
    SetPixel(x, 0, 0, 0, 0);
  }
}

void Shooter::ClearPlayers(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (p1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((p1.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1], 0, 0,
               0);
    }

    /* And then again, for P2. If the pixel is in bounds */
    if(0 <= (p2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((p2.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1], 0, 0,
               0);
    }
  }
}

uint8_t Shooter::DrawPlayers(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (p1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* If the pixel is already lit, i.e. touched by an enemy */
      if(field[(p1.position / PLAYER_SCALAR) + shipShape[i][0]][shipShape[i][1]][0] >
          0) {
        return FALSE;
      }
      /* Otherwise, draw that part of the ship */
      field[(p1.position / PLAYER_SCALAR) + shipShape[i][0]][shipShape[i][1]][1] =
        0x40;
    }
  }

  /* And then again, for P2. If the pixel is in bounds */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (p2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* If the pixel is already lit, i.e. touched by an enemy */
      if(field[(p2.position / PLAYER_SCALAR) + shipShape[i][0]][shipShape[i][1]][0] >
          0) {
        return FALSE;
      }
      /* Otherwise, draw that part of the ship */
      field[(p2.position / PLAYER_SCALAR) + shipShape[i][0]][shipShape[i][1]][2] =
        0x40;
    }
  }

  return TRUE;
}

void Shooter::GameOver(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  resetTimer = IRQ_HZ * 5;
}

uint8_t Shooter::NumEnemies(uint8_t field[BOARD_SIZE][BOARD_SIZE][3])
{
  uint8_t x, y, enemies = 0;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      if(field[x][y][0] == 0x40) {
        enemies++;
      }
    }
  }
  return enemies;
  return 0;
}
