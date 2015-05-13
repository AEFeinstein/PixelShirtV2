#include "Shooter.h"

int8_t shipShape[4][2] = {{-1,BOARD_SIZE-1},{0,BOARD_SIZE-1},{1,BOARD_SIZE-1},{0,BOARD_SIZE-2}};
#define PLAYER_SCALAR 16

#define BULLET_COLOR 0x151515
#define ENEMY_COLOR  0x004000

void Shooter::UpdatePhysics( )
{
  uint8_t i;

  if(resetTimer > 0) {
    resetTimer--;
    DisplayScore(score, SCORE_COLOR);
    if(resetTimer == 0) {
      ResetGame(0, 0);
    }
  }
  else {
    /* Clear bullets */
    for (i = 0; i < NUM_BULLETS; i++) {
      if(p1.bullets[i][1] != -1 && p1.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(p1.bullets[i][0], p1.bullets[i][1], EMPTY_COLOR);
      }
      if(p2.bullets[i][1] != -1 && p2.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(p2.bullets[i][0], p2.bullets[i][1], EMPTY_COLOR);
      }
    }

    /* Should the invaders move? */
    moveTimer--;
    if(moveTimer == 0) {
      moveTimer = currentMovementSpeed;
      ShiftBoard();
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
        if(GetPixel(p1.bullets[i][0], p1.bullets[i][1]) == ENEMY_COLOR) {
          /* KILL SHOT, clear the pixel, invalidate the bullet, increment the score */
          SetPixel(p1.bullets[i][0], p1.bullets[i][1], EMPTY_COLOR);
          KillEnemy();
          p1.bullets[i][1] = -1;
        }
        else {
          SetPixel(p1.bullets[i][0], p1.bullets[i][1], BULLET_COLOR);
        }
      }
      if(p2.bullets[i][1] != -1) {
        if(GetPixel(p2.bullets[i][0], p2.bullets[i][1]) == ENEMY_COLOR) {
          /* KILL SHOT, clear the pixel, invalidate the bullet, increment the score */
          SetPixel(p2.bullets[i][0], p2.bullets[i][1], EMPTY_COLOR);
          KillEnemy();
          p2.bullets[i][1] = -1;
        }
        else {
          SetPixel(p2.bullets[i][0], p2.bullets[i][1], BULLET_COLOR);
        }
      }
    }
  }
}

void Shooter::KillEnemy()
{
  score++;
  if(score == 250) {
    GameOver();
  }

  /* For each row killed, make dudes go faster */
  uint8_t enemies = NumEnemies();
  if(enemies % 5 == 0 && currentMovementSpeed > 3) {
    currentMovementSpeed--;
  }
  if(enemies == 0) {
    SpawnWave();
  }
}

void Shooter::ResetGame(
  
  __attribute__((unused)) uint8_t isInit,
  __attribute__((unused)) uint8_t whoWon)
{
  uint8_t i;

  currentMovementSpeed = IRQ_HZ;
  score = 0; /* wave speed depends on score */
  SpawnWave();

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

  if(FALSE == DrawPlayers()) {
    GameOver();
  }

  resetTimer = 0;
}

void Shooter::SpawnWave()
{
  uint8_t x, y;

  for(x=0; x < BOARD_SIZE; x++) {
    for(y=0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }

  for(x = 2; x < BOARD_SIZE - 2; x += 2) {
    for(y = 0; y < BOARD_SIZE - 6; y += 2) {
      SetPixel(x + ((y%4==2) ? 1 : 0),y, ENEMY_COLOR);
    }
  }
}

void Shooter::ProcessInput(
  __attribute__((unused))  
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
    ClearPlayers();

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

    if(FALSE == DrawPlayers()) {
      GameOver();
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

void Shooter::ShiftBoard()
{
  uint8_t canMove = TRUE;
  uint8_t x, y;

  switch(activeDirection) {
    case LEFT : {
        /* Check for left side boundry condition */
        ClearPlayers();
        for(y = 0; y < BOARD_SIZE; y++) {
          if(IsPixelLit(0, y)) {
            canMove = FALSE;
          }
        }
        if(FALSE == DrawPlayers()) {
          GameOver();
        }

        if(!canMove) {
          activeDirection = RIGHT; /* Reverse direction, for next time */
          ClearPlayers();
          DropBoard();
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers()) {
            GameOver();
          }
        }
        else {
          ClearPlayers();
          /* Shift the columns leftward */
          for(x = 0; x < BOARD_SIZE - 1; x++) {
            for(y = 0; y < BOARD_SIZE; y++) {
              SetPixel(x, y, GetPixel(x+1, y));
            }
          }
          /* Clear the last column */
          for(y = 0; y < BOARD_SIZE; y++) {
            SetPixel(BOARD_SIZE-1, y, EMPTY_COLOR);
          }
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers()) {
            GameOver();
          }
        }
        break;
      }
    case RIGHT : {
        /* Check for right side boundry condition */
        ClearPlayers();
        for(y = 0; y < BOARD_SIZE; y++) {
          if(IsPixelLit(BOARD_SIZE - 1, y)) {
            canMove = FALSE;
          }
        }
        if(FALSE == DrawPlayers()) {
          GameOver();
        }

        if(!canMove) {
          activeDirection = LEFT; /* Reverse direction, for next time */
          ClearPlayers();
          DropBoard();
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers()) {
            GameOver();
          }
        }
        else {
          ClearPlayers();
          /* Shift the columns rightward */
          for(x = BOARD_SIZE - 1; x > 0; x--) {
            for(y = 0; y < BOARD_SIZE; y++) {
              SetPixel(x, y , GetPixel(x-1, y));
            }
          }
          /* Clear the last column */
          for(y = 0; y < BOARD_SIZE; y++) {
            SetPixel(0, y, EMPTY_COLOR);
          }
          /* Try to redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers()) {
            GameOver();
          }
        }
        break;
      }
  }
}

void Shooter::DropBoard()
{
  uint8_t x, y;
  for(y = BOARD_SIZE - 1; y > 0; y--) {
    for(x = 0; x < BOARD_SIZE; x++) {
      SetPixel(x, y, GetPixel(x, y-1));
    }
  }
  for(x = 0; x < BOARD_SIZE; x++) {
    SetPixel(x, 0, EMPTY_COLOR);
  }
}

void Shooter::ClearPlayers()
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (p1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((p1.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1], EMPTY_COLOR);
    }

    /* And then again, for P2. If the pixel is in bounds */
    if(0 <= (p2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((p2.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1], EMPTY_COLOR);
    }
  }
}

uint8_t Shooter::DrawPlayers()
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (p1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* If the pixel is already lit red, i.e. touched by an enemy */
      if(GetPixel((p1.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1]) == ENEMY_COLOR) {
        return FALSE;
      }
      /* Otherwise, draw that part of the ship */
      SetPixel((p1.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1], P1_COLOR);
    }
  }

  /* And then again, for P2. If the pixel is in bounds */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (p2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (p2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* If the pixel is already lit red, i.e. touched by an enemy */
      if(GetPixel((p2.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1]) == ENEMY_COLOR) {
        return FALSE;
      }
      /* Otherwise, draw that part of the ship */
      SetPixel((p2.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1], P2_COLOR);
    }
  }

  return TRUE;
}

void Shooter::GameOver()
{
  resetTimer = IRQ_HZ * 5;
}

uint8_t Shooter::NumEnemies()
{
  uint8_t x, y, enemies = 0;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      if(GetPixel(x, y) == ENEMY_COLOR) {
        enemies++;
      }
    }
  }
  return enemies;
}
