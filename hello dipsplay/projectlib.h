/*

#include "projectlib.h"

*/

// declaration of sprites:
extern const uint8_t const ufo[19];
extern const uint8_t const spaceRock[10];
static const uint8_t const font[];

// uint8_t gameMap[512];     //! probobly not needed
int charactersLane; // the lane in which the character is in, starts at 128 which is middle, top is 0 and bottom is 256

// checkio.c functions:
int getbtns (void);
int getsw (void);

// display functions

void showUfo(void);
void move_ufo(int direction)
void setup_gameMap(void)

// init functions:
void timer2init();
void timer4init();
void buttonInit();
void debugInit();