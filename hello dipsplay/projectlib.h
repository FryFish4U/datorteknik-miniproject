/*

#include "projectlib.h"

*/

// declaration of sprites:

extern const uint8_t const ufo[19];
extern const uint8_t const spaceRock[10];


// uint8_t gameMap[512];     //! probobly not needed

int charactersLane; // the lane in which the character is in, starts at 128 which is middle, top is 0 and bottom is 256


// checkio.c functions:

int getbtns (void);
int getsw (void);


// display functions

void showUfo(void);


// init functions:

void time2init();
void time4init();
void buttonInit();
void debugInit();