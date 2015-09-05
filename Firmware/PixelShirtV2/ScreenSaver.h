#ifndef _SCREENSAVER_H_
#define _SCREENSAVER_H_

void DisplayScreensaver(void);
uint16_t GetScreensaverTimer(void);
void HandleScreensaverTimer(void);
void ExitScreensaver(ArduinoGame* currentGame);

#endif /* _SCREENSAVER_H_ */
