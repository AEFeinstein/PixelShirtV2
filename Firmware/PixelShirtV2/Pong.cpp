#include "Pong.h"

/**************************
 *                         *
 *         Pong!!          *
 *                         *
 ***************************/

#define BALL_COLOR 0x004000

void Pong::UpdatePhysics( )
{
  int16_t diff, rotation;
  if(restartTimer > 0) {
    restartTimer--;
  }
  else {
    // Physics!
    // Update the ball's position
    ballLoc[X] += (ballVel[X] * TIME_STEP);
    ballLoc[Y] += (ballVel[Y] * TIME_STEP);

    // Check for collisions, win conditions. Nudge the ball if necessary
    // Top wall collision
    if (ballLoc[Y] < 0 && ballVel[Y] < 0) {
      ballVel[Y] = -ballVel[Y];
      ballLoc[Y] = 0;
    }
    // Bottom wall collision
    else if (ballLoc[Y] >= BOARD_SIZE * S_M * V_M && ballVel[Y] > 0) {
      ballVel[Y] = -ballVel[Y];
      ballLoc[Y] = (BOARD_SIZE) * S_M * V_M - 1;
    }
    // left paddle collision
    else if (ballLoc[X] < 1 * S_M * V_M && ballVel[X] < 0
             && (paddleLocL <= ballLoc[Y]
                 && ballLoc[Y] < paddleLocL + PADDLE_SIZE)) {
      ballVel[X] = -ballVel[X];
      ballLoc[X] = 1 * S_M * V_M; // right on the edge of the paddle

      //Increase speed on collision
      IncreaseSpeed(37); // remember, divided by V_M

      // Apply extra rotation depending on part of the paddle hit
      // range of diff is -PADDLE_SIZE/2 to PADDLE_SIZE/2 (+/- 8192)
      diff = ballLoc[Y] - (paddleLocL + PADDLE_SIZE / 2);
      rotation = (EXTRA_ROTATION_ON_EDGE * diff) / (PADDLE_SIZE /
                 2); // rotate 45deg at edge of paddle, 0deg in middle, linear in between
      RotateBall(rotation);
    }
    // right paddle collision
    else if (ballLoc[X] >= (BOARD_SIZE - 1) * S_M * V_M && ballVel[X] > 0
             && (paddleLocR <= ballLoc[Y]
                 && ballLoc[Y] < paddleLocR + PADDLE_SIZE)) {
      ballVel[X] = -ballVel[X];
      ballLoc[X] = (BOARD_SIZE - 1) * S_M * V_M -
                   1; // right on the edge of the paddle

      //Increase speed on collision
      IncreaseSpeed(37); // remember, divided by V_M

      // Apply extra rotation depending on part of the paddle hit
      // range of diff is -PADDLE_SIZE/2 to PADDLE_SIZE/2 (+/- 8192)
      diff = ballLoc[Y] - (paddleLocR + PADDLE_SIZE / 2);
      rotation = (EXTRA_ROTATION_ON_EDGE * diff) / (PADDLE_SIZE /
                 2); // rotate 45deg at edge of paddle, 0deg in middle, linear in between
      RotateBall(-rotation);
    }
    // left wall win
    else if (ballLoc[X] < 0 && ballVel[X] < 0) {
      ResetGame(0, 1);
    }
    // right wall win
    else if (ballLoc[X] >= BOARD_SIZE * S_M * V_M && ballVel[X] > 0) {
      ResetGame(0, 0);
    }
  }
  DrawField();
}

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

// Rotate the ball in degrees (-360 -> 359)
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

// Apply a multiplier to the velocity. make this additive instead?
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

void Pong::DrawField( )
{
  int16_t i, j;
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      if ((j == 0) && ((paddleLocL / (S_M * V_M)) <= i
                       && i < (paddleLocL / (S_M * V_M)) + PADDLE_SIZE / (V_M * S_M))) {
        SetPixel(j, i, P1_COLOR);
      }
      else if ((j == (BOARD_SIZE - 1)) && ((paddleLocR / (S_M * V_M)) <= i
                                           && i < (paddleLocR / (S_M * V_M)) + PADDLE_SIZE / (V_M * S_M))) {
        SetPixel(j, i, P2_COLOR);
      }
      else if (j == (ballLoc[X] / (S_M * V_M)) && i == (ballLoc[Y] / (S_M * V_M))) {
        SetPixel(j, i, BALL_COLOR);
      }
      else {
        SetPixel(j, i, EMPTY_COLOR);
      }
    }
  }
}

void Pong::ProcessInput(
  __attribute__((unused))  
  __attribute__((unused)) int32_t p1ax,
  int32_t p1ay,
  __attribute__((unused)) int8_t p1bl,
  __attribute__((unused)) int8_t p1br,
  __attribute__((unused)) int8_t p1bu,
  __attribute__((unused)) int8_t p1bd,
  __attribute__((unused)) int32_t p2ax,
  int32_t p2ay,
  __attribute__((unused)) int8_t p2bl,
  __attribute__((unused)) int8_t p2br,
  __attribute__((unused)) int8_t p2bu,
  __attribute__((unused)) int8_t p2bd)
{
  if(p1ay < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p1ay) {
    paddleLocL += ((512 - p1ay) * 3);
    if(paddleLocL < 0) {
      paddleLocL = 0;
    }
    else if(paddleLocL > 12288) {
      paddleLocL = 12288;
    }
  }

  if(p2ay < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p2ay) {
    paddleLocR += ((512 - p2ay) * 3);
    if(paddleLocR < 0) {
      paddleLocR = 0;
    }
    else if(paddleLocR > 12288) {
      paddleLocR = 12288;
    }
  }
}




