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
uint8_t StartSeq[] = {
  0xFF, 0xFF, 0xFF, 0xFF
};

#define HEARTBEAT_PIN 13

#define SCREENSAVER_TIMEOUT (IRQ_HZ * 30)
#define SQUARE_WAVE_SIZE  6
uint16_t screensaverTimer = 1;
uint8_t squareWavePoint = 0;
uint8_t colorStep = 0;
#define RED   0
#define GREEN 1
#define BLUE  2
uint8_t getIntensity(uint8_t step, uint8_t color);
uint8_t screensaverShiftTimer = IRQ_HZ / 4;

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
  uint8_t i, x, y;
  uint8_t* fieldPtr;

  uint8_t p1buVal = p1bu.read();
  uint8_t p1bdVal = p1bd.read();
  uint8_t p1brVal = p1br.read();
  uint8_t p1blVal = p1bl.read();
  int16_t p1ax = analogRead(P1_X);
  int16_t p1ay = analogRead(P1_Y);

  uint8_t p2buVal = p2bu.read();
  uint8_t p2bdVal = p2bd.read();
  uint8_t p2brVal = p2br.read();
  uint8_t p2blVal = p2bl.read();
  int16_t p2ax = analogRead(P2_X);
  int16_t p2ay = analogRead(P2_Y);

  /* If both up buttons are held, maybe the game mode is being changed */
  if(!p1buVal && !p2buVal) {
    downTimer++;
  }
  else {
    downTimer = 0;
  }

  /* If there is any input, exit the screensaver */
  if(p1buVal || p1bdVal || p1blVal || p1brVal ||
      p2buVal || p2bdVal || p2blVal || p2brVal ||
      (p1ax < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p1ax) ||
      (p2ax < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p2ax)) {
    screensaverTimer = SCREENSAVER_TIMEOUT;
  }

  /* Decrement the screensaver timer */
  if(screensaverTimer > 0) {
    screensaverTimer--;
    if(screensaverTimer == 0) {
      /* Clear the field */
      memset(field, 0, sizeof(field));
    }
  }

  if(screensaverTimer == 0) {
    /* display screensaver */

    if(screensaverShiftTimer > 0) {
      screensaverShiftTimer--;
    }
    if(screensaverShiftTimer == 0) {
      screensaverShiftTimer = IRQ_HZ / 4;

      /* Shift leftward */
      for(x = 0; x < BOARD_SIZE - 1; x++) {
        for(y = 0; y < BOARD_SIZE; y++) {
          field[x][y][0] = field[x + 1][y][0];
          field[x][y][1] = field[x + 1][y][1];
          field[x][y][2] = field[x + 1][y][2];
        }
      }

      /* Draw new rightmost column */
      if(squareWavePoint < SQUARE_WAVE_SIZE - 1) {
        /* top bar */
        SetPixel(BOARD_SIZE - 1, (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2,
                 getIntensity(colorStep, RED), getIntensity(colorStep, GREEN),
                 getIntensity(colorStep, BLUE));
      }
      else if(squareWavePoint == SQUARE_WAVE_SIZE - 1) {
        /* falling edge */
        for(y = (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2;
            y <= ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1; y++) {
          SetPixel(BOARD_SIZE - 1, y, getIntensity(colorStep, RED),
                   getIntensity(colorStep, GREEN), getIntensity(colorStep, BLUE));
        }
      }
      else if(squareWavePoint < (SQUARE_WAVE_SIZE * 2) - 1) {
        /* bottom bar */
        SetPixel(BOARD_SIZE - 1, ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1,
                 getIntensity(colorStep, RED), getIntensity(colorStep, GREEN),
                 getIntensity(colorStep, BLUE));
      }
      else if(squareWavePoint == (SQUARE_WAVE_SIZE * 2) - 1) {
        /* rising edge */
        for(y = (BOARD_SIZE - SQUARE_WAVE_SIZE) / 2;
            y <= ((BOARD_SIZE + SQUARE_WAVE_SIZE) / 2) - 1; y++) {
          SetPixel(BOARD_SIZE - 1, y, getIntensity(colorStep, RED),
                   getIntensity(colorStep, GREEN), getIntensity(colorStep, BLUE));
        }
      }
      squareWavePoint = (squareWavePoint + 1) % (SQUARE_WAVE_SIZE * 2);
      colorStep = (colorStep + 1) % 48;
    }
  }
  else {
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
      /* Process new controller input */
      currentGame->ProcessInput(field,
                                p1ax, p1ay, !p1blVal, !p1brVal, !p1buVal, !p1bdVal,
                                p2ax, p2ay, !p2blVal, !p2brVal, !p2buVal, !p2bdVal);

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
}

uint8_t getIntensity(uint8_t step, uint8_t color)
{
  switch(color) {
    case RED:
      if(step < 8) {
        /* High */
        return 0x80;
      }
      else if(step < 16) {
        /* fall */
        return (0x80 >> (step - 8));
      }
      else if(step >=40) {
        /* rise */
        return (0x01 << (step - 40));
      }
      return 0;
    case GREEN:
      if(step < 8) {
        return 0;
      }
      else if (step < 16) {
        /* rise */
        return (0x01 << (step - 8));
      }
      else if (step < 24) {
        /* High */
        return 0x80;
      }
      else if (step < 32) {
        /* fall */
        return (0x80 >> (step - 24));
      }
      return 0;
    case BLUE:
      if(step >= 40) {
        /* fall */
        return (0x80 >> (step - 40));
      }
      else if(step >= 32) {
        /* High */
        return 0x80;
      }
      else if(step >= 24) {
        /* rise */
        return (0x01 << (step - 24));
      }
      return 0;
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


