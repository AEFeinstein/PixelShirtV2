#include "ArduinoGame.h"

#include "Pong.h"
#include "Lightcycle.h"
#include "Tetris.h"
#include "SuperSquare.h"
#include "Shooter.h"
#include "Adafruit_NeoPixel.h"
#include "ScreenSaver.h"
#include "nrf24.h"

/* Pixel strip pin connections */
#define NP_PIN_0            8
#define NP_PIN_1            10
#define NP_PIN_2            9
#define NP_PIN_3            11

/* Numbers of pixels per strip */
#define NUMPIXELS      64

/* Neopixel objects */
Adafruit_NeoPixel pixels0 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_0,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_1,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_2,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels3 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_3,
                            NEO_GRB + NEO_KHZ800);

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

#define HEARTBEAT_PIN 13

uint8_t tx_address[5] = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7};
uint8_t rx_address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
uint32_t p1controller;
uint32_t p2controller;

void doEverything();

void setup()
{
  /* Set up hearbeat */
  pinMode(HEARTBEAT_PIN, OUTPUT);

  /* Seed the RNG */
  randomSeed(analogRead(RANDOM_PIN));

  /* Initialize the NeoPixel library. */
  pixels0.begin();
  pixels1.begin();
  pixels2.begin();
  pixels3.begin();

  /* Clear Variables */
  uint8_t x, y;
  for(x = 0; x < BOARD_SIZE; x++) {
    for(y = 0; y < BOARD_SIZE; y++) {
      SetPixel(x, y, EMPTY_COLOR);
    }
  }
  downTimer = 0;

  /* Set the current game */
  gameMode = PONG;
  currentGame = &pong;
  currentGame->ResetGame(1,0);

  /* init hardware pins */
  nrf24_init();

  /* Channel #2 , payload length: 5 */
  nrf24_config(2, 4);

  /* Set the device addresses */
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);

  /* Init controller state */
  p1controller = 0;
  p2controller = 0;
  SET_X_AXIS(p1controller, 512);
  SET_Y_AXIS(p1controller, 512);
  SET_X_AXIS(p2controller, 512);
  SET_Y_AXIS(p2controller, 512);
}

void loop()
{
  uint32_t jsTmp;
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

  if (nrf24_dataReady()) {
    nrf24_getData((uint8_t*)&jsTmp);
    switch(GET_PLAYER(jsTmp)) {
      case 0: {
        p1controller = jsTmp;
        break;
      }
      case 1: {
        p2controller = jsTmp;
        break;
      }
    }
  }

  /* Heartbeat */
  if((millis() / 1000) % 2 == 0) {
    digitalWrite(HEARTBEAT_PIN, HIGH);
  }
  else {
    digitalWrite(HEARTBEAT_PIN, LOW);
  }
}

void doEverything()
{
  uint8_t p1buVal = GET_BUTTONS(p1controller) & UP;
  uint8_t p1bdVal = GET_BUTTONS(p1controller) & DOWN;
  uint8_t p1brVal = GET_BUTTONS(p1controller) & RIGHT;
  uint8_t p1blVal = GET_BUTTONS(p1controller) & LEFT;
  int16_t p1ax = GET_X_AXIS(p1controller);
  int16_t p1ay = GET_Y_AXIS(p1controller);

  uint8_t p2buVal = GET_BUTTONS(p2controller) & UP;
  uint8_t p2bdVal = GET_BUTTONS(p2controller) & DOWN;
  uint8_t p2brVal = GET_BUTTONS(p2controller) & RIGHT;
  uint8_t p2blVal = GET_BUTTONS(p2controller) & LEFT;
  int16_t p2ax = GET_X_AXIS(p2controller);
  int16_t p2ay = GET_Y_AXIS(p2controller);

  /* If both up buttons are held, maybe the game mode is being changed */
  if(p1buVal && p2buVal) {
    downTimer++;
  }
  else {
    downTimer = 0;
  }

  /* If there is any input, exit the screensaver */
  if(p1buVal || p1bdVal || p1blVal || p1brVal ||
      p2buVal || p2bdVal || p2blVal || p2brVal ||
      (p1ax < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p1ax) ||
      (p2ax < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p2ax) ||
      (p1ay < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p1ay) ||
      (p2ay < 512 - DEAD_ZONE || 512 + DEAD_ZONE < p2ay)) {
    ExitScreensaver(currentGame);
  }

  /* Decrement the screensaver timer */
  HandleScreensaverTimer();

  if(GetScreensaverTimer() == 0) {
    DisplayScreensaver();
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
      currentGame->ResetGame(1,0);
    }
    else {
      /* Process new controller input */
      currentGame->ProcessInput(p1ax, p1ay, p1blVal, p1brVal, p1buVal, p1bdVal,
                                p2ax, p2ay, p2blVal, p2brVal, p2buVal, p2bdVal);

      /* Update the game state */
      currentGame->UpdatePhysics();
    }
  }

  /* Display the frame */
  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
}

void DisplayScore(  uint16_t score, uint32_t rgb)
{
  uint8_t i, j;
  for(i = 0; i < BOARD_SIZE; i++) {
    for(j = 0; j < BOARD_SIZE; j++) {
      SetPixel(i, j, EMPTY_COLOR);
    }
  }

  // draw "score" on the field
  if(score > 999) {
    DrawNumber((score / 1000) % 10, 0 , 5, rgb);
  }
  if(score > 99) {
    DrawNumber((score / 100 ) % 10, 4 , 5, rgb);
  }
  if(score > 9) {
    DrawNumber((score / 10  ) % 10, 8 , 5, rgb);
  }
  DrawNumber((score / 1   ) % 10, 12, 5, rgb);
}

void DrawNumber( uint8_t number, uint8_t offsetX, uint8_t offsetY, uint32_t rgb)
{
  switch(number) {
    case 0: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 1: {
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 2: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 3: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 4: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 5: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 6: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 7: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 3, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        break;
      }
    case 8: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 0, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 0, offsetY + 4, rgb);
        SetPixel(offsetX + 1, offsetY + 4, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
    case 9: {
        SetPixel(offsetX + 0, offsetY + 0, rgb);
        SetPixel(offsetX + 1, offsetY + 0, rgb);
        SetPixel(offsetX + 2, offsetY + 0, rgb);
        SetPixel(offsetX + 0, offsetY + 1, rgb);
        SetPixel(offsetX + 2, offsetY + 1, rgb);
        SetPixel(offsetX + 0, offsetY + 2, rgb);
        SetPixel(offsetX + 1, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 2, rgb);
        SetPixel(offsetX + 2, offsetY + 3, rgb);
        SetPixel(offsetX + 2, offsetY + 4, rgb);
        break;
      }
  }
}

/* Sets a pixel in the matrix */
void SetPixel(int8_t x, int8_t y, uint32_t val)
{
  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0:
      pixels0.setPixelColor(index, val);
      break;
    case 1:
      pixels1.setPixelColor(index, val);
      break;
    case 2:
      pixels2.setPixelColor(index, val);
      break;
    case 3:
      pixels3.setPixelColor(index, val);
      break;
  }
}

uint32_t GetPixel(int8_t x, int8_t y)
{
  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0:
      return pixels0.getPixelColor(index);
      break;
    case 1:
      return pixels1.getPixelColor(index);
      break;
    case 2:
      return pixels2.getPixelColor(index);
      break;
    case 3:
      return pixels3.getPixelColor(index);
      break;
  }
  return 0;
}

