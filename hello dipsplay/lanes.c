#include <stdio.h>
#include <stdint.h>
#include <pic32mx.h>
#include <porjectLib.h>
#include <checkio.c>

int pressedBtns = -1; // int which getbtns should write to. Must be defaulted to -1 after use.

void laneRedirect(){

	pressedBtns = getbtns();

  	if(IFS(0) & 0x100){ // if int.ext.2 flag is 1 
		
		if(pressedBtns == 0b101) // if both move left and right are pressed: default pressedBtns
			pressedBtns = -1; // default pressedBtns
			
		if(pressedBtns == 0b001 && charactersLane > 0){ // move up if btn 4 is pressed
			charactersLane--; // move up
			pressedBtns = -1; // default pressedBtns
		}

	    if(pressedBtns == 0b100 && charactersLane < 2){ // move down if btn 2 is pressed
			charactersLane++; // move down
			pressedBtns = -1; // default pressedBtns
		}

		IFSCLR(0) = 0x100;
	}
} 


int main(){
    return 0;
}