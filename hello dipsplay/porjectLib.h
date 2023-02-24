#include <pic32mx.h>
#include <stdio.h>
#include <stdint.h>

//declaration of sprites:
extern const uint8_t ufo[19];
extern const uint8_t spaceRock[20];



uint8_t gameMap[512];
volatile int charactersLane = 128; // the lane in which the character is in, starts at 128 which is middle, top is 0 and bottom is 256
