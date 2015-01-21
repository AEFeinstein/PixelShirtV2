#include "ArduinoGame.h"

#include "Pong.h"
#include "Lightcycle.h"
#include "Tetris.h"
#include "SuperSquare.h"
#include "Shooter.h"

#include "I2C.h"
#include "DigitalPin.h"

/* Digital input buttons */
DigitalPin<P1_BL> p1bl;
DigitalPin<P1_BR> p1br;
DigitalPin<P1_BU> p1bu;
DigitalPin<P1_BD> p1bd;
DigitalPin<P2_BL> p2bl;
DigitalPin<P2_BR> p2br;
DigitalPin<P2_BU> p2bu;
DigitalPin<P2_BD> p2bd;

/* LED field, RGB */
uint8_t field[BOARD_SIZE][BOARD_SIZE][3];

/* Manage the Games */
#define NUM_GAMES 5

#define SUPER_SQUARE 0
#define PONG 1
#define LIGHTCYCLE 2
#define SHOOTER 3
#define TETRIS 4

uint8_t gameMode;
uint8_t downTimer;

ArduinoGame* currentGame;
SuperSquare superSquare;
Pong pong;
Lightcycle lightcycle;
Shooter shooter;
Tetris tetris;

/* To periodically call the main function in the loop */
uint32_t microsCnt = 0;
uint32_t microsLast = 0;

/* I2C Frame start sequence */
#define I2C_BUFFER_SIZE 128
uint8_t StartSeq[] = {0xFF, 0xFF, 0xFF, 0xFF};

#define HEARTBEAT_PIN 13

void setup()
{
  /* Set up hearbeat */
  pinMode(HEARTBEAT_PIN, OUTPUT);

  /* Set up i2c link to GPU */
  I2c.begin();
  I2c.setSpeed(1);
  I2c.pullup(1);

  /* Seed the RNG */
  randomSeed(analogRead(RANDOM_PIN));

  /* Clear Variables */
  memset((void*)field, 0, sizeof(field));
  downTimer = 0;

  /* Set the current game */
  gameMode = SUPER_SQUARE;
  currentGame = &superSquare;
  currentGame->ResetGame(field,1,0);

  /* Set internal pullups */
  p1bl.mode(INPUT);
  p1bl.high();
  p1br.mode(INPUT);
  p1br.high();
  p1bu.mode(INPUT);
  p1bu.high();
  p1bd.mode(INPUT);
  p1bd.high();
  p2bl.mode(INPUT);
  p2bl.high();
  p2br.mode(INPUT);
  p2br.high();
  p2bu.mode(INPUT);
  p2bu.high();
  p2bd.mode(INPUT);
  p2bd.high();
}

void loop()
{
  uint32_t currentTime = micros();

  /* Run the code at 32Hz (every 31250 microseconds) */
  /* Only add to the counter if it doesn't roll over */
  if(currentTime > microsLast) {
    microsCnt += (currentTime - microsLast);
  }
  if(microsCnt >= 31250) {
    microsCnt = 0;
    doEverything();
  }
  microsLast = currentTime;

  /* Heartbeat */
  if((millis() / 500) % 2 == 0) {
    digitalWrite(HEARTBEAT_PIN, HIGH);
  }
  else {
    digitalWrite(HEARTBEAT_PIN, LOW);
  }
}

void doEverything()
{
  uint8_t i;
  uint8_t* fieldPtr;

  /* If both up buttons are held, maybe the game mode is being changed */
  if(!p1bu.read() && !p2bu.read()) {
    downTimer++;
  }
  else {
    downTimer = 0;
  }

  /* Yep, the game mode is being changed */
  if(downTimer == IRQ_HZ * 2) {
    downTimer = 0;
    gameMode = (gameMode+1)%NUM_GAMES;
    switch(gameMode) {
      case PONG: {
          currentGame = &pong;
          break;
        }
      case LIGHTCYCLE: {
          currentGame = &lightcycle;
          break;
        }
      case TETRIS: {
          currentGame = &tetris;
          break;
        }
      case SUPER_SQUARE: {
          currentGame = &superSquare;
          break;
        }
      case SHOOTER: {
          currentGame = &shooter;
          break;
        }
    }
    currentGame->ResetGame(field,1,0);
  }
  else {
    /* Read new controller input */
    currentGame->ProcessInput(field,
                              analogRead(P1_X), analogRead(P1_Y),
                              !p1bl.read(), !p1br.read(), !p1bu.read(), !p1bd.read(),
                              analogRead(P2_X), analogRead(P2_Y),
                              !p2bl.read(), !p2br.read(), !p2bu.read(), !p2bd.read());

    /* Update the game state */
    currentGame->UpdatePhysics(field);

    /* Write the start sequence to send the frame to the GPU */
    I2c.write(4,0xFF, StartSeq, 3);

    /* Write each pixel, in buffered block */
    fieldPtr = &field[0][0][0];
    for(i=0; i < ((3*256)/I2C_BUFFER_SIZE); i++) {
      I2c.write(4, fieldPtr[i * I2C_BUFFER_SIZE], &fieldPtr[i * I2C_BUFFER_SIZE + 1],
                I2C_BUFFER_SIZE);
    }
  }
}

void DisplayScore( uint8_t field[BOARD_SIZE][BOARD_SIZE][3], uint16_t score)
{
  uint8_t i, j;
  for(i = 0; i < BOARD_SIZE; i++) {
    for(j = 0; j < BOARD_SIZE; j++) {
      SetPixel(i, j, 0,0,0);
    }
  }

  // draw "score" on the field
  if(score > 999) {
    DrawNumber(field, (score / 1000) % 10, 0 , 5, 0,0x40,0);
  }
  if(score > 99) {
    DrawNumber(field, (score / 100 ) % 10, 4 , 5, 0,0x40,0);
  }
  if(score > 9) {
    DrawNumber(field, (score / 10  ) % 10, 8 , 5, 0,0x40,0);
  }
  DrawNumber(field, (score / 1   ) % 10, 12, 5, 0,0x40,0);
}

void DrawNumber( uint8_t field[BOARD_SIZE][BOARD_SIZE][3], uint8_t number,
                 uint8_t offsetX, uint8_t offsetY, uint8_t r,
                 uint8_t g, uint8_t b)
{
  switch(number) {
    case 0: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 0, offsetY + 1, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 0, offsetY + 3, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 0, offsetY + 4, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 1: {
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 2: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 0, offsetY + 3, r, g, b);
        SetPixel(offsetX + 0, offsetY + 4, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 3: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 0, offsetY + 4, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 4: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 0, offsetY + 1, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 5: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 0, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 0, offsetY + 4, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 6: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 0, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 0, offsetY + 3, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 0, offsetY + 4, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 7: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 3, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        break;
      }
    case 8: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 0, offsetY + 1, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 0, offsetY + 3, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 0, offsetY + 4, r, g, b);
        SetPixel(offsetX + 1, offsetY + 4, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
    case 9: {
        SetPixel(offsetX + 0, offsetY + 0, r, g, b);
        SetPixel(offsetX + 1, offsetY + 0, r, g, b);
        SetPixel(offsetX + 2, offsetY + 0, r, g, b);
        SetPixel(offsetX + 0, offsetY + 1, r, g, b);
        SetPixel(offsetX + 2, offsetY + 1, r, g, b);
        SetPixel(offsetX + 0, offsetY + 2, r, g, b);
        SetPixel(offsetX + 1, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 2, r, g, b);
        SetPixel(offsetX + 2, offsetY + 3, r, g, b);
        SetPixel(offsetX + 2, offsetY + 4, r, g, b);
        break;
      }
  }
}
