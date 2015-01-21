#include "Adafruit_NeoPixel.h"
#include <Wire.h>

/* Pixel strip pin connections */
#define NP_PIN_0            7
#define NP_PIN_1            5
#define NP_PIN_2            6
#define NP_PIN_3            4

/* Numbers of pixels per strip */
#define NUMPIXELS      64

/* Function prototype */
void setMatrixPixel(int8_t x, int8_t y, uint32_t val);

/* Neopixel objects */
Adafruit_NeoPixel pixels0 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_0,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_1,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_2,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels3 = Adafruit_NeoPixel(NUMPIXELS, NP_PIN_3,
                            NEO_GRB + NEO_KHZ800);

/* For a debug pattern */
//#define DEBUG_PATTERN
//#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
uint32_t tStart;
#endif

uint8_t cnt = 0;
uint32_t color;
uint8_t shades;
uint32_t lastFourBytes = 0;
uint8_t incomingByte;

uint8_t heartbeat;

/* function that executes whenever data is received from master */
/* this function is registered as an event, see setup() */
void receiveEvent(int howMany)
{
  #ifndef DEBUG_PATTERN

  #ifdef SERIAL_DEBUG
  Serial.print("Receive: ");
  Serial.print(howMany);
  Serial.print("\n");
  #endif

  while(Wire.available()) { /* loop through all but the last */
    /* read the incoming byte: */
    incomingByte = Wire.read();
    lastFourBytes = (lastFourBytes << 8) | incomingByte;

    /* Start of a new frame */
    if(lastFourBytes == 0xFFFFFFFF) {
      #ifdef SERIAL_DEBUG
      tStart = micros();
      #endif

      cnt = 0;
      color = 0;
      shades = 0;
    }

    /* If we receive a valid pixel value, set it */
    if(incomingByte != 0xFF) {
      color = (color << 8) | incomingByte;
      shades++;
      if(shades == 3) {
        setMatrixPixel(cnt%16, cnt/16, color);
        cnt = (cnt+1) % (256);
        shades = 0;
      }
      /* cnt wrapped around, time to draw a new frame */
      if(cnt == 0 && shades == 0) {
        #ifdef SERIAL_DEBUG
        Serial.print(micros() - tStart);
        Serial.print("\n");
        #endif
        pixels0.show();
        pixels1.show();
        pixels2.show();
        pixels3.show();
      }
    }
  }
  #endif
}

void setup()
{
  /* Initialize the NeoPixel library. */
  pixels0.begin();
  pixels1.begin();
  pixels2.begin();
  pixels3.begin();

  /* Initialize the serial library */
  #ifdef SERIAL_DEBUG
  Serial.begin(115200, SERIAL_8N1);
  #endif

  Wire.begin(4);                /* join i2c bus with address #4 */
  Wire.onReceive(receiveEvent); /* register event */

  pinMode(13, OUTPUT);
}

void loop()
{
  if((millis() / 500) % 2 == 0 && heartbeat == 0) {
    digitalWrite(13, HIGH);
    heartbeat = 1;
  }
  else if(heartbeat == 1) {
    digitalWrite(13, LOW);
    heartbeat = 0;
  }

  #ifdef DEBUG_PATTERN
  setMatrixPixel(15- cnt%16, 15- cnt/16, 0x000000);
  cnt = (cnt+1) % (256);
  setMatrixPixel(15- cnt%16, 15- cnt/16, 0x000040);

  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
  delay(50);
  #endif
}

/* Sets a pixel in the matrix */
void setMatrixPixel(int8_t y, int8_t x, uint32_t val)
{

  uint8_t index = (-16 * (x%4)) + 63 - y;

  switch(x / 4) {
    case 0:
      pixels0.setPixelColor(index , val);
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



