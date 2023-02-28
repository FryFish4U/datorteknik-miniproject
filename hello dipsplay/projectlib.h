/*

#include "projectlib.h"

*/

// declaration of sprites:
extern const uint8_t const ufo[19];
extern const uint8_t const spaceRock[10];
extern const uint8_t const exp1[10];
extern const uint8_t const exp2[10];
extern const uint8_t const exp3[10];
extern const uint8_t const exp4[10];

int characterLane; // the lane in which the character is in, starts at 128 which is middle, top is 0 and bottom is 256

// checkio.c functions:
int getbtns (void);
int getsw (void);

// display functions

void showUfo(void);
void move_ufo(int direction);
void setup_gameMap(void);
void spawn_obstacle(int lane);
uint8_t gameMap[512];		// array for the basic map

// init functions:
void timer2init();
void timer4init();
void buttonInit();
void debugInit();