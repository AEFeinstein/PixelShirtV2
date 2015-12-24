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

#ifndef _LIGHTCYCLE_H_
#define _LIGHTCYCLE_H_

class Lightcycle : public ArduinoGame
{
 public:
  Lightcycle(void);
  ~Lightcycle(void) {};
  void UpdatePhysics(void);
  void ResetGame( uint8_t isInit,
                  uint8_t losers);
  void ProcessInput( int32_t p1, int32_t p2);
 private:
  int8_t p1pos[2], p2pos[2];
  uint8_t p1dir, p2dir;
  uint8_t movementTimer;
  uint8_t showingWinningLEDs;
  uint8_t started;
};

#endif
