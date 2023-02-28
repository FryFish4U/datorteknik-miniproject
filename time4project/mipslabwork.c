/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog
   This file was modified 2023-03-01 by Alvin Pettersson and David Haendler 

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int mytime = 0x5957;

char textstring[] = "text, more text, and even more text!";

int timeOutCount = 0; //! temp mby, should be able to use integrated counter

int prime = 1234567;

int moreThen = 2000;

int gameSpeedUpEvents = 0;

int characterLane = 1;

volatile uint32_t delayTest;

uint8_t gameMap[512];		

/* Interrupt Service Routine */ 
void user_isr( void ) //! INTENTIONAL: CLEARS ALL FLAGS 'IN CASE OF EMERGENCY'
{
	// if(IFS(0) & 0x800){  // if int.ext.2 flag is 1 
	// 	mytime += 3;      // then +3 seconds passes
	// 	IFSCLR(0) = 0x800;
	// }

	// if(IFS(0) & 0x100){
	// timeOutCount++;

	// if(timeOutCount >= 10){
	// 	time2string( textstring, mytime );
	// 	display_string( 3, textstring );
	// 	display_update();
	// 	tick( &mytime );

	// 	timeOutCount = 0;
	// }
	IFSCLR(0) = 0x8900;
}

/* Lab-specific initialization goes here */
void labinit( void )
{
	volatile int* PortEPointer = (volatile int*) 0xbf886100; // Pointer points to Port E, register TRISE
	TRISDSET = 0x00000fe0;

	/* init timer 2: */
	T2CON = 0x0; // disables timer before config - necessary

	T2CONSET = 0x70; // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
	TMR2 = 0x0; // clears timer counter reg. 
	PR2 = 0x7a12; // 16 bit register - since timer only uses 4 bytes - lower number means slower clock(??) - int value seems to change tick rate

	IPC(2) = 0x1f; // priority 7 sub prio 3 - aka highest priority
	IECSET(0) = 0x100; // enable timer 2 flag

	IPCSET(2) = 0x8e000000; // prio 7 subprio 2 
	IECSET(0) = 0x800; // enable ext.int.2

	IFSCLR(0) = 0x100; // clear timer interupt

	IFSCLR(0) = 0x800; // clear ext.int.2 flag

  	T2CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems
	/* end of timer 2 init */
  
	/* init timer 4: */
	T4CON = 0x0; // disables timer before config - necessary

	T4CONSET = 0x70;    // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
	TMR4 = 0x0;         // clears timer counter reg. 
	PR4 = 0x7a12;       // 16 bit register - since timer only uses 4 bytes
						// - lower number means slower clock (int value defines tick rate)

	IPC(4) = 0x4;       // priority 2 (0b 0100)
	IPCSET(4) = 0x1;    // sub prio 1 (0b xxx1)
	IECSET(0) = 0x1000;  // enable timer 4 flag

	IECSET(0) = 0x8000; // enable ext.int.4

	IPCSET(4) = 0x8e000000; // prio 7 subprio 2 

	T4CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems
	/* end of timer 4 init */


	enable_interrupt(); // enables interuppts via labwork.s

	setup_gameMap();
	showUfo();

	return;
}


void showUfo(void){	// funktion för att ladda in ufot i game map.
	int w = 0;
	for(w = 0 ; w < 19 ; w++){
		gameMap[(characterLane*128) + (10 + w)] = (gameMap[(characterLane*128) + (10 + w)] & ufo[w]);
	} 
	
	//display_update();
	display_image(0, gameMap);
    return;
}

void laneRedirect(){
	int pressedBtns = 0; // int which getbtns should write to. Must be defaulted to 0 after use.

	pressedBtns = getbtns();

		if((pressedBtns & 0b001) && (pressedBtns & 0b100)) // if both move left and right are pressed: default pressedBtns
			pressedBtns = 0; // default pressedBtns
			
		if((pressedBtns & 0b001) && (characterLane > 0)){ // move up if btn 4 is pressed
			characterLane--; // move up
			move_ufo(-1);
			pressedBtns = 0; // default pressedBtns
		}

	    if((pressedBtns & 0b100) && (characterLane < 2)){ // move down if btn 2 is pressed
			characterLane++; // move down
			move_ufo(1);
			pressedBtns = 0; // default pressedBtns
		}
} 

void gameSpeed(){ // code should lower the value of PR4(tickrate 4) when TMR2(counter 2) reaches procedural values 

	int timerCount = TMR2; // saves the value of TMR2(counter 2) in an int for stability, in case it changes value in the middle of function

	if(moreThen < timerCount){ // checks if timerCount is within a set parameter
		PR4 = (0x7a12 / (1 + ((gameSpeedUpEvents + 1) / 10))); // should increase tickrate by 0.1 every time if statement is true
		gameSpeedUpEvents++; // advances gameSpeedUpEvents one, could be used to display the games current level
		moreThen += 2000; // increases the requirment for timerCount in the 'if' statement
	}
}

void move_ufo(int direction){ 	// will accept input to decide whiche direction to move. direction < 0 = upwards. directione > 0 = downwards
								// updates screen afterwards.

	if (direction < 0){		//move upwards
		int timeOutCount = 0;
		int blanksBelow = 128;	// to create blanks below the ship as it leaves the lane
		int blanksAbove = 127;	// to create blanks above the ship as it spawns in
		int shiftAbove = 7;		// how much to shift the ufo template when spawning
		int shiftBelow = 1;		// how much to shift the ufo template when despawing
		int j = 8;				// increment the outer loop
		int i = 0;				// a counter
		uint8_t temp0[19];		// tempporary image array. for the page we move into
		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

		while (j > 0){	// loops 8 times. 1 for each pixel in a page.

			IFSCLR(0) = 0x800; // reset flag for delay

			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);	
				temp1[i] = ((ufo[i] >> shiftBelow) | blanksBelow);	
			}

			for(i = 0 ; i < 19 ; i++){	// this for loop copies out temp value into the map at the right lane
				gameMap[(characterLane*128) + (10 + i)] = ((gameMap[(characterLane*128) + (10 + i)] | 255/* or 255 to reset the image*/ )
				& temp0[i]);
				gameMap[((characterLane + 1)*128) + (10 + i)] = ((gameMap[((characterLane + 1)*128) + (10 + i)] | 255 )	& temp1[i]);
			} 
			blanksAbove = (blanksAbove / 2); 	// decrements how much blank space should be used above
			blanksBelow = ((blanksBelow / 2) + 128);// increments how much blank space should be used below 
			shiftAbove-- ;						// decrements how much of the ufo template to remove above
			shiftBelow++;						// increments how much of the ufo template to remove below
			j--;								// counts itterations. 

			//display_update();
			display_image(0, gameMap);

			while(!(IFS(0) & 0x800)); // delay untill flag event
		}
	}

	if (direction > 0){		//move downwards
		int timeOutCount = 0;
		int blanksBelow = 254;	// to create blanks below the ship as itspawns  
		int blanksAbove = 1;	// to create blanks above the ship as it leaves the lane
		int shiftAbove = 1;		// how much to shift the ufo template when spawning
		int shiftBelow = 7;		// how much to shift the ufo template when despawing
		int j = 8;				// increment the outer loop
		int i = 0;				// counter variable
		uint8_t temp0[19];		// tempporary image array. for the page we move into
		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

		
			

		while (j > 0){	// loops 8 times. 1 for each pixel in a page.
			
			IFSCLR(0) = 0x800; //reset flag for delay

			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);
				temp1[i] = ((ufo[i] >> shiftBelow) | blanksBelow);	
			}

			for(i = 0 ; i < 19 ; i++){	// this for loop copies out temp value into the map at the right lane
				gameMap[((characterLane - 1)*128) + (10 + i)] = ((gameMap[((characterLane - 1)*128) + (10 + i)] | 255 )	& temp0[i]);
				gameMap[((characterLane)*128) + (10 + i)] = ((gameMap[((characterLane)*128) + (10 + i)] | 255 )	& temp1[i]);
			} 

			blanksAbove = ((blanksAbove*2) + 1); 	
			blanksBelow = ((blanksBelow*2) - 256);	
			shiftAbove++;						
			shiftBelow--;						
			j--;								

			//display_update();
			display_image(0, gameMap);

			while(!(IFS(0) & 0x800));  // delay untill flag event
		}
	}
	return;
}

void setup_gameMap(void){
	int row = 0;
	int column = 0;
	for(row = 0 ; row < 4 ; row++){			//this loop is for filling the map with the darkness of space.
		for(column = 0 ; column < 128 ; column++){			
				gameMap[(row*128) + column] = 255;		
		}
	}
}

void create_obstacle (int lane){ // spawns a spaceRock at the end of the map Lane 0 is top lane
    int i = 0;
	int column;
	for(column = 127 ; column > 117 ; column--){			
		gameMap[(lane*128) + column] = gameMap[((lane*128) + column) | 255] & spaceRock[i];
        i++;

        display_image(0, gameMap);

		// delayTest = 0;      //! ska bytas mot timer och interrupt med counter
		// while (delayTest < 100000){
		// 	delayTest ++;	
	    // }   
    }
}

void move_obs(int lane){	 //recives lane argument, 0 is top lane 2is bot lane
	int i = 0;
	int column;
	for(column = 116 ; column > 0 ; column--){		// moves the obs. column by column in it's lane
		IFSCLR(0) = 0x8000;							// clear flag for delay
		for (i = 0 ; i < 10 ; i++){					// updates image
		gameMap[((lane*128) + column) + i] = gameMap[(((lane*128) + column) + i) | 255] & spaceRock[i];
		}
        display_image(0, gameMap);
		while (!(IFS(0) & 0x8000));// dealy until flag event
	}
}

void spawn_obstacle(int bLane){ // bLane checks the 3 LSB and calls a function to create obstacles 
								// in the lanes corresponded by the bits. 
	int l = bLane & 0b111;

	if(l == 0b111)	// if all three lanes should get an obstacle
		return;		// return, due to them being impossible for the player to avoid 

	if(l & 0b100)			// if the 3rd bit is 1, create obstacle in the lowest lane
		create_obstacle(2);	
	if(l & 0b010)			// if the 2nd bit is 1, create obstacle in the middle lane
		create_obstacle(1);	
	if(l & 0b001)			// if the 1st bit is 1, create obstacle in the top lane
		create_obstacle(0);	
}

void explode(int lane){ //! testa funktionen

	int j;
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp1[j];
	}
	display_image(0, gameMap);
    delayTest = 0;      //! ska bytas mot timer och interrupt med counter
    while (delayTest < 100000){
		delayTest++;
	}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp1[j] & exp2[j]);
	}
		display_image(0, gameMap);
	
        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp2[j];
	}
		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp2[j] & exp3[j]);
	}
		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){	
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp3[j];
	}
		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp3[j] & exp4[j]);
	}
		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp4[j];
	}
		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255);
	}

 		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 100000){
			delayTest++;
		}
   
}


//? Templates for timer interrupts:
/* delay with timer 2:
	if(IFS(0) & 0x800){  // if int.ext.2 flag is 1 
		//*[CODE THAT SHOULD BE EXECUTED]
		IFSCLR(0) = 0x800;
	}
*/

/* delay with timer 4:
	if(IFS(0) & 0x8000){  // if int.ext.2 flag is 1 
		//*[CODE THAT SHOULD BE EXECUTED]
		IFSCLR(0) = 0x8000;
	}
*/

/* This function is called repetitively from the main program */
void labwork( void )
{
  	/* start of test code */
	
	//explode(2);

	/* end of test code */

	gameSpeed();

	laneRedirect();

	// if(btnOut | 0b0010)	// if the down button is pressed
	// 	move_ufo(1);	// move the UFO down
	// else				// else
	// if(btnOut | 0b1000)	// if the up button is pressed
	// 	move_ufo(-1);	// move the UFO up

	//display_update();
}
