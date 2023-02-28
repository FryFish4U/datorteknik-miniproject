#include <stdio.h>
#include <stdint.h>
#include <pic32mx.h>
#include "projectlib.h"
extern int characterLane = 1;


int pressedBtns = -1; // int which getbtns should write to. Must be defaulted to -1 after use.

void laneRedirect(){

	pressedBtns = getbtns();

  	if(IFS(0) & 0x100){ // if int.ext.2 flag is 1 
		
		if(pressedBtns & 0b101) // if both move left and right are pressed: default pressedBtns
			pressedBtns = -1; // default pressedBtns
			
		if(pressedBtns & 0b001 && characterLane > 0){ // move up if btn 4 is pressed
			characterLane--; // move up
			showUfo();
			pressedBtns = -1; // default pressedBtns
		}

	    if(pressedBtns & 0b100 && characterLane < 2){ // move down if btn 2 is pressed
			characterLane++; // move down
			showUfo();
			pressedBtns = -1; // default pressedBtns
		}

		IFSCLR(0) = 0x100;
	}
} 
