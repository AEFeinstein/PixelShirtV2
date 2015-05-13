#ifndef _SCREENSAVER_H_
#define _SCREENSAVER_H_

void DisplayScreensaver();
uint16_t GetScreensaverTimer();
void HandleScreensaverTimer();
void ExitScreensaver(ArduinoGame* currentGame);

#endif /* _SCREENSAVER_H_ */
