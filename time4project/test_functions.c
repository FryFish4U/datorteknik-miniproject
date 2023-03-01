#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

//Overhead function

void setup_ufo_area(void) {
    // setting up the area first time. called once in init before setting up the ufo on the map (setup_ufo();)
    int i = 0;
    int j = 0;

    for (i = 0; i < 3; i++){
        for (j = 0; j < 19; j++){
            if (i == 1){
                ufo_area[(i*19) + j] = (255 & ufo[j]); //since a 1 on the scrren means black. the 0s in ufo will be visible by bitwise AND
            }
            else{
                ufo_area[(i*19) + j] = 255; // rest of the area is black
            }
        }
    }
}

void setup_ufo(void){
    // setting up the ufo on the map. should only be called once. goes in labinit
    int i = 0;
    int j = 0;

    for (i = 0; i < 3; i++){
        for (j = 0; j < 128; j++){
            if (9 < j < 24){
                map[(i*128) + j] = (255 & ufo_area[(i*128) + j]);
            }
            else{
                 map[(i*128) + j] = 255;
            }
        }
    }
    display_image(0,map);
}
void map_update(void){
//what is meant to be done here is to combine all the new information on the top three pages and display it.
//this is done once with every flag event from timer 2
    int i = 0;
    int j = 0;

    for (i = 0; i < 3; i++){
        for (j = 0; j < 128; j++){
            if (9 < j < 24){ //if j is in this intervall we are in the area of the ufo and does an bitwise AND between ufo and obs.
                map[(i*128) + j] = (255 & (obs_area[(i*148) + (j+10)] & ufo_area[(i*128) + j]));
            }
            else{ // here we are outside od the ufo range and as such do not need the ufo_area
                map[(i*128) + j] = (255 & (obs_area[(i*148) + (j+10)]));
            }
        }
    }
    display_image(0,map);
}
//areas
uint8_t map[384];
uint8_t obs_area[444]; //it is biggger than map. goes from ((0 to 2) *148). spot (((0 to 2) *148) + 10) until(( (0 to 2) *148) + 137) overlaps with map
uint8_t ufo_area[57]; // goes from (0 to 2) *19) area is placed in (((0 to 2) *128) + 10)

//movement
void move_ufo (int direction){
// this function will move the ufo in the direction that is indicated by the argument by one pixel at the time for as long as the buttun is held.
//this will be done by udating a set area of pixels in page 0-2 row 9 to 28'
    uint32_t temp[19];
    int i = 0;
    int j = 0;

    //first we copy the whole row of all three pages into an int array
    // first the first page the is copied and shifted to the left to make room for the next page.
    for (i = 0; i < 3; i++){
        for (j = 0; j < 19; j++){
            temp[j] = (temp[j] & ufo_area [(i*19) + j]);
            temp[j] = (temp[j] << 8);
        }
    }
    if(/*syntax for button for moving up is pressed*/){
        temp[j] = ((temp[j] >> 1) | 0x800); // for moving up, we switch everything to the right and then add at blank space as the "most sigificant bit"
        for (i = 2; i >= 0; i--){
            for (j = 18; j <= 0; j--){ // then we copy it back into the ufo_area but backwards since the least significant bits represent the lowest lane.
                ufo_area[(i*19) + j] = (temp[j] & ufo_area [(i*19) + j]);
                temp[j] = (temp[j] >> 8);
            }
        }
    }
    if(/*syntax for button for moving down is pressed*/){
        temp[j] = ((temp[j] << 1) | 1); // for movin down we shift to the left and add an empty space
        for (i = 2; i >= 0; i--){
            for (j = 18; j <= 0; j--){ // same type of write back int ufo_area as above
                ufo_area[(i*19) + j] = (temp[j] & ufo_area [(i*19) + j]);
                temp[j] = (temp[j] >> 8);
            }
        }
    }
}

void move_obs(void){
//This function will upgrade the obs area from row to move all current obs one row to the left (128 rows)


}
void spawn_obs(void){
    // at every 24th flag of timer 4 the obs are that is furthest to the right 
    // Will be updated with obstacles
    // there can be at most 2 obstacles in one row.
}