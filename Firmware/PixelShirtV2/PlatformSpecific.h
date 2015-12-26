/*
 * PlatformSpecific.h
 *
 *  Created on: Dec 24, 2015
 *      Author: adam
 */

#ifndef PLATFORMSPECIFIC_H_
#define PLATFORMSPECIFIC_H_

void initializeHardware(void);
uint8_t readJoystickData(uint32_t*, uint32_t*);

void SetPixel(int8_t y, int8_t x, uint32_t val);
uint32_t GetPixel(int8_t x, int8_t y);
void displayPixels(void);

uint32_t getMicroseconds(void);
uint32_t randomNumber(uint32_t max);

#define IsPixelLit(x,y) GetPixel(x, y)

#endif /* PLATFORMSPECIFIC_H_ */
