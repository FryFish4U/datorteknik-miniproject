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


int timeOutCount = 0; //! temp mby, should be able to use integrated counter

int moreThen = 2000;

int gameSpeedUpEvents = 0; // amount of times PR4 has changed its value

int timesObsMoved = 0;

int characterLane = 9;	// used to limit movement for ufo up or down.

int timer4counter = 0; // used to count timer 4 flag events

//areas
uint8_t map[384];
uint8_t obs_area[444]; //it is biggger than map. goes from ((0 to 2) *148). spot (((0 to 2) *148) + 10) until(( (0 to 2) *148) + 137) overlaps with map
uint8_t ufo_area[57]; // goes from (0 to 2) *19) area is placed in (((0 to 2) *128) + 10)
	

volatile int* PortEPointer = (volatile int*) 0xbf886110; // Pointer goes to Port E, register PORTE (LEDs), used for debug purposes


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
    // setting up the ufo_area on the map. should only be called once. goes in labinit
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

	//IPCSET(2) = 0x8e000000; // prio 7 subprio 2 
	//IECSET(0) = 0x800; // enable ext.int.2

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

	IPC(4) = 0x1f;       // priority 2 (0b 0100)

	IECSET(0) = 0x10000;  // enable timer 4 flag
	IFSCLR(0) = 0x10000;	// clear timer 4 flag
	//IECSET(0) = 0x8000; // enable ext.int.4

	//IPCSET(4) = 0x8e000000; // prio 7 subprio 2 

	T4CONSET = 0x10000; // starts timer - adviced to start at end to minimalize problems
	/* end of timer 4 init */


	enable_interrupt(); // enables interuppts via labwork.s

	setup_ufo_area();
	setup_ufo();

	//* legacy code:	
	// setup_gameMap();
	// showUfo();

	return;
}

int randomInt(char incl0, int toInt){ // generates a random number from 1 (or 0 if incl0 > 0) to toInt  
	//* https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
	
	int internIncl0 = 1;

	if(!incl0) // if incl0 is 0
		internIncl0 = 0; // set internIncl0 = 1 
	
	unsigned int seed = TMR2; // seed value is equal the current tick value
	int randomInt = 0;  

	srand(seed); // seeds rand() with TMR2 value
	randomInt = (rand() % toInt) + incl0; // generates a number between (either 0 or 1 depending on toInt) and toInt
	return randomInt;
}

// void showUfo(void){	// funktion för att ladda in ufot i game map.
// 	int w = 0;
// 	for(w = 0 ; w < 19 ; w++){
// 		gameMap[(characterLane*128) + (10 + w)] = (gameMap[(characterLane*128) + (10 + w)] & ufo[w]);
// 	} 
	
// 	//display_update();
// 	display_image(0, gameMap);
//     return;
// }

// Legacy code:
// void laneRedirect(){
// 	int pressedBtns = 0; // int which getbtns should write to. Must be defaulted to 0 after use.

// 	pressedBtns = getbtns();

// 		if((pressedBtns & 0b001) && (pressedBtns & 0b100)) // if both move left and right are pressed: default pressedBtns
// 			pressedBtns = 0; // default pressedBtns
			
// 		if((pressedBtns & 0b001) && (characterLane > 0)){ // move up if btn 4 is pressed
// 			characterLane--; // move up
// 			move_ufo(-1);
// 			pressedBtns = 0; // default pressedBtns
// 			*PortEPointer = (*PortEPointer & 0xffffff00); // Pointer goes to Port E, register PORTE (LEDs) //! DEBUG

// 		}

// 	    if((pressedBtns & 0b100) && (characterLane < 2)){ // move down if btn 2 is pressed
// 			characterLane++; // move down
// 			move_ufo(1);
// 			pressedBtns = 0; // default pressedBtns
// 			*PortEPointer = (*PortEPointer & 0xffffff00); // Pointer goes to Port E, register PORTE (LEDs) //! DEBUG
// 		}
} 

void gameSpeed(){ // code should lower the value of PR4(tickrate 4) when TMR2(counter 2) reaches procedural values 

	int timerCount = TMR2; // saves the value of TMR2(counter 2) in an int for stability, in case it changes value in the middle of function

	if(moreThen < timerCount){ // checks if timerCount is within a set parameter
		PR4 = (0x7a12 / (1 + ((gameSpeedUpEvents + 1) / 10))); // should increase tickrate by 0.1 every time if statement is true
		gameSpeedUpEvents++; // advances gameSpeedUpEvents one, could be used to display the games current level
		moreThen += 2000; // increases the requirment for timerCount in the 'if' statement
	}
}
void move_ufo (int direction){
// this function will move the ufo in the direction that is indicated by the argument by one pixel at the time for as long as the buttun is held.
// this will be done by udating a set area of pixels in determained by ufo_area

int pressedBtns = 0; // int which getbtns should write to. Must be defaulted to 0 after use.

	pressedBtns = getbtns();

		if((pressedBtns & 0b001) && (pressedBtns & 0b100)){ // if both move left and right are pressed: default pressedBtns
			return;
		}
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
    if((pressedBtns & 0b001) && (characterLane > 0)){ // move up if btn 4 is pressed
        temp[j] = ((temp[j] >> 1) | 0x800); // for moving up, we switch everything to the right and then add at blank space as the "most sigificant bit"
        for (i = 2; i >= 0; i--){
            for (j = 18; j <= 0; j--){ // then we copy it back into the ufo_area but backwards since the least significant bits represent the lowest lane.
                ufo_area[(i*19) + j] = (temp[j] & ufo_area [(i*19) + j]);
                temp[j] = (temp[j] >> 8);
            }
        }
		characterLane --;
    }								
    if((pressedBtns & 0b100) && (characterLane < 17)){ // move down if btn 2 is pressed
											//!  ^check if this number needs to be 18 or 17.
        temp[j] = ((temp[j] << 1) | 1); // for movin down we shift to the left and add an empty space
        for (i = 2; i >= 0; i--){
            for (j = 18; j <= 0; j--){ // same type of write back int ufo_area as above
                ufo_area[(i*19) + j] = (temp[j] & ufo_area [(i*19) + j]);
                temp[j] = (temp[j] >> 8);
            }
        }
		characterLane++;
    }
}

void move_obs(void){ 
//This function will upgrade the obs area from row to move all current obs one row to the left
// should be executed once at every flag event of timer 4 (at first maybe 10time per second with increasing speed if possible.)
// also spawns obstacles
    int i = 0;
    int j = 0;
    timer4counter ++;

    //spawn part
    //if 24 flags, then spawn obstacle:
    if (timer4counter == 24){
        for (i = 0; i < 3; i++){
            int k = 0;
            for (j = 136; j < 147; j++){
                obs_area[j + (i*147)] = (255 & spaceRock[k]); //! right now, if I am correct, thsi will spawn one obstacle in each lane.
                k++;                                          //! conditions must be set so that this will only happen in maximum 2 lanes at a time
            }                                                 //! there should be 6 different situations. lane 1,2,3,1+2,1+3 or 2+3.
        }                                                     //! can this be done wit some kind of loop or do we need 6 different "if" statements?
        timer4counter = 0;
    }

    // this part will move everything one pixel to the left on screen.
    // everytime this function is called, this part should be executed, but if spawn is done, this should execute after spawn
    for (i = 0; i < 3; i++){
        for (j = 0; j < 147; j++){
            obs_area[(i*148) + j] = (255 & obs_area[(i*148) + (j+1)]);
        }
        obs_area[(i*148) + 147] = 255;
    }
}
// //! move ufo works but the whole game locks while it is moving
// void move_ufo(int direction){ 	// will accept input to decide whiche direction to move. direction < 0 = upwards. directione > 0 = downwards
// 								// updates screen afterwards.

// 	if (direction < 0){			//move upwards
// 		int timeOutCount = 0;
// 		int blanksBelow = 128;	// to create blanks below the ship as it leaves the lane
// 		int blanksAbove = 127;	// to create blanks above the ship as it spawns in
// 		int shiftAbove = 7;		// how much to shift the ufo template when spawning
// 		int shiftBelow = 1;		// how much to shift the ufo template when despawing
// 		int j = 8;				// increment the outer loop
// 		int i = 0;				// a counter
// 		uint8_t temp0[19];		// tempporary image array. for the page we move into
// 		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

// 		while (j > 0){	// loops 8 times. 1 for each pixel in a page.
// 		if(IFS(0) & 0x100){
// 			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
// 				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);	
// 				temp1[i] = ((ufo[i] >> shiftBelow) | blanksBelow);	
// 			}

// 			for(i = 0 ; i < 19 ; i++){	// this for loop copies out temp value into the map at the right lane
// 				gameMap[(characterLane*128) + (10 + i)] = ((gameMap[(characterLane*128) + (10 + i)] | 255/* or 255 to reset the image*/ )
// 				& temp0[i]);
// 				gameMap[((characterLane + 1)*128) + (10 + i)] = ((gameMap[((characterLane + 1)*128) + (10 + i)] | 255 )	& temp1[i]);
// 			} 
// 			blanksAbove = (blanksAbove / 2); 	// decrements how much blank space should be used above
// 			blanksBelow = ((blanksBelow / 2) + 128);// increments how much blank space should be used below 
// 			shiftAbove-- ;						// decrements how much of the ufo template to remove above
// 			shiftBelow++;						// increments how much of the ufo template to remove below
// 			j--;								// counts itterations. 

// 			//display_update();
// 			display_image(0, gameMap);
// 			IFSCLR(0) = 0x100;
// 			}
// 		}
// 	}
// 	else
// 	if (direction > 0){		//move downwards
// 		int timeOutCount = 0;
// 		int blanksBelow = 254;	// to create blanks below the ship as itspawns  
// 		int blanksAbove = 1;	// to create blanks above the ship as it leaves the lane
// 		int shiftAbove = 1;		// how much to shift the ufo template when spawning
// 		int shiftBelow = 7;		// how much to shift the ufo template when despawing
// 		int j = 8;				// increment the outer loop
// 		int i = 0;				// counter variable
// 		uint8_t temp0[19];		// tempporary image array. for the page we move into
// 		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

// 		while (j > 0){	// loops 8 times. 1 for each pixel in a page.
			
// 			if(IFS(0) & 0x100){
// 				IFSCLR(0) = 0x100;
// 				for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
// 					temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);
// 					temp1[i] = ((ufo[i] >> shiftBelow) | blanksBelow);	
// 				}

// 				for(i = 0 ; i < 19 ; i++){	// this for loop copies out temp value into the map at the right lane
// 					gameMap[((characterLane - 1)*128) + (10 + i)] = ((gameMap[((characterLane - 1)*128) + (10 + i)] | 255 )	& temp0[i]);
// 					gameMap[((characterLane)*128) + (10 + i)] = ((gameMap[((characterLane)*128) + (10 + i)] | 255 )	& temp1[i]);
// 				} 

// 				blanksAbove = ((blanksAbove*2) + 1); 	
// 				blanksBelow = ((blanksBelow*2) - 256);	
// 				shiftAbove++;						
// 				shiftBelow--;						
// 				j--;								

// 				display_image(0, gameMap);
// 			}
// 		}
// 	}

// 	return;
// }

// Legacy code:
// void setup_gameMap(void){
// 	int row = 0;
// 	int column = 0;
// 	for(row = 0 ; row < 4 ; row++){			//this loop is for filling the map with the darkness of space.
// 		for(column = 0 ; column < 128 ; column++){			
// 				gameMap[(row*128) + column] = 255;		
// 		}
// 	}
// }

// void create_obstacle (int lane){ // spawns a spaceRock at the end of the map Lane 0 is top lane
//     int i = 0;
// 	int column;
// 	for(column = 127 ; column > 117 ; column--){			
// 		gameMap[(lane*128) + column] = gameMap[((lane*128) + column) | 255] & spaceRock[i];
//         i++;
//     }
// 	display_image(0, gameMap);
// }

// void move_obs(void){	//! fucked up from here

// 	// alla lanes en pixel per call

// 	int lane = 0;
// 	int column = 0;
// 	uint8_t temp[512];
// 	for (lane = 0; lane < 3; lane++){
// 		for(column = 0; column <127; column++){
// 			temp[((lane*128) + column)] = (gameMap[((lane*128) + (column + 1))] |255);
// 		}
// 		temp[(lane* 128) + 127] = 255;
// 	}
// 	for (lane = 0; lane < 512; lane ++){
// 		gameMap[lane] = gameMap[lane] & temp[lane];

// 						//! to about here
// 		timesObsMoved++;
// 	}
// 	display_image(0,gameMap);
// 	showUfo();
	
// 	// int i = 0;
// 	// int column;
	
// 	// //if(IFS(0) & 0x100){  // if int.ext.2 flag is 1 
// 	// for(column = 116 ; column > 0 ; column--){		// moves the obs. column by column in it's lane
// 	// 	for (i = 0 ; i < 10 ; i++){					// updates image
// 	// 		gameMap[((lane*128) + column) + i] = gameMap[(((lane*128) + column) + i) | 255] & spaceRock[i];
// 	// 		*PortEPointer += 1; //! DEBUG
// 	// 		gameMap[((lane*128) + column) + 11] = 255;
// 	// 		}
// 	// 	}
// 	// 	//IFSCLR(0) = 0x100;
//     //     display_image(0, gameMap);
// 	// //}
// }

// void spawn_obstacle(int bLane){ // bLane checks the 3 LSB and calls a function to create obstacles 
// 								// in the lanes corresponded by the bits. 
	
// 	int l = bLane & 0b111;

// 	if(l == 0b111)	// if all three lanes should get an obstacle
// 		return;		// return, due to them being impossible for the player to avoid 

// 	if(l & 0b100){			// if the 3rd bit is 1, create obstacle in the lowest lane
// 		create_obstacle(2);
// 	}	
// 	if(l & 0b010){			// if the 2nd bit is 1, create obstacle in the middle lane
// 		create_obstacle(1);	
// 	}
// 	if(l & 0b001){			// if the 1st bit is 1, create obstacle in the top lane
// 		create_obstacle(0);
// 	}	
// }
//! Ska testas också
// int randomInt(char incl0, int toInt){ // generates a random number from 1 (or 0 if incl0 > 0) to toInt  
// 	//* https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
	
// 	int internIncl0 = 1;

// 	if(!incl0) // if incl0 is 0
// 		internIncl0 = 0; // set internIncl0 = 1 
	
// 	unsigned int seed = TMR2; // seed value is equal the current tick value
// 	int randomInt = 0;  

// 	srand(seed); // seeds rand() with TMR2 value
// 	randomInt = (rand() % toInt) + incl0; // generates a number between (either 0 or 1 depending on toInt) and toInt
// 	return randomInt;
// }

void explode(int lane){ //! will not use

	int j;
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp1[j];
	}
	display_image(0, gameMap);
    delayTest = 0;      
    while (delayTest < 100000){
		delayTest++;
	}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp1[j] & exp2[j]);
	}
		display_image(0, gameMap);
	
        delayTest = 0;     
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp2[j];
	}
		display_image(0, gameMap);

        delayTest = 0;    
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp2[j] & exp3[j]);
	}
		display_image(0, gameMap);

        delayTest = 0;     
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){	
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp3[j];
	}
		display_image(0, gameMap);

        delayTest = 0;     
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp3[j] & exp4[j]);
	}
		display_image(0, gameMap);

        delayTest = 0;      
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp4[j];
	}
		display_image(0, gameMap);

        delayTest = 0;     
        while (delayTest < 100000){
			delayTest++;
		}
	for(j = 0; j < 10; j++){
		gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255);
	}

 		display_image(0, gameMap);

        delayTest = 0;  
        while (delayTest < 100000){
			delayTest++;
		}
   
}


//? Templates for timer interrupts:
/* delay with timer 2:
	if(IFS(0) & 0x100){  // if int.ext.2 flag is 1 
		//*[CODE THAT SHOULD BE EXECUTED]
		IFSCLR(0) = 0x100;
	}
*/

/* delay with timer 4:
	if(IFS(0) & 0x10000){  // if int.ext.2 flag is 1 
		//*[CODE THAT SHOULD BE EXECUTED]
		IFSCLR(0) = 0x10000;
	}
*/

void map_update(void){
//what is meant to be done here is to combine all the new information on the top three pages of the display and show it.
//this is done once with every flag event from timer 2 (if possible, 30 times per second?)
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

/* This function is called repetitively from the main program */
void labwork( void )
{
  	/* start of test code */
	
	if (getbtns() & 0b010){
		spawn_obstacle(0b11);
	}

	int aaa = 1;
	while(aaa){
	if(IFS(0) & 0x100){  // if int.ext.2 flag is 1 
		if((*PortEPointer & 0xffffff00) + 255 == *PortEPointer) //! DEBUG
    		*PortEPointer = (*PortEPointer & 0xffffff00); // lets all binary 1s be unchanged except the ones in the last 2 byte //! DEBUG
  		else					//! DEBUG
			*PortEPointer += 1; //! DEBUG
			move_obs();

		IFSCLR(0) = 0x100;
		aaa = 0;
		}	
	}

	/* end of test code */


	//* uses rng to spawn obstacles:
	/* 
	if(timesObsMoved > 30){ // if obstacles has moved 25 pixels //todo: might be able to scale this to difficulity
		timesObsMoved = 0;	// reset timesObsMoved
		spawn_obstacle(randomInt(NULL, 6)); // and spawn random obstacles //! might fuck up, heard srand doesnt work on chipkit
	}
	*/

	gameSpeed();
	laneRedirect();

	// if(btnOut | 0b0010)	// if the down button is pressed
	// 	move_ufo(1);	// move the UFO down
	// else				// else
	// if(btnOut | 0b1000)	// if the up button is pressed
	// 	move_ufo(-1);	// move the UFO up

	//display_update();
}
