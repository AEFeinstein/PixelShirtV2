/*
 * PlatformArduino.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: adam
 */

#ifdef ARDUINO

#include "Adafruit_NeoPixel.h"
#include "nRF24L01.h"
#include "nrf24.h"
#include "Arduino.h"
#include "ArduinoGame.h"
#include "PixelShirtV2.h"
#include "PlatformSpecific.h"
/* A pin to seed random with */
#define RANDOM_PIN   4  /* Analog, not connected */

/* Pixel strip pin connections */
#define NP_PIN_0            11
#define NP_PIN_1            12
#define NP_PIN_2            8
#define NP_PIN_3            9

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

/* The LED pin for the heart-beat blink */
#define HEARTBEAT_PIN 13

/* Addresses to talk to the wireless controllers. Hard-coded in the controller
 * firmware too
 */
uint8_t tx_address[5] = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7};
uint8_t rx_address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

/**
 * Sets a pixel in the display at the given position to the given color
 *
 * @param x   The X coordinate of the pixel to set
 * @param y   The Y coordinate of the pixel to set
 * @param rgb The color to set the pixel to
 */
void SetPixel(int8_t x, int8_t y, uint32_t rgb)
{
  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0: {
        pixels0.setPixelColor(index, rgb);
        break;
      }
    case 1: {
        pixels1.setPixelColor(index, rgb);
        break;
      }
    case 2: {
        pixels2.setPixelColor(index, rgb);
        break;
      }
    case 3: {
        pixels3.setPixelColor(index, rgb);
        break;
      }
  }
}

/**
 * Gets a pixel in the display at the given position and returns the color
 *
 * @param x   The X coordinate of the pixel to get
 * @param y   The Y coordinate of the pixel to get
 * @return    The color of the given pixel
 */
uint32_t GetPixel(int8_t x, int8_t y)
{
  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0: {
        return pixels0.getPixelColor(index);
      }
    case 1: {
        return pixels1.getPixelColor(index);
      }
    case 2: {
        return pixels2.getPixelColor(index);
      }
    case 3: {
        return pixels3.getPixelColor(index);
      }
    default: {
        return 0;
      }
  }
}

/**
 * Initializes the hardware including the heartbeat LED, display,
 * joystick RF, and random number generator
 */
void initializeHardware(void)
{
  /* Set up heart-beat */
  pinMode(HEARTBEAT_PIN, OUTPUT);

  /* Initialize the NeoPixel library. */
  pixels0.begin();
  pixels1.begin();
  pixels2.begin();
  pixels3.begin();

  /* Initialize hardware pins */
  nrf24_init();

  /* Channel #2 , payload length: 5 */
  nrf24_config(2, 4);

  /* Set the device addresses */
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);

  /* Seed the RNG */
  randomSeed(analogRead(RANDOM_PIN));
}

/**
 * Checks if there any incoming joystick data on the RF and
 * stores it locally for processing later
 *
 * Also handles the heartbeat LED, since this function is called often
 *
 * @param p1controller A pointer to the current controller data,
 *                     where new data should be stored
 * @param p2controller A pointer to the current controller data,
 *                     where new data should be stored
 */
uint8_t readJoystickData(uint32_t* p1controller, uint32_t* p2controller)
{
  uint32_t jsTmp;

  /* Heart-beat */
  if((millis() / 1000) % 2 == 0) {
    digitalWrite(HEARTBEAT_PIN, HIGH);
  }
  else {
    digitalWrite(HEARTBEAT_PIN, LOW);
  }

  /* If there is data from the controllers, get it and store it */
  if (nrf24_dataReady()) {
    nrf24_getData((uint8_t*)&jsTmp);
    switch(GET_PLAYER(jsTmp)) {
      case 0: {
          *p1controller = jsTmp;
          break;
        }
      case 1: {
          *p2controller = jsTmp;
          break;
        }
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * Both button presses and releases are reported, so do nothing.
 *
 * @param p1 unused
 * @param p2 unused
 */
void platformPostprocessInput(__attribute__((unused)) uint32_t* p1,
                              __attribute__((unused)) uint32_t* p2)
{
  ;
}

/**
 * Pushes all pixels out to the display
 */
void displayPixels(void)
{
  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
}

/**
 * Platform specific wrapper to get the time since boot in microseconds
 *
 * @return Number of microseconds since the program started (unsigned long)
 */
uint32_t getMicroseconds(void)
{
  return micros();
}

/**
 * Platform specific wrapper to get a random number
 *
 * @param max upper bound of the random value, exclusive
 * @return a random number between 0 and max-1 (long)
 */
uint32_t randomNumber(uint32_t max)
{
  return random(max);
}

#endif
