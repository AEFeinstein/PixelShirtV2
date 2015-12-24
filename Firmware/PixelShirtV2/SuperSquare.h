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

#ifndef _SUPER_SQUARE_H_
#define _SUPER_SQUARE_H_

#define NUM_LINES 6
#define LINE_SPEED_SCALER 128

typedef struct {
  int16_t position;
  int16_t velocity;
  uint8_t direction;
} Line;

#define INVALIDATE_LINE(x) ((x).position = -1)
#define IS_LINE_VALID(x) ((x).position >= 0)

class SuperSquare : public ArduinoGame
{
 public:
  SuperSquare(void);
  ~SuperSquare(void) {};
  void UpdatePhysics(void);
  void ResetGame(  uint8_t isInit,
                   uint8_t whoWon);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  void PlacePlayerPixel(uint8_t position[]);
  uint8_t DrawLine( Line line, uint32_t rgb);
  void AddLine(uint8_t direction, uint16_t velocity);

  int16_t playerPosition;
  int16_t currentVelocity;
  uint16_t score;
  uint8_t newLineTimer;
  uint8_t resetTimer;
  Line lines[NUM_LINES];
  uint8_t linesDrawn;
};

#endif
