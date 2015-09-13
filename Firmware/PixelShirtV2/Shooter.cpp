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

#include "Shooter.h"

int8_t shipShape[4][2] = {
		{-1,BOARD_SIZE-1},
		{0,BOARD_SIZE-1},
		{1,BOARD_SIZE-1},
		{0,BOARD_SIZE-2}};
#define PLAYER_SCALAR 16

#define BULLET_COLOR 0x151515
#define ENEMY_COLOR  0x004000

/**
 * Default constructor, resets the game with the initialization flag
 */
Shooter::Shooter(void)
{
  ResetGame(1, 0);
}

/**
 * If the game is finished, display the score until resetTimer expires.
 * Move all bullets and all invaders laterally or down, depending on timers.
 * Remove any invader hit by a bullet, and the bullet that hit it.
 */
void Shooter::UpdatePhysics(void)
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
      if(player1.bullets[i][1] != -1 &&
    		  player1.bullets[i][1] != BOARD_SIZE - 2) {
        SetPixel(player1.bullets[i][0], player1.bullets[i][1], EMPTY_COLOR);
      }
      if(player2.bullets[i][1] != -1 &&
    		  player2.bullets[i][1] != BOARD_SIZE - 2) {
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
        if(GetPixel(player1.bullets[i][0], player1.bullets[i][1]) ==
        		ENEMY_COLOR) {
          /* Clear the pixel, invalidate the bullet, increment the score */
          SetPixel(player1.bullets[i][0], player1.bullets[i][1], EMPTY_COLOR);
          KillEnemy();
          player1.bullets[i][1] = -1;
        }
        else {
          SetPixel(player1.bullets[i][0], player1.bullets[i][1], BULLET_COLOR);
        }
      }
      if(player2.bullets[i][1] != -1) {
        if(GetPixel(player2.bullets[i][0], player2.bullets[i][1]) ==
        		ENEMY_COLOR) {
          /* Clear the pixel, invalidate the bullet, increment the score */
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
 * Called when an invader is killed. Increments the score, increases speed,
 * and spawns a new wave if necessary
 */
void Shooter::KillEnemy(void)
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
 * Always reset the player's position and bullet state. Spawn the initial
 * wave of invaders.
 *
 * @param isInit	Ignored
 * @param whoWon	Ignored
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
 * Clear the display, then draw a new wave of space invaders
 */
void Shooter::SpawnWave(void)
{
  uint8_t x, y;

  /* Clear the display */
  for(x=0; x < BOARD_SIZE; x++) {
    for(y=0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }

  /* Draw invaders */
  for(x = 2; x < BOARD_SIZE - 2; x += 2) {
    for(y = 0; y < BOARD_SIZE - 6; y += 2) {
      SetPixel(x + ((y%4==2) ? 1 : 0),y, ENEMY_COLOR);
    }
  }
}

/**
 * The joysticks control the lateral motion of the ships, and the down buttons
 * fire bullets. The player's ships cannot pass through each other.
 *
 * @param p1	Player 1's input, to be masked into joystick and button
 * @param p2	Player 2's input, to be masked into joystick and button
 */
void Shooter::ProcessInput(int32_t p1, int32_t p2)
{
  uint8_t i;

  if(resetTimer == 0) {
    ClearPlayers();

    /* Handle position */
    if((GET_X_AXIS(p1)) < 512 - DEAD_ZONE ||
    		512 + DEAD_ZONE < (GET_X_AXIS(p1))) {
      player1.position += (((GET_X_AXIS(p1)) - 512) / 32);
      if(player1.position < 0) {
        player1.position = 0;
      }
      else if(player1.position > player2.position - PLAYER_SCALAR) {
        player1.position = player2.position - PLAYER_SCALAR;
      }
    }

    if((GET_X_AXIS(p2)) < 512 - DEAD_ZONE ||
    		512 + DEAD_ZONE < (GET_X_AXIS(p2))) {
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
        /* If there is space, store the bullet position and start the clock */
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
        /* If there is space, store the bullet position and start the clock */
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
 * Move all of the invaders to either the right or the left. If any intersect
 * with a player's ship, that's game over.
 */
void Shooter::ShiftBoard(void)
{
  uint8_t canMove = TRUE;
  uint8_t x, y;

  switch(activeDirection) {
    case LEFT : {
        /* Check for left side boundary condition */
        ClearPlayers();
        for(y = 0; y < BOARD_SIZE; y++) {
          if(IsPixelLit(0, y)) {
            canMove = FALSE;
          }
        }
        /* Redraw the player, if it intersects with a ship, game over */
        if(FALSE == DrawPlayers()) {
          GameOver();
        }

        if(!canMove) {
          activeDirection = RIGHT; /* Reverse direction, for next time */
          ClearPlayers();
          DropBoard();
          /* Redraw the player, if it intersects with a ship, game over */
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
          /* Redraw the player, if it intersects with a ship, game over */
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
        /* Redraw the player, if it intersects with a ship, game over */
        if(FALSE == DrawPlayers()) {
          GameOver();
        }

        if(!canMove) {
          activeDirection = LEFT; /* Reverse direction, for next time */
          ClearPlayers();
          DropBoard();
          /* Redraw the player, if it intersects with a ship, game over */
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
          /* Redraw the player, if it intersects with a ship, game over */
          if(FALSE == DrawPlayers()) {
            GameOver();
          }
        }
        break;
      }
  }
}

/**
 * Move all of the invaders one pixel towards the player's ships.
 */
void Shooter::DropBoard(void)
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
 * Clear the player's ship's pixels from the display
 */
void Shooter::ClearPlayers(void)
{
  uint8_t i;

  /* For each pixel in the ship */
  for(i = 0; i < 4; i++) {
    /* If the pixel is in bounds */
    if(0 <= (player1.position / PLAYER_SCALAR) + shipShape[i][0]
        && (player1.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((player1.position / PLAYER_SCALAR) + shipShape[i][0],
    		  shipShape[i][1], EMPTY_COLOR);
    }

    /* And then again, for player2. If the pixel is in bounds */
    if(0 <= (player2.position / PLAYER_SCALAR) + shipShape[i][0]
        && (player2.position / PLAYER_SCALAR) + shipShape[i][0] < BOARD_SIZE) {
      /* Clear it */
      SetPixel((player2.position / PLAYER_SCALAR) + shipShape[i][0],
    		  shipShape[i][1], EMPTY_COLOR);
    }
  }
}

/**
 * Draw the player's ship's pixels on the display
 *
 * @return FALSE if the ships intersect with an invader, TRUE otherwise
 */
uint8_t Shooter::DrawPlayers(void)
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
      SetPixel((player1.position / PLAYER_SCALAR) + shipShape[i][0],
    		  shipShape[i][1], P1_COLOR);
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
      SetPixel((player2.position / PLAYER_SCALAR) + shipShape[i][0],
    		  shipShape[i][1], P2_COLOR);
    }
  }

  return TRUE;
}

/**
 * Called when an invader intersects with a player's ship. Starts the score
 * display for five seconds.
 */
void Shooter::GameOver(void)
{
  resetTimer = IRQ_HZ * 5;
}

/**
 * Counts the number invaders, and returns it
 *
 * @return	The current number of invaders on the display
 */
uint8_t Shooter::NumEnemies(void)
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
