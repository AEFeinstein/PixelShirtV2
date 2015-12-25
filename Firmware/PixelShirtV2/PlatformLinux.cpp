/*
 * PlatformLinux.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: adam
 */

#ifndef ARDUINO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include "ArduinoGame.h"
#include "PixelShirtV2.h"

/* Prototypes */
int kbhit(void);
uint8_t nearestColor(uint32_t color);

/* Pixel data for Linux */
uint32_t field[BOARD_SIZE][BOARD_SIZE];

/* Stored analog values for the controllers.
 * On linux, these are modified with keyboard buttons
 */
int16_t p1Analog[2] = { 512, 512 };
int16_t p2Analog[2] = { 512, 512 };

/* Whether or not ncurses supports color */
uint8_t mHasColor = 0;

/**
 * Main function emulating how Arduino works
 *
 * @param argc The number of arguments, unused
 * @param argv An array of pointers, pointing to the string arguments
 * @return EXIT_SUCCESS
 */
int main(
  __attribute__((unused)) int argc,
  __attribute__((unused)) char* argv[])
{
  setup();
  while(1) {
    loop();
  }
  return EXIT_SUCCESS;
}

/**
 * Initializes the "hardware" on linux: ncurses and the
 * random number generator
 */
void initializeHardware(void)
{
  /* Initialize ncurses, with color if possible */
  initscr();
  if (has_colors()) {
    mHasColor = 1;
    start_color();
    init_color(COLOR_BLACK, 0, 0, 0);

    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);

    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);

    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
  }
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  /* Seed the RNG */
  srand(time(0));
}

/**
 * Detect keypresses
 *
 * @return 1 if a keypress was detected, 0 otherwise
 */
int kbhit(void)
{
  int ch = getch();

  if (ch != ERR) {
    ungetch(ch);
    return 1;
  }
  else {
    return 0;
  }
}

/**
 * Nonblocking function to read keypresses from the keyboard
 * This emulates joystick input
 *
 * Player 1:
 *   w    t
 *  a s  f h
 *   d    g   b
 *
 * Player 2:
 *   8    i
 *  4 6  j l
 *   5    k   ,
 *
 * z cycles the current game
 *
 * q quits the program
 */
void readJoystickData(void)
{

  /* Detect Keypresses, exit if necessary */
  if (kbhit()) {
    switch (getch()) {
      case 'w':
        p1Analog[Y] = CLAMP(p1Analog[Y] - 64, 1023);
        break;
      case 's':
        p1Analog[Y] = CLAMP(p1Analog[Y] + 64, 1023);
        break;
      case 'a':
        p1Analog[X] = CLAMP(p1Analog[X] - 64, 1023);
        break;
      case 'd':
        p1Analog[X] = CLAMP(p1Analog[X] + 64, 1023);
        break;

      case 't':
        SET_BUTTONS(p1controller, UP);
        break;
      case 'f':
        SET_BUTTONS(p1controller, LEFT);
        break;
      case 'g':
        SET_BUTTONS(p1controller, DOWN);
        break;
      case 'h':
        SET_BUTTONS(p1controller, RIGHT);
        break;
      case 'b':
        SET_BUTTONS(p1controller, STICK);
        break;

      case '8':
        p2Analog[Y] = CLAMP(p2Analog[Y] - 64, 1023);
        break;
      case '5':
        p2Analog[Y] = CLAMP(p2Analog[Y] + 64, 1023);
        break;
      case '4':
        p2Analog[X] = CLAMP(p2Analog[X] - 64, 1023);
        break;
      case '6':
        p2Analog[X] = CLAMP(p2Analog[X] + 64, 1023);
        break;

      case 'i':
        SET_BUTTONS(p2controller, UP);
        break;
      case 'j':
        SET_BUTTONS(p2controller, LEFT);
        break;
      case 'k':
        SET_BUTTONS(p2controller, DOWN);
        break;
      case 'l':
        SET_BUTTONS(p2controller, RIGHT);
        break;
      case ',':
        SET_BUTTONS(p2controller, STICK);
        break;

      case 'z':
        switchGame();
        break;
      case 'q':
        /* Quit the program */
        endwin();
        exit(0);
        return;
    }
  }
  SET_X_AXIS(p1controller, p1Analog[X]);
  SET_Y_AXIS(p1controller, p1Analog[Y]);
  SET_X_AXIS(p2controller, p2Analog[X]);
  SET_Y_AXIS(p2controller, p2Analog[Y]);
}

/**
 * Sets a pixel in the display at the given position to the given color
 *
 * @param x   The X coordinate of the pixel to set
 * @param y   The Y coordinate of the pixel to set
 * @param rgb The color to set the pixel to
 */
void SetPixel(int8_t x, int8_t y, uint32_t rgb)
{
  field[x][y] = rgb;
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
  return field[x][y];
}

/**
 * Maps an RGB color value to the nearest ncurses color
 *
 * @param color The input color to map, in 0xRRGGBB form
 * @return The closest ncurses color
 */
uint8_t nearestColor(uint32_t color)
{
  uint8_t r = (color & 0xFF0000) >> 16;
  uint8_t g = (color & 0x00FF00) >> 8;
  uint8_t b = (color & 0x0000FF) >> 0;

  if (r > g && r > b) {
    return COLOR_RED;
  }
  else if (g > r && g > b) {
    return COLOR_GREEN;
  }
  else if (b > r && b > g) {
    return COLOR_BLUE;
  }
  else if (r == g && g > b) {
    return COLOR_YELLOW;
  }
  else if (r == b && b > g) {
    return COLOR_MAGENTA;
  }
  else if (b == g && g > r) {
    return COLOR_CYAN;
  }
  return COLOR_WHITE;
}

/**
 * Draw the "LEDs" to the terminal
 * Also clear the controller input, since it was processed
 * for this frame by now
 */
void displayPixels(void)
{
  erase();

  int i, j;

  if (mHasColor) {
    attron(COLOR_PAIR(COLOR_WHITE));
  }
  printw("------------------\n");
  if (mHasColor) {
    attroff(COLOR_PAIR(COLOR_WHITE));
  }

  for (i = 0; i < BOARD_SIZE; i++) {

    if (mHasColor) {
      attron(COLOR_PAIR(COLOR_WHITE));
    }
    printw("|");
    if (mHasColor) {
      attroff(COLOR_PAIR(COLOR_WHITE));
    }

    for (j = 0; j < BOARD_SIZE; j++) {
      if (field[j][i]) {
        if (mHasColor) {
          attron(COLOR_PAIR(nearestColor(field[j][i])));
        }
        printw("%c", 178);
        if (mHasColor) {
          attroff(COLOR_PAIR(nearestColor(field[j][i])));
        }
      }
      else {
        printw(" ");
      }
    }

    if (mHasColor) {
      attron(COLOR_PAIR(COLOR_WHITE));
    }
    printw("|\n");
    if (mHasColor) {
      attroff(COLOR_PAIR(COLOR_WHITE));
    }
  }

  if (mHasColor) {
    attron(COLOR_PAIR(COLOR_WHITE));
  }
  printw("------------------\n");
  if (mHasColor) {
    attroff(COLOR_PAIR(COLOR_WHITE));
  }

  refresh();

  /* Clear the controller for next time around */
  p1controller = 0;
  p2controller = 0;
}

/**
 * Platform specific wrapper to get the current time in microseconds
 *
 * @return the current time in microseconds. Since this isn't since
 *         boot, it may wrap
 */
uint32_t getMicroseconds(void)
{
  struct timeval tv;
  gettimeofday(&tv,0);
  return tv.tv_usec + (tv.tv_sec * 1000000);
}

/**
 * Platform specific wrapper to get a random number
 *
 * @param max upper bound of the random value, exclusive
 * @return a random number between 0 and max-1 (long)
 */
uint32_t randomNumber(uint32_t max)
{
  return rand() % max;
}

#endif
