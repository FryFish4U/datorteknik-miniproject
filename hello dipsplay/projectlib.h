/*

#include "projectlib.h"

*/

// declaration of sprites:
//* uint8_t stands for unsigned integer with 8 bits in length

extern const uint8_t const ufo[19]; // the player character sprite
extern const uint8_t const spaceRock[10]; // sprite of debris that will approach the player character


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