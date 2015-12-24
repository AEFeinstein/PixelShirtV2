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

#include <stdint.h>
#include "ArduinoGame.h"
#include "PixelFighter.h"
#include "PlatformSpecific.h"

#define FACING_RIGHT 0
#define FACING_LEFT 1

PixelFighter fighter_one(FACING_RIGHT);
PixelFighter fighter_two(FACING_LEFT);

#define FIGHTER_WIDTH  4
#define FIGHTER_HEIGHT 9
#define STARTING_HP    5

#define JUMP_TIME   15 /* Should be a multiple of 5 */
#define MOVE_TIME   (IRQ_HZ/4)
#define ACTION_TIME (IRQ_HZ/4)
#define DANCE_TIME  (IRQ_HZ/2)

#define ATTACK_COLOR  0x202000
#define BLOCK_COLOR   0x002020
#define BODY_COLOR    0x151515

enum {
  COLOR_HEAD = 1, COLOR_BODY = 2, COLOR_ATTACK = 3, COLOR_BLOCK = 4
};

#define IDLE_2      0
#define ATTACK_HIGH 1
#define ATTACK_LOW  2
#define BLOCK_HIGH  3
#define BLOCK_LOW   4
#define IDLE_1      5

uint8_t sprites[6][19][2] = { { { 0x02, COLOR_HEAD }, { 0x03, COLOR_HEAD }, {
    0x12, COLOR_BODY }, { 0x13, COLOR_BODY }, { 0x22, COLOR_BODY }, { 0x31,
    COLOR_BODY }, { 0x32, COLOR_BODY }, { 0x33, COLOR_BODY },
    { 0x40, COLOR_BODY }, { 0x42, COLOR_BODY }, { 0x52, COLOR_BODY }, { 0x61,
        COLOR_BODY }, { 0x63, COLOR_BODY }, { 0x71, COLOR_BODY }, { 0x73,
        COLOR_BODY }, { 0x80, COLOR_BODY }, { 0x83, COLOR_BODY }, }, { { 0x02,
    COLOR_HEAD }, { 0x03, COLOR_HEAD }, { 0x12, COLOR_BODY },
    { 0x13, COLOR_BODY }, { 0x22, COLOR_BODY }, { 0x31, COLOR_BODY }, { 0x32,
        COLOR_BODY }, { 0x33, COLOR_BODY }, { 0x34, COLOR_ATTACK }, { 0x35,
        COLOR_ATTACK }, { 0x42, COLOR_BODY }, { 0x52, COLOR_BODY }, { 0x61,
        COLOR_BODY }, { 0x63, COLOR_BODY }, { 0x71, COLOR_BODY }, { 0x73,
        COLOR_BODY }, { 0x80, COLOR_BODY }, { 0x83, COLOR_BODY }, }, { { 0x02,
    COLOR_HEAD }, { 0x03, COLOR_HEAD }, { 0x12, COLOR_BODY },
    { 0x13, COLOR_BODY }, { 0x22, COLOR_BODY }, { 0x31, COLOR_BODY }, { 0x32,
        COLOR_BODY }, { 0x33, COLOR_BODY }, { 0x40, COLOR_BODY }, { 0x42,
        COLOR_BODY }, { 0x52, COLOR_BODY }, { 0x61, COLOR_BODY }, { 0x63,
        COLOR_BODY }, { 0x64, COLOR_ATTACK }, { 0x65, COLOR_ATTACK }, { 0x71,
        COLOR_BODY }, { 0x80, COLOR_BODY }, }, { { 0x01, COLOR_HEAD }, { 0x02,
    COLOR_HEAD }, { 0x11, COLOR_BODY }, { 0x12, COLOR_BODY },
    { 0x21, COLOR_BODY }, { 0x24, COLOR_BLOCK }, { 0x30, COLOR_BODY }, { 0x31,
        COLOR_BODY }, { 0x32, COLOR_BODY }, { 0x33, COLOR_BODY }, { 0x34,
        COLOR_BLOCK }, { 0x41, COLOR_BODY }, { 0x51, COLOR_BODY }, { 0x60,
        COLOR_BODY }, { 0x62, COLOR_BODY }, { 0x70, COLOR_BODY }, { 0x72,
        COLOR_BODY }, { 0x80, COLOR_BODY }, { 0x83, COLOR_BODY }, }, { { 0x01,
    COLOR_HEAD }, { 0x02, COLOR_HEAD }, { 0x11, COLOR_BODY },
    { 0x12, COLOR_BODY }, { 0x21, COLOR_BODY }, { 0x30, COLOR_BODY }, { 0x31,
        COLOR_BODY }, { 0x32, COLOR_BODY }, { 0x41, COLOR_BODY }, { 0x51,
        COLOR_BODY }, { 0x54, COLOR_BLOCK }, { 0x60, COLOR_BODY }, { 0x62,
        COLOR_BODY }, { 0x63, COLOR_BODY }, { 0x64, COLOR_BLOCK }, { 0x70,
        COLOR_BODY }, { 0x80, COLOR_BODY }, }, { { 0x01, COLOR_HEAD }, { 0x02,
    COLOR_HEAD }, { 0x11, COLOR_BODY }, { 0x12, COLOR_BODY },
    { 0x21, COLOR_BODY }, { 0x30, COLOR_BODY }, { 0x31, COLOR_BODY }, { 0x32,
        COLOR_BODY }, { 0x41, COLOR_BODY }, { 0x43, COLOR_BODY }, { 0x51,
        COLOR_BODY }, { 0x60, COLOR_BODY }, { 0x62, COLOR_BODY }, { 0x70,
        COLOR_BODY }, { 0x72, COLOR_BODY }, { 0x80, COLOR_BODY }, { 0x83,
        COLOR_BODY }, }, };

/***********************************
 *                                 *
 *         PixelFighterGame        *
 *                                 *
 ***********************************/

/**
 * TODO Document
 */
PixelFighterGame::PixelFighterGame(void) {
  ResetGame(1, 0);
}

/**
 * TODO Document
 */
void PixelFighterGame::UpdatePhysics(void) {
  fighter_one.ManageTimers(0, fighter_two.getXPos() - FIGHTER_WIDTH);
  fighter_two.ManageTimers(fighter_one.getXPos() + FIGHTER_WIDTH,
  BOARD_SIZE - FIGHTER_WIDTH);

  /* Clear the field */
  uint8_t x, y;
  for (x = 0; x < BOARD_SIZE; x++) {
    for (y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }
  fighter_one.DrawFighter();
  fighter_two.DrawFighter();
  /* TODO collisions & score */
}

/**
 * TODO Document
 *
 * @param isInit
 * @param losers
 */
void PixelFighterGame::ResetGame(__attribute__((unused))        uint8_t isInit,
    __attribute__((unused))        uint8_t losers) {
  /* TODO do something for the winner */
  fighter_one.InitFighter(FACING_RIGHT);
  fighter_two.InitFighter(FACING_LEFT);
}

/**
 * TODO Document
 *
 * @param p1
 * @param p2
 */
void PixelFighterGame::ProcessInput(int32_t p1, int32_t p2) {
  fighter_one.ProcessFighterInput(p1);
  fighter_two.ProcessFighterInput(p2);
}

/***********************************
 *                                 *
 *           PixelFighter          *
 *                                 *
 ***********************************/

/**
 * TODO Document
 *
 * @param facing
 */
void PixelFighter::InitFighter(uint8_t facing) {
  direction = facing;
  if (facing == FACING_LEFT) {
    fighter_two.xPos = BOARD_SIZE - FIGHTER_WIDTH - 1;
  }
  else if (facing == FACING_RIGHT) {
    xPos = 1;
  }
  yPos = BOARD_SIZE - FIGHTER_HEIGHT;
  hitPoints = STARTING_HP;
  sprite = IDLE_1;
  danceTimer = DANCE_TIME;
  jumpTimer = 0;
  actionTimer = 0;
  moveTimer = 0;
  velocity = 0;
}

/**
 * TODO Document
 *
 * @param input
 */
void PixelFighter::ProcessFighterInput(uint32_t input) {
  /* Only move if the fighter isn't jumping */
  if (jumpTimer == 0) {
    /* Start a jump */
    if ((GET_BUTTONS(input) & UP) || GET_Y_AXIS(input) < 256) {
      /* Set the jump timer */
      jumpTimer = JUMP_TIME;
    }

    /* Set the X axis velocity, only when on the ground */
    if (GET_X_AXIS(input) < 256) {
      velocity = -1;
    }
    else if (GET_X_AXIS(input) > 768) {
      velocity = 1;
    }
    else {
      velocity = 0;
    }
  }

  /* If the fighter can perform an action */
  if (actionTimer == 0) {
    /* This is an attack */
    if (GET_BUTTONS(input) & RIGHT) {
      /* Tilt the stick down for a low action */
      if (GET_Y_AXIS(input) > 768) {
        sprite = ATTACK_LOW;
      }
      /* Default is high */
      else {
        sprite = ATTACK_HIGH;
      }
      /* Set the action timer */
      actionTimer = ACTION_TIME;
    }
    /* This is a block */
    else if (GET_BUTTONS(input) & DOWN) {
      /* Tilt the stick down for a low action */
      if (GET_Y_AXIS(input) > 768) {
        sprite = BLOCK_LOW;
      }
      /* Default is high */
      else {
        sprite = BLOCK_HIGH;
      }
      /* Set the action timer */
      actionTimer = ACTION_TIME;
    }
  }
}

/**
 * TODO Document
 */
void PixelFighter::DrawFighter() {
  int8_t index;

  int8_t fighterOffset;
  int8_t posMultiplier;
  int8_t scoreOffset;
  uint32_t playerColor;
  uint32_t color;

  if (direction == FACING_LEFT) {
    posMultiplier = -1;
    fighterOffset = FIGHTER_WIDTH - 1;
    scoreOffset = BOARD_SIZE - 1;
    playerColor = P2_COLOR;
  }
  else if (direction == FACING_RIGHT) {
    posMultiplier = 1;
    fighterOffset = 0;
    scoreOffset = 0;
    playerColor = P1_COLOR;
  }

  for (index = 0; index < 19; index++) {
    if (sprites[sprite][index][1] != 0) {
      switch (sprites[sprite][index][1]) {
        case COLOR_HEAD: {
          color = playerColor;
          break;
        }
        case COLOR_BODY: {
          color = BODY_COLOR;
          break;
        }
        case COLOR_ATTACK: {
          color = ATTACK_COLOR;
          break;
        }
        case COLOR_BLOCK: {
          color = BLOCK_COLOR;
          break;
        }
      }
      SetPixel(
          (fighterOffset + posMultiplier * (sprites[sprite][index][0] & 0x0F))
              + xPos, ((sprites[sprite][index][0] & 0xF0) >> 4) + yPos, color);
    }
  }
  for (index = 0; index < hitPoints; index++) {
    SetPixel(scoreOffset + (posMultiplier * index), 0, SCORE_COLOR);
  }
}

/**
 * TODO Document
 *
 * @param lowerBound
 * @param upperBound
 */
void PixelFighter::ManageTimers(uint8_t lowerBound, uint8_t upperBound) {

  /* If the fighter is moving, wait for the move to finish */
  if (moveTimer > 0) {
    moveTimer--;
  }
  else {
    /* Is the fighter moving? */
    switch (velocity) {
      case 1: {
        if (xPos + 1 <= upperBound) {
          xPos++;
          moveTimer = MOVE_TIME;
        }
        break;
      }
      case -1: {
        if (xPos >= lowerBound + 1) {
          xPos--;
          moveTimer = MOVE_TIME;
        }
        break;
      }
    }
  }

  /* Is the fighter jumping? */
  if (jumpTimer > 0) {
    /* Adjust the height based on the time in the jump */
    if (jumpTimer > (JUMP_TIME / 5) * 4) {
      /* Rising */
      yPos = BOARD_SIZE - FIGHTER_HEIGHT - 1;
    }
    else if (jumpTimer > (JUMP_TIME / 5) * 3) {
      /* Rising */
      yPos = BOARD_SIZE - FIGHTER_HEIGHT - 2;
    }
    else if (jumpTimer > (JUMP_TIME / 5) * 2) {
      /* Apex */
      yPos = BOARD_SIZE - FIGHTER_HEIGHT - 3;
    }
    else if (jumpTimer > (JUMP_TIME / 5) * 1) {
      /* Falling */
      yPos = BOARD_SIZE - FIGHTER_HEIGHT - 2;
    }
    else if (jumpTimer > (JUMP_TIME / 5) * 0) {
      /* Falling */
      yPos = BOARD_SIZE - FIGHTER_HEIGHT - 1;
    }

    jumpTimer--;
  }
  else {
    /* On the ground */
    yPos = BOARD_SIZE - FIGHTER_HEIGHT;
  }

  /* Is an action being performed? */
  if (actionTimer > 0) {
    actionTimer--;
    /* If the action ended, return to idle animation */
    if (actionTimer == 0) {
      sprite = IDLE_1;
      danceTimer = DANCE_TIME;
    }
  }

  /* Is the fighter idling? */
  if (!isJumping() && (sprite == IDLE_1 || sprite == IDLE_2)) {
    /* Hold the animation for DANCE_TIME */
    if (danceTimer) {
      danceTimer--;
    }
    /* When the timer expires */
    else {
      /* Swap the sprite */
      if (sprite == IDLE_1) {
        sprite = IDLE_2;
      }
      else {
        sprite = IDLE_1;
      }
      /* Reset the timer */
      danceTimer = DANCE_TIME;
    }
  }
}

/**
 * TODO Document
 *
 * @return
 */
uint8_t PixelFighter::getXPos(void) {
  return xPos;
}

/**
 * TODO Document
 *
 * @return
 */
uint8_t PixelFighter::isJumping(void) {
  return jumpTimer > 0;
}

/**
 * TODO Document
 *
 * @param facing
 */
PixelFighter::PixelFighter(uint8_t facing) {
  InitFighter(facing);
}
