#include "Shooter.h"

int8_t shipShape[4][2] = {{-1,BOARD_SIZE-1},{0,BOARD_SIZE-1},{1,BOARD_SIZE-1},{0,BOARD_SIZE-2}};
#define PLAYER_SCALAR 16

#define BULLET_COLOR 0x151515
#define ENEMY_COLOR  0x004000

/**
 * TODO
 */
Shooter::Shooter()
{
  ResetGame(1, 0);
}

/**
 * TODO
 */
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
      if(player1.bullets[i][1] != -1 && player1.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(player1.bullets[i][0], player1.bullets[i][1], EMPTY_COLOR);
      }
      if(player2.bullets[i][1] != -1 && player2.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(player2.bullets[i][0], player2.bullets[i][1], EMPTY_COLOR);
      }
    }

    /* Should the invaders move? */
    moveTimer--;
    if(moveTimer == 0) {
      moveTimer = currentMovementSpeed;
      ShiftBoard();
    }

    /* Cooldown for shooting */
    if(player1.shotClock > 0) {
      player1.shotClock--;
    }
    if(player2.shotClock > 0) {
      player2.shotClock--;
    }

    /* Move bullets */
    for (i = 0; i < NUM_BULLETS; i++) {
      if(player1.bullets[i][1] != -1) {
        /* Move the bullet, if it is == -1, it will invalidate */
        player1.bullets[i][1]--;
      }
      if(player2.bullets[i][1] != -1) {
        /* Move the bullet, if it is == -1, it will invalidate */
        player2.bullets[i][1]--;
      }
    }

    /* Draw bullets, checking for collisions */
    for (i = 0; i < NUM_BULLETS; i++) {
      if(player1.bullets[i][1] != -1) {
        if(GetPixel(player1.bullets[i][0], player1.bullets[i][1]) == ENEMY_COLOR) {
          /* KILL SHOT, clear the pixel, invalidate the bullet, increment the score */
          SetPixel(player1.bullets[i][0], player1.bullets[i][1], EMPTY_COLOR);
          KillEnemy();
          player1.bullets[i][1] = -1;
        }
        else {
          SetPixel(player1.bullets[i][0], player1.bullets[i][1], BULLET_COLOR);
        }
      }
      if(player2.bullets[i][1] != -1) {
        if(GetPixel(player2.bullets[i][0], player2.bullets[i][1]) == ENEMY_COLOR) {
          /* KILL SHOT, clear the pixel, invalidate the bullet, increment the score */
          SetPixel(player2.bullets[i][0], player2.bullets[i][1], EMPTY_COLOR);
          KillEnemy();
          player2.bullets[i][1] = -1;
        }
        else {
          SetPixel(player2.bullets[i][0], player2.bullets[i][1], BULLET_COLOR);
        }
      }
    }
  }
}

/**
 * TODO
 */
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

/**
 * TODO
 * @param isInit
 * @param whoWon
 */
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

  player1.position = PLAYER_SCALAR * 3;
  player2.position = PLAYER_SCALAR * 12;

  player1.shotClock = 0;
  player2.shotClock = 0;

  for(i=0; i < NUM_BULLETS; i++) {
    player1.bullets[i][1] = -1;
    player2.bullets[i][1] = -1;
  }

  if(FALSE == DrawPlayers()) {
    GameOver();
  }

  resetTimer = 0;
}

/**
 * TODO
 */
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

/**
 * TODO
 * @param p1
 * @param p2
 */
void Shooter::ProcessInput(int32_t p1, int32_t p2)
{
  uint8_t i;

  if(resetTimer == 0) {
    ClearPlayers();

    /* Handle position */
    if((GET_X_AXIS(p1)) < 512 - DEAD_ZONE || 512 + DEAD_ZONE < (GET_X_AXIS(p1))) {
      player1.position += (((GET_X_AXIS(p1)) - 512) / 32);
      if(player1.position < 0) {
        player1.position = 0;
      }
      else if(player1.position > player2.position - PLAYER_SCALAR) {
        player1.position = player2.position - PLAYER_SCALAR;
      }
    }

    if((GET_X_AXIS(p2)) < 512 - DEAD_ZONE || 512 + DEAD_ZONE < (GET_X_AXIS(p2))) {
      player2.position += (((GET_X_AXIS(p2)) - 512) / 32);
      if(player2.position < player1.position + PLAYER_SCALAR) {
        player2.position = player1.position + PLAYER_SCALAR;
      }
      else if(player2.position > PLAYER_SCALAR * (BOARD_SIZE - 1)) {
        player2.position = PLAYER_SCALAR * (BOARD_SIZE - 1);
      }
    }

    if(FALSE == DrawPlayers()) {
      GameOver();
    }

    /* Handle bullets */
    /* If the player presses the button, and the timer has expired */
    if((GET_BUTTONS(p1) & DOWN) && player1.shotClock == 0) {
      /* Find a place to store the bullet */
      for( i = 0; i < NUM_BULLETS; i++) {
        /* If there is space, store the bullet position and start the clock again */
        if(player1.bullets[i][1] == -1) {
          player1.bullets[i][0] = player1.position / PLAYER_SCALAR;
          player1.bullets[i][1] = BOARD_SIZE - 2;
          player1.shotClock = IRQ_HZ / 4;
          break;
        }
      }
    }

    /* If the player presses the button, and the timer has expired */
    if((GET_BUTTONS(p2) & DOWN) && player2.shotClock == 0) {
      /* Find a place to store the bullet */
      for( i = 0; i < NUM_BULLETS; i++) {
        /* If there is space, store the bullet position and start the clock again */
        if(player2.bullets[i][1] == -1) {
          player2.bullets[i][0] = player2.position / PLAYER_SCALAR;
          player2.bullets[i][1] = BOARD_SIZE - 2;
          player2.shotClock = IRQ_HZ / 4;
          break;
        }
      }
    }
  }
}

/**
 * TODO
 */
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

/**
 * TODO
 */
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

/**
 * TODO
 */
void Shooter::ClearPlayers()
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (player1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (player1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((player1.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1],
               EMPTY_COLOR);
    }

    /* And then again, for player2. If the pixel is in bounds */
    if(0 <= (player2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (player2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((player2.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1],
               EMPTY_COLOR);
    }
  }
}

/**
 * TODO
 * @return
 */
uint8_t Shooter::DrawPlayers()
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (player1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (player1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* If the pixel is already lit red, i.e. touched by an enemy */
      if(GetPixel((player1.position / PLAYER_SCALAR) + shipShape[i][0],
                  shipShape[i][1]) == ENEMY_COLOR) {
        return FALSE;
      }
      /* Otherwise, draw that part of the ship */
      SetPixel((player1.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1],
               P1_COLOR);
    }
  }

  /* And then again, for player2. If the pixel is in bounds */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (player2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (player2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* If the pixel is already lit red, i.e. touched by an enemy */
      if(GetPixel((player2.position / PLAYER_SCALAR) + shipShape[i][0],
                  shipShape[i][1]) == ENEMY_COLOR) {
        return FALSE;
      }
      /* Otherwise, draw that part of the ship */
      SetPixel((player2.position / PLAYER_SCALAR) + shipShape[i][0], shipShape[i][1],
               P2_COLOR);
    }
  }

  return TRUE;
}

/**
 * TODO
 */
void Shooter::GameOver()
{
  resetTimer = IRQ_HZ * 5;
}

/**
 * TODO
 * @return
 */
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
