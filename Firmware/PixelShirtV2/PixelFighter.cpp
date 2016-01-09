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

/* TODO add time for the defender to block an attack after the "hit"
 * TODO stun after a parry
 */

/* How many hits does it take to lose?
 * No more than 8
 */
#define STARTING_HP    5

/* Timeouts to limit the speed of the game */
#define JUMP_TIME       (6 * 3) /* Should be a multiple of 6 */
#define MOVE_TIME       (IRQ_HZ/4)
#define ACTION_TIME     (IRQ_HZ/4) /* TODO split attack & block time */
#define DANCE_TIME      (IRQ_HZ/2)
#define INIT_PAUSE_TIME (IRQ_HZ * 4)
#define PAUSE_TIME      (IRQ_HZ * 2)

/* RGB color defines */
#define ATTACK_COLOR  0x202000
#define BLOCK_COLOR   0x002020
#define BODY_COLOR    0x151515

/* Indices for different colors.
 * It's easier to store an 8 bit index than a 32 bit color
 */
typedef enum {
  COLOR_IDX_NONE,  //!< COLOR_IDX_NONE
  COLOR_IDX_HEAD,  //!< COLOR_IDX_HEAD
  COLOR_IDX_BODY,  //!< COLOR_IDX_BODY
  COLOR_IDX_ATTACK,//!< COLOR_IDX_ATTACK
  COLOR_IDX_BLOCK  //!< COLOR_IDX_BLOCK
} fighterColor_t;

/* The size of a fighter */
#define FIGHTER_WIDTH  4
#define FIGHTER_HEIGHT 9

/* The different sprites a PixelFigher can show
 * This matches the order of the values in sprites[][][]
 */
typedef enum {
  IDLE_1,
  IDLE_2,
  ATTACK_HIGH,
  ATTACK_LOW,
  BLOCK_HIGH,
  BLOCK_LOW,
} sprite_t;

/* The maximum number of pixels in a sprite */
#define PIXELS_PER_SPRITE 19

/* The 4 MSB of the first byte is the X index
 * The 4 LSB of the first byte is the Y index
 * The second byte is the color index for that pixel
 */
const uint8_t sprites[6][PIXELS_PER_SPRITE][2]= {
  { {0x01,COLOR_IDX_HEAD},
    {0x02,COLOR_IDX_HEAD},
    {0x11,COLOR_IDX_BODY},
    {0x12,COLOR_IDX_BODY},
    {0x21,COLOR_IDX_BODY},
    {0x30,COLOR_IDX_BODY},
    {0x31,COLOR_IDX_BODY},
    {0x32,COLOR_IDX_BODY},
    {0x41,COLOR_IDX_BODY},
    {0x43,COLOR_IDX_BODY},
    {0x51,COLOR_IDX_BODY},
    {0x60,COLOR_IDX_BODY},
    {0x62,COLOR_IDX_BODY},
    {0x70,COLOR_IDX_BODY},
    {0x72,COLOR_IDX_BODY},
    {0x80,COLOR_IDX_BODY},
    {0x83,COLOR_IDX_BODY},
  },
  { {0x02,COLOR_IDX_HEAD},
    {0x03,COLOR_IDX_HEAD},
    {0x12,COLOR_IDX_BODY},
    {0x13,COLOR_IDX_BODY},
    {0x22,COLOR_IDX_BODY},
    {0x31,COLOR_IDX_BODY},
    {0x32,COLOR_IDX_BODY},
    {0x33,COLOR_IDX_BODY},
    {0x40,COLOR_IDX_BODY},
    {0x42,COLOR_IDX_BODY},
    {0x52,COLOR_IDX_BODY},
    {0x61,COLOR_IDX_BODY},
    {0x63,COLOR_IDX_BODY},
    {0x71,COLOR_IDX_BODY},
    {0x73,COLOR_IDX_BODY},
    {0x80,COLOR_IDX_BODY},
    {0x83,COLOR_IDX_BODY},
  },
  { {0x02,COLOR_IDX_HEAD},
    {0x03,COLOR_IDX_HEAD},
    {0x12,COLOR_IDX_BODY},
    {0x13,COLOR_IDX_BODY},
    {0x22,COLOR_IDX_BODY},
    {0x31,COLOR_IDX_BODY},
    {0x32,COLOR_IDX_BODY},
    {0x33,COLOR_IDX_BODY},
    {0x34,COLOR_IDX_ATTACK},
    {0x35,COLOR_IDX_ATTACK},
    {0x42,COLOR_IDX_BODY},
    {0x52,COLOR_IDX_BODY},
    {0x61,COLOR_IDX_BODY},
    {0x63,COLOR_IDX_BODY},
    {0x71,COLOR_IDX_BODY},
    {0x73,COLOR_IDX_BODY},
    {0x80,COLOR_IDX_BODY},
    {0x83,COLOR_IDX_BODY},
  },
  { {0x02,COLOR_IDX_HEAD},
    {0x03,COLOR_IDX_HEAD},
    {0x12,COLOR_IDX_BODY},
    {0x13,COLOR_IDX_BODY},
    {0x22,COLOR_IDX_BODY},
    {0x31,COLOR_IDX_BODY},
    {0x32,COLOR_IDX_BODY},
    {0x33,COLOR_IDX_BODY},
    {0x40,COLOR_IDX_BODY},
    {0x42,COLOR_IDX_BODY},
    {0x52,COLOR_IDX_BODY},
    {0x61,COLOR_IDX_BODY},
    {0x63,COLOR_IDX_BODY},
    {0x64,COLOR_IDX_ATTACK},
    {0x65,COLOR_IDX_ATTACK},
    {0x71,COLOR_IDX_BODY},
    {0x80,COLOR_IDX_BODY},
  },
  { {0x01,COLOR_IDX_HEAD},
    {0x02,COLOR_IDX_HEAD},
    {0x11,COLOR_IDX_BODY},
    {0x12,COLOR_IDX_BODY},
    {0x21,COLOR_IDX_BODY},
    {0x24,COLOR_IDX_BLOCK},
    {0x30,COLOR_IDX_BODY},
    {0x31,COLOR_IDX_BODY},
    {0x32,COLOR_IDX_BODY},
    {0x33,COLOR_IDX_BODY},
    {0x34,COLOR_IDX_BLOCK},
    {0x41,COLOR_IDX_BODY},
    {0x51,COLOR_IDX_BODY},
    {0x60,COLOR_IDX_BODY},
    {0x62,COLOR_IDX_BODY},
    {0x70,COLOR_IDX_BODY},
    {0x72,COLOR_IDX_BODY},
    {0x80,COLOR_IDX_BODY},
    {0x83,COLOR_IDX_BODY},
  },
  { {0x01,COLOR_IDX_HEAD},
    {0x02,COLOR_IDX_HEAD},
    {0x11,COLOR_IDX_BODY},
    {0x12,COLOR_IDX_BODY},
    {0x21,COLOR_IDX_BODY},
    {0x30,COLOR_IDX_BODY},
    {0x31,COLOR_IDX_BODY},
    {0x32,COLOR_IDX_BODY},
    {0x41,COLOR_IDX_BODY},
    {0x51,COLOR_IDX_BODY},
    {0x54,COLOR_IDX_BLOCK},
    {0x60,COLOR_IDX_BODY},
    {0x62,COLOR_IDX_BODY},
    {0x63,COLOR_IDX_BODY},
    {0x64,COLOR_IDX_BLOCK},
    {0x70,COLOR_IDX_BODY},
    {0x80,COLOR_IDX_BODY},
  },
};

/***********************************
 *                                 *
 *         PixelFighterGame        *
 *                                 *
 ***********************************/

/**
 * Constructor. This resets the game with no winner
 */
PixelFighterGame::PixelFighterGame(void)
{
  ResetGame(1, 0);
}

/**
 * Called IRQ_HZ times per second. This handles timers
 * for each fighter, manages game state, and redraws
 * the fighters on the display
 */
void PixelFighterGame::UpdatePhysics(void)
{
  /* Handle timers for each fighter */
  fighterOne.ManageTimers(0, fighterTwo.getXPos() - FIGHTER_WIDTH);
  fighterTwo.ManageTimers(fighterOne.getXPos() + FIGHTER_WIDTH,
                          BOARD_SIZE - FIGHTER_WIDTH);

  /* Clear the field */
  uint8_t x, y;
  for (x = 0; x < BOARD_SIZE; x++) {
    for (y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }

  /* Check for fighter interaction */
  if(fighterTwo.getXPos() - fighterOne.getXPos() < FIGHTER_WIDTH + 3) {
    /* fighters are within striking range */
    CheckForHitAndBlock(&fighterOne, &fighterTwo);
    CheckForHitAndBlock(&fighterTwo, &fighterOne);
  }

  /* Draw the fighters */
  fighterOne.DrawFighter();
  fighterTwo.DrawFighter();
}

/**
 * TODO
 * @param attackingFighter
 * @param defendingFighter
 */
void PixelFighterGame::CheckForHitAndBlock(
    PixelFighter * attackingFighter,
    PixelFighter * defendingFighter)
{
  /* Check to see if the fighter is starting an attack */
  if(attackingFighter->getSuccessfulAttackTimer() == 0 &&
      attackingFighter->isAttacking()) {
    /* Start the successful attack timer. */
    attackingFighter->StartSuccessfulAttackTimer();
  }

  /* Check to see if the attack was blocked */
  if(attackingFighter->getSuccessfulAttackTimer() > 0 &&
      (defendingFighter->isBlocking() &&
          (defendingFighter->getActionHeight() == attackingFighter->getActionHeight()))) {
    attackingFighter->attackIsBlocked();
  }
}

/**
 * Reset the game by resetting the individual fighters. Pause them as
 * appropriate. Hide the loser if there is a loser
 *
 * @param isInit Whether or not this is the initial reset
 * @param losers Which fighter lost the match
 */
void PixelFighterGame::ResetGame(uint8_t isInit, uint8_t losers)
{
  /* Initialize the fighters */
  fighterOne.InitFighter(FACING_RIGHT, 1);
  fighterTwo.InitFighter(FACING_LEFT, 1);

  /* If this is the initial reset, pause longer */
  if(isInit) {
    fighterOne.SetPause(INIT_PAUSE_TIME);
    fighterTwo.SetPause(INIT_PAUSE_TIME);
  }
  /* Otherwise pause a little */
  else {
    fighterOne.SetPause(PAUSE_TIME);
    fighterTwo.SetPause(PAUSE_TIME);
  }

  /* If loser is 1 or 2, hide that player for the pause duration */
  switch(losers) {
    case 1: {
      fighterOne.Lost();
      break;
    }
    case 2: {
      fighterTwo.Lost();
      break;
    }
  }
}

/**
 * Processes input for each fighter
 *
 * @param p1 The joystick input for player 1
 * @param p2 The joystick input for player 2
 */
void PixelFighterGame::ProcessInput(int32_t p1, int32_t p2)
{
  fighterOne.ProcessFighterInput(p1);
  fighterTwo.ProcessFighterInput(p2);
}

/***********************************
 *                                 *
 *           PixelFighter          *
 *                                 *
 ***********************************/

/**
 * Default constructor
 */
PixelFighter::PixelFighter()
{
  /* Set all variables, FACING_LEFT by default */
  InitFighter(FACING_LEFT, 1);
}

/**
 * Initializer for a PixelFighter. Resets position, timers,
 * and hitpoints
 *
 * @param facing  The direction this fighter is facing,
 *                either FACING_RIGHT or FACING_LEFT
 * @param resetHP TRUE if HP should be reset, FALSE otherwise
 */
void PixelFighter::InitFighter(direction_t facing, uint8_t resetHP)
{
  direction = facing;
  if (facing == FACING_LEFT) {
    xPos = BOARD_SIZE - FIGHTER_WIDTH - 1;
  }
  else if (facing == FACING_RIGHT) {
    xPos = 1;
  }
  if(resetHP) {
    hitPoints = STARTING_HP;
  }
  sprite = IDLE_1;
  danceTimer = DANCE_TIME;
  attackTimer = 0;
  blockTimer = 0;
  moveTimer = 0;
  velocity = 0;
  successfulAttackTimer = 0;
}

/**
 * Process joystick input for a figher. Button actions
 * may only be performed if their respective timers have
 * timed out. The joystick doesn't actually move the fighter,
 * but rather sets a velocity for it. The movement happens in
 * PixelFighter::ManageTimers()
 *
 * @param input The controller input to process
 */
void PixelFighter::ProcessFighterInput(uint32_t input)
{
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

  /* If the fighter can perform an action */
  if (!isAttacking() && !isBlocking()) {
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
      attackTimer = ACTION_TIME;
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
      blockTimer = ACTION_TIME;
    }
  }
}

/**
 * Draws both the PixelFigher and it's HP bar
 * on the display
 */
void PixelFighter::DrawFighter(void)
{
  int8_t index;

  int8_t fighterOffset;
  int8_t posMultiplier;
  int8_t scoreOffset;
  uint32_t playerColor;
  uint32_t color;

  /* If we shouldn't draw, return */
  if(dontDrawTimer) {
    return;
  }

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

  for (index = 0; index < PIXELS_PER_SPRITE; index++) {
    if (sprites[sprite][index][1] != COLOR_IDX_NONE) {
      switch (sprites[sprite][index][1]) {
        case COLOR_IDX_HEAD: {
            color = playerColor;
            break;
          }
        case COLOR_IDX_BODY: {
            color = BODY_COLOR;
            break;
          }
        case COLOR_IDX_ATTACK: {
            color = ATTACK_COLOR;
            break;
          }
        case COLOR_IDX_BLOCK: {
            color = BLOCK_COLOR;
            break;
          }
      }

      /* Find the old color index, to check for overdraw */
      uint32_t oldPixel = GetPixel(
          (fighterOffset + posMultiplier * (sprites[sprite][index][0] & 0x0F)) + xPos,
          ((sprites[sprite][index][0] & 0xF0) >> 4) + BOARD_SIZE - FIGHTER_HEIGHT);
      uint8_t oldIdx = 0;
      switch(oldPixel) {
        case BODY_COLOR: {
          oldIdx = COLOR_IDX_BODY;
          break;
        }
        case P1_COLOR: {
          oldIdx = COLOR_IDX_HEAD;
          break;
        }
        case P2_COLOR: {
          oldIdx = COLOR_IDX_HEAD;
          break;
        }
        case ATTACK_COLOR: {
          oldIdx = COLOR_IDX_ATTACK;
          break;
        }
        case BLOCK_COLOR: {
          oldIdx = COLOR_IDX_BLOCK;
          break;
        }
        default: {
          oldIdx = COLOR_IDX_NONE;
        }
      }

      /* If the new pixel should be drawn over the old one */
      if(sprites[sprite][index][1] > oldIdx) {
        SetPixel(
          (fighterOffset + posMultiplier * (sprites[sprite][index][0] & 0x0F)) + xPos,
          ((sprites[sprite][index][0] & 0xF0) >> 4) + BOARD_SIZE - FIGHTER_HEIGHT,
          color);
      }
    }
  }
  for (index = 0; index < hitPoints; index++) {
    SetPixel(scoreOffset + (posMultiplier * index), 0, SCORE_COLOR);
  }
}

/**
 * Manage timers for lateral movement, jumping, and actions
 * Move the fighter if necessary
 *
 * @param lowerBound As far left as the fighter can move
 * @param upperBound As far right as the fighter can move
 */
void PixelFighter::ManageTimers(uint8_t lowerBound, uint8_t upperBound)
{
  /* If the fighter is paused, don't move, perform actions, or idle
   * Jumping is OK
   */
  if (pauseTimer > 0) {
    pauseTimer--;
  }
  else {
    /* If the fighter is moving, wait for the move to finish */
    if (moveTimer > 0) {
      moveTimer--;
    }
    else {
      /* Is the fighter moving? */
      switch (velocity) {
        case 1: {
            /* Moving rightward */
            if (xPos + 1 <= upperBound) {
              xPos++;
              moveTimer = MOVE_TIME;
            }
            break;
          }
        case -1: {
            /* Moving leftward */
            if (xPos >= lowerBound + 1) {
              xPos--;
              moveTimer = MOVE_TIME;
            }
            break;
          }
      }
    }

    /* Is the fighter attacking? */
    if (attackTimer > 0) {
      attackTimer--;
      /* If the action ended, return to idle animation */
      if (attackTimer == 0) {
        sprite = IDLE_1;
        danceTimer = DANCE_TIME;
      }
    }

    /* Is the fighter attacking? */
    if (blockTimer > 0) {
      blockTimer--;
      /* If the action ended, return to idle animation */
      if (blockTimer == 0) {
        sprite = IDLE_1;
        danceTimer = DANCE_TIME;
      }
    }

    /* Is the fighter idling? */
    if (sprite == IDLE_1 || sprite == IDLE_2) {
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

    /* Make sure the attack isn't parried */
    if(successfulAttackTimer > 0) {
      successfulAttackTimer--;
      if (successfulAttackTimer == 0) {
        // TODO attack was successful, decrement HP
        decrementHP();
      }
    }
  }

  /* Decrement the don't draw timer if necessary */
  if(dontDrawTimer > 0) {
    dontDrawTimer--;
  }

}

/**
 * @return The current X coordinate of this fighter
 */
uint8_t PixelFighter::getXPos(void)
{
  return xPos;
}

/**
 * TODO document
 */
uint8_t PixelFighter::getActionHeight(void)
{
  if(sprite == BLOCK_HIGH || sprite == ATTACK_HIGH) {
    return 1;
  }
  else {
    return 0;
  }
}

/**
 * TODO document
 * @return
 */
uint8_t PixelFighter::isAttacking(void)
{
  return attackTimer > 0;
}

/**
 * TODO document
 * @return
 */
uint8_t PixelFighter::isBlocking(void)
{
  return blockTimer > 0;
}

/**
 * TODO document
 * @return
 */
uint8_t PixelFighter::decrementHP(void)
{
  if(hitPoints > 0) {
    hitPoints--;
  }
  return hitPoints;
}

/**
 * TODO document
 * @param time
 */
void PixelFighter::SetPause(uint8_t time)
{
  pauseTimer = time;
}

/**
 * TODO document
 */
void PixelFighter::Lost(void)
{
  dontDrawTimer = INIT_PAUSE_TIME - PAUSE_TIME;
}

/**
 * TODO
 */
void PixelFighter::StartSuccessfulAttackTimer(void) {
  successfulAttackTimer = IRQ_HZ / 4;
}

/**
 * TODO
 * @return
 */
uint8_t PixelFighter::getSuccessfulAttackTimer(void) {
  return successfulAttackTimer;
}

/**
 * TODO
 */
void PixelFighter::attackIsBlocked(void) {
  successfulAttackTimer = 0;
  attackTimer = 0;
  sprite = IDLE_1;
  SetPause(IRQ_HZ); // TODO find a proper value
  SetPixel(1,1,P1_COLOR);
}
