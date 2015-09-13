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

#include "Pong.h"

#define BALL_COLOR 0x004000

/**
 * Default constructor. Reset the game with the initialization flag
 * and no winners
 */
Pong::Pong(void)
{
  ResetGame(1, 0);
}

/**
 * Updates the ball's location and velocity, checking for collisions with the
 * walls or paddles. The ball's velocity is increased with every paddle hit.
 * Checks if a goal was scored, and updates the display.
 */
void Pong::UpdatePhysics(void)
{
  int16_t diff, rotation;
  /* If we're in a reset, don't update the game, just decrement the timer */
  if(restartTimer > 0) {
    restartTimer--;
  }
  else {
    /* Physics! */
    /* Update the ball's position */
    ballLoc[X] += (ballVel[X] * TIME_STEP);
    ballLoc[Y] += (ballVel[Y] * TIME_STEP);

    /* Check for collisions, win conditions. Nudge the ball if necessary */
    /* Top wall collision */
    if (ballLoc[Y] < 0 && ballVel[Y] < 0) {
      ballVel[Y] = -ballVel[Y];
      ballLoc[Y] = 0;
    }
    /* Bottom wall collision */
    else if (ballLoc[Y] >= BOARD_SIZE * S_M * V_M && ballVel[Y] > 0) {
      ballVel[Y] = -ballVel[Y];
      ballLoc[Y] = (BOARD_SIZE) * S_M * V_M - 1;
    }
    /* left paddle collision */
    else if (ballLoc[X] < 1 * S_M * V_M && ballVel[X] < 0
             && (paddleLocL <= ballLoc[Y]
                 && ballLoc[Y] < paddleLocL + PADDLE_SIZE)) {
      ballVel[X] = -ballVel[X];
      ballLoc[X] = 1 * S_M * V_M; /* right on the edge of the paddle */

      /* Increase speed on collision */
      IncreaseSpeed(37); /* remember, divided by V_M */

      /* Apply extra rotation depending on part of the paddle hit */
      /* range of diff is -PADDLE_SIZE/2 to PADDLE_SIZE/2 (+/- 8192) */
      diff = ballLoc[Y] - (paddleLocL + PADDLE_SIZE / 2);
      /* rotate 45deg at edge of paddle, 0deg in middle, linear in between */
      rotation = (EXTRA_ROTATION_ON_EDGE * diff) / (PADDLE_SIZE / 2);
      RotateBall(rotation);
    }
    /* right paddle collision */
    else if (ballLoc[X] >= (BOARD_SIZE - 1) * S_M * V_M && ballVel[X] > 0
             && (paddleLocR <= ballLoc[Y]
                 && ballLoc[Y] < paddleLocR + PADDLE_SIZE)) {
      ballVel[X] = -ballVel[X];
      ballLoc[X] = (BOARD_SIZE - 1) * S_M * V_M -
                   1; /* right on the edge of the paddle */

      /* Increase speed on collision */
      IncreaseSpeed(37); /* remember, divided by V_M */

      /* Apply extra rotation depending on part of the paddle hit */
      /* range of diff is -PADDLE_SIZE/2 to PADDLE_SIZE/2 (+/- 8192) */
      diff = ballLoc[Y] - (paddleLocR + PADDLE_SIZE / 2);
      /* rotate 45deg at edge of paddle, 0deg in middle, linear in between */
      rotation = (EXTRA_ROTATION_ON_EDGE * diff) / (PADDLE_SIZE / 2);
      RotateBall(-rotation);
    }
    /* left wall win */
    else if (ballLoc[X] < 0 && ballVel[X] < 0) {
      ResetGame(0, 1);
    }
    /* right wall win */
    else if (ballLoc[X] >= BOARD_SIZE * S_M * V_M && ballVel[X] > 0) {
      ResetGame(0, 0);
    }
  }
  DrawField();
}

/**
 * Always resets the timers, ball position, and ball velocity, Resets the
 * paddles if this isn't the initial reset. Serves to the winner.
 *
 * @param isInit  1 if this the initial reset, 0 otherwise
 * @param whoWon  The winner of last round, used to determine serve direction
 */
void Pong::ResetGame(  uint8_t isInit,
                       uint8_t whoWon)
{
  if (isInit) {
    paddleLocL = (V_M * S_M * BOARD_SIZE / 2) - (PADDLE_SIZE / 2);
    paddleLocR = (V_M * S_M * BOARD_SIZE / 2) - (PADDLE_SIZE / 2);
  }
  restartTimer = IRQ_HZ * 3;
  ballLoc[X] = V_M * S_M * (BOARD_SIZE / 2);
  ballLoc[Y] = V_M * S_M * (BOARD_SIZE / 2);
  if(whoWon) {
    ballVel[X] = V_M * 5;
  }
  else {
    ballVel[X] = -V_M * 5;
  }
  uint8_t initY = random(8);
  if(initY == 4) {
    initY++;
  }
  ballVel[Y] = (initY - 4) * V_M;
  DrawField();
}

/**
 * Rotate the ball's velocity vector using a rotation matrix.
 * Trigonometry uses a lookup table rather than a function.
 *
 * @param degree  The number of degrees to rotate the vector
 */
void Pong::RotateBall(int16_t degree)
{
  if (degree < 0) {
    degree += 360;
  }
  if(degree > 359) {
    degree -= 360;
  }

  ballVel[X] = (ballVel[X] * (int32_t)cos32[degree] - ballVel[Y] *
                (int32_t)sin32[degree]) / 32;
  ballVel[Y] = (ballVel[X] * (int32_t)sin32[degree] + ballVel[Y] *
                (int32_t)cos32[degree]) / 32;
}

/**
 * Multiply the ball's velocity vector to make it go faster.
 * Speed is capped at SPEED_LIMIT.
 *
 * @param speedM The amount to add to the ball's velocity vector.
 */
void Pong::IncreaseSpeed(int16_t speedM)
{
  ballVel[X] = (ballVel[X] * speedM) / V_M;
  if(ballVel[X] > SPEED_LIMIT) {
    ballVel[X] = SPEED_LIMIT;
  }
  else if(ballVel[X] < -SPEED_LIMIT) {
    ballVel[X] = -SPEED_LIMIT;
  }

  ballVel[Y] = (ballVel[Y] * speedM) / V_M;
  if(ballVel[Y] > SPEED_LIMIT) {
    ballVel[Y] = SPEED_LIMIT;
  }
  else if(ballVel[Y] < -SPEED_LIMIT) {
    ballVel[Y] = -SPEED_LIMIT;
  }
}

/**
 * Draw the display. Each pixel is iterated over and is assigned a color
 * based on ball, paddle, or empty.
 */
void Pong::DrawField(void)
{
  int16_t i, j;
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      if ((j == 0) && ((paddleLocL / (S_M * V_M)) <= i &&
                       i < (paddleLocL / (S_M * V_M)) + PADDLE_SIZE / (V_M * S_M))) {
        SetPixel(j, i, P1_COLOR);
      }
      else if ((j == (BOARD_SIZE - 1)) && ((paddleLocR / (S_M * V_M)) <= i &&
                                           i < (paddleLocR / (S_M * V_M)) + PADDLE_SIZE / (V_M * S_M))) {
        SetPixel(j, i, P2_COLOR);
      }
      else if (j == (ballLoc[X] / (S_M * V_M)) &&
               i == (ballLoc[Y] / (S_M * V_M))) {
        SetPixel(j, i, BALL_COLOR);
      }
      else {
        SetPixel(j, i, EMPTY_COLOR);
      }
    }
  }
}

/**
 * Process the input. Paddles are moved with the analog joystick. Inputs only
 * count if the joystick is outside of the DEAD_ZONE, so there isn't any drift
 * with no input.
 *
 * @param p1  The 32 bits of player 1 input, to be masked into the joystick
 * @param p2  The 32 bits of player 2 input, to be masked into the joystick
 */
void Pong::ProcessInput(int32_t p1, int32_t p2)
{
  if((GET_Y_AXIS(p1)) < 512 - DEAD_ZONE || 512 + DEAD_ZONE < (GET_Y_AXIS(p1))) {
    paddleLocL -= ((512 - (GET_Y_AXIS(p1))) * 3);
    if(paddleLocL < 0) {
      paddleLocL = 0;
    }
    else if(paddleLocL > 12288) {
      paddleLocL = 12288;
    }
  }

  if((GET_Y_AXIS(p2)) < 512 - DEAD_ZONE || 512 + DEAD_ZONE < (GET_Y_AXIS(p2))) {
    paddleLocR -= ((512 - (GET_Y_AXIS(p2))) * 3);
    if(paddleLocR < 0) {
      paddleLocR = 0;
    }
    else if(paddleLocR > 12288) {
      paddleLocR = 12288;
    }
  }
}
