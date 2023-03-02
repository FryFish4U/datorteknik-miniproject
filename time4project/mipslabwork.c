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

int timesObsMoved = 0; // integer that keeps count of the amount of times the obstacles has been moved
int score = 0; // integer of the players score

int scene = 0; // should be used to tell labwork what 'scene' to display (i.e. if it should display game over or the main game, etc.)
int characterLane = 0;	// used to limit movement for ufo up or down.

int timer4counter = 0; // used to count timer 4 flag events

int obsCounter = 1;

//areas
uint8_t map[384];
uint8_t obs_area[414]; //it is biggger than map. goes from ((0 to 2) *138). spot (((0 to 2) *138)) until(( (0 to 2) *138) + 127) overlaps with map
uint8_t ufo_area[57]; // goes from (0 to 2) *19) area is placed in (((0 to 2) *19) + 10)
	

volatile int* PortEPointer = (volatile int*) 0xbf886110; // Pointer goes to Port E, register PORTE (LEDs), used for debug purposes


/* Interrupt Service Routine */ 
void user_isr( void )
{
	int buttons = getbtns();
	
	int aaa = 1;
	while(aaa){
	if(IFS(0) & 0x100){  // if int.ext.2 flag is 1 
		if((*PortEPointer & 0xffffff00) + 255 == *PortEPointer) //! DEBUG
    		*PortEPointer = (*PortEPointer & 0xffffff00); // lets all binary 1s be unchanged except the ones in the last 2 byte //! DEBUG
  		else					//! DEBUG
			*PortEPointer += 1; //! DEBUG

		timer4counter++;

		if(buttons)
			move_ufo(buttons);
		
		move_obs(obsCounter);
		map_update();

		IFSCLR(0) = 0x100;
		aaa = 0;
		}
	}
}

void setup_ufo_area(void) { //* by David
    // setting up the area first time. called once in init before setting up the ufo on the map (setup_ufo();)
    int i = 0;
    int j = 0;

    for (i = 0; i < 3; i++){
        for (j = 0; j < 19; j++){
           if (i == 0){
                ufo_area[(i*19) + j] = (ufo[j]); //since a 1 on the screen means black. the 0s in ufo will be visible by bitwise AND
            }
            else{
                ufo_area[(i*19) + j] = 255; // rest of the area is black
           }
        }
    }
}

// this sets the map initially to all black
void setup_map(void){ //* by David 
	int i = 0;
    int j = 0;

	for (i = 0; i < 3; i++){
        for (j = 0; j < 128; j++){
            map[(i*128) + j] = 255;
		}
	}
	display_image(0, map);
	return;
}

    // setting up the ufo_area on the map. should only be called once. goes in labinit
void setup_ufo(void){//* by David

    int i = 0;
    int j = 0;

    for (i = 0; i < 3; i++){
		int k = 0;
        for (j = 0; j < 128; j++){
            if ((j > 9) && (j < 29)){
                map[(i*128) + j] = (255 & ufo_area[(i*19) + k]);
				k++;
            }
            else{
                 map[(i*128) + j] = 255;
            }
        }
    }
    display_image(0,map);
	return;
}
void setup_obs_area (void){
	int i = 0;
	for (i = 0; i < 414; i++){
		obs_area[i] = 255;
	}
}

/* Lab-specific initialization goes here */
void labinit( void )
{
	volatile int* PortEPointer = (volatile int*) 0xbf886100; // Pointer points to Port E, register TRISE
	TRISDSET = 0x00000fe0;

	/* init timer 2: */ //* Alvins
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
  
	/* init timer 4: */ //* Alvins
	T4CON = 0x0; // disables timer before config - necessary

	T4CONSET = 0x70;    // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
	TMR4 = 0x0;         // clears timer counter reg. 
	PR4 = 0x7a12;       // 16 bit register - since timer only uses 4 bytes
						// - lower number means slower clock (int value defines tick rate)

	IPC(4) = 0x1f;       // priority 7 sub prio 3 - aka highest priority

	IECSET(0) = 0x10000;  // enable timer 4 flag
	IFSCLR(0) = 0x10000;	// clear timer 4 flag
	//IECSET(0) = 0x8000; // enable ext.int.4

	//IPCSET(4) = 0x8e000000; // prio 7 subprio 2 

	T4CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems
	/* end of timer 4 init */


	enable_interrupt(); // enables interuppts via labwork.S //* Alvins

	display_update();
	setup_map();
	setup_ufo_area();
	setup_ufo();	

	//* legacy code:	
	// setup_gameMap();
	// showUfo();
	int i = 0;		
	for(i = 0; i < sizeof(obs_area); i++)	// fills obs_area with 1's. when array is initilzed it will be all zeroes untill value is set. that will make the screen white.
		obs_area[i] = 255;

	return;
}

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
// 	int button = 0; // int which getbtns should write to. Must be defaulted to 0 after use.

// 	button = getbtns();

// 		if((button & 0b001) && (button & 0b100)) // if both move left and right are pressed: default button
// 			button = 0; // default button
			
// 		if((button & 0b001) && (characterLane > 0)){ // move up if btn 4 is pressed
// 			characterLane--; // move up
// 			move_ufo(-1);
// 			button = 0; // default button
// 			*PortEPointer = (*PortEPointer & 0xffffff00); // Pointer goes to Port E, register PORTE (LEDs) //! DEBUG

// 		}

// 	    if((button & 0b100) && (characterLane < 2)){ // move down if btn 2 is pressed
// 			characterLane++; // move down
// 			move_ufo(1);
// 			button = 0; // default button
// 			*PortEPointer = (*PortEPointer & 0xffffff00); // Pointer goes to Port E, register PORTE (LEDs) //! DEBUG
// 		}
//} 

void gameSpeed(){ // code should lower the value of PR4(tickrate 4) when TMR2(counter 2) reaches procedural values //* Alvins

	int timerCount = TMR2; // saves the value of TMR2(counter 2) in an int for stability, in case it changes value in the middle of function

	if(moreThen < timerCount){ // checks if timerCount is within a set parameter
		PR4 = (int) (0x7a12 / (1 + ((gameSpeedUpEvents + 1) / 10))); // should increase tickrate by 0.1 every time if the above statement is true
		gameSpeedUpEvents++; // advances gameSpeedUpEvents one, could be used to display the games current level
		moreThen += 2000; // increases the requirment for timerCount in the 'if' statement
	}
}

//* By David
void move_ufo (int button){     // setting up the ufo_area on the map. should only be called once. goes in labinit
// this function will move the ufo in the direction that is indicated by the argument by one pixel at the time for as long as the buttun is held.
// this functions displays a shifted version of the ufo sprite in one of the three lanes at the same time.
// the amount of shifts are dependant on the characterLane variable.
	if((button & 0b001) && (button & 0b100)){ // if both buttons or no button is pressed, m
		return;
	}
	
	int tempClane = 0;
	
    if((button & 0b100) && (tempClane > 0)){ // move up if btn 4 is pressed
		characterLane --;
		tempClane = characterLane;
		
		//create blank space above and below variables
		uint8_t bl_sp_abv;
		uint8_t bl_sp_blw;

		// create the sprite copies
		uint8_t temp0 [19];
		uint8_t temp1 [19];
		uint8_t temp2 [19];

		int i = 0;
		int j = 0;

		// depending on where the characterLane variable (tempClane) is at, we handle the shifts and fillers differently.
		for (i = 0; i < 19; i++){
			if(tempClane < 8){  // if it is moving in the middle and top page
				bl_sp_abv = (254 >> (8 - tempClane));
				bl_sp_blw = 128;
				for (j = 0; j < (7 - tempClane); j++){
					bl_sp_blw = ((bl_sp_blw / 2) + 128);
				}
				temp0[i] = ((ufo[i] << (tempClane)) | bl_sp_abv);
				temp1[i] = ((ufo[i] >> (8 - tempClane)) | bl_sp_blw);
				temp2[i] = 255;
			}
			if((tempClane > 7)){ // if it is moving in the lower and middle page
				uint8_t bl_sp_abv = (254 >> (16 - tempClane));
				bl_sp_blw = 128;
				for (j = 0; j < (15 - tempClane); j++){
					bl_sp_blw = ((bl_sp_blw / 2) + 128);
				}
				temp0[i] = 255;
				temp1[i] = (ufo[i] << ((tempClane - 8))| bl_sp_abv);
				temp2[i] = (ufo[i] >> ((16 - tempClane)) | bl_sp_blw);
			}
		}		
		for (i = 0; i< 19; i++){
			ufo_area[i] = temp0[i];
			ufo_area[19+i] = temp1[i];
			ufo_area[38+i] = temp2[i];
		}
		return;
    }
			// same thing as upwards, but downwards. theese two parts are almost identical and should be possible to make smaller
			// but i have not the time to figure it out.						
    if((button & 0b001) && (tempClane < 16)){ // move down if btn 2 is pressed
		characterLane++;
		int tempClane = characterLane;

		uint8_t bl_sp_abv;
		uint8_t bl_sp_blw;

		uint8_t temp0 [19];
		uint8_t temp1 [19];
		uint8_t temp2 [19];

		int i = 0;
		int j = 0;
		for (i = 0; i < 19; i++){
			if(tempClane < 8){
				bl_sp_abv = (254 >> (8 - tempClane));
				bl_sp_blw = 128;
				for (j = 0; j < (7 - tempClane); j++){
					bl_sp_blw = ((bl_sp_blw / 2) + 128);
				}
				temp0[i] = ((ufo[i] << (tempClane)) | bl_sp_abv);
				temp1[i] = ((ufo[i] >> (8 - tempClane)) | bl_sp_blw);
				temp2[i] = 255;
			}
			if(((tempClane > 7) && (tempClane < 16))){
				uint8_t bl_sp_abv = (254 >> (16 - tempClane));
				bl_sp_blw = 128;
				for (j = 0; j < (15-tempClane); j++){
					bl_sp_blw = ((bl_sp_blw / 2) + 128);
				}
				temp0[i] = 255;
				temp1[i] = (ufo[i] << ((tempClane - 8))| bl_sp_abv);
				temp2[i] = (ufo[i] >> ((16 - tempClane)) | bl_sp_blw);
			}
			if(tempClane == 16){
				temp0[i] = 255;
				temp1[i] = 255;
				temp2[i] = ufo[i];
			}
		}	
			
		for (i = 0; i< 19; i++){
			ufo_area[i] = temp0[i];
			ufo_area[19+i] = temp1[i];
			ufo_area[38+i] = temp2[i];
		}
		return;
    }
}

void move_obs(int version) { //* by david
//This function will upgrade the obs area from row to move all current obs one row to the left
// should be executed once at every flag event of timer 4 (at first maybe 10time per second with increasing speed if possible.)
// also spawns obstacles
    int i = 0;
    int j = 0;
   

   // spawn part
   // if 24 flags, then spawn obstacle:
   // there is 6 different versions. an obstacle spwans in either page 0, 1, 2, 0+1, 0+2 or 1+2;

   // this code seem also like it can be made shorter and more effective, or at leas look less duplicated.
    if (timer4counter == 70){

		if(version == 1){
			obsCounter+= 2;
			for (i = 0; i < 1; i++){ // the "i" variable in each version decides where tho obstacles spawn
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (spaceRock[k]); 
					k++;                                      
				}
			}
		}

		if(version == 2){
			obsCounter += 4;
			for (i = 1; i < 2; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++;                                      
				}
			}

		}

		if(version == 3){
			obsCounter += 2;
			for (i = 2; i < 3; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}   

		if(version == 4){
			obsCounter += (-3);
			for (i = 0; i < 2; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}      

		if(version == 5){
			obsCounter += (-3);
			for (i = 1; i < 3; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}                             

		if(version == 6){
			obsCounter += (-2);
			for (i = 0; i < 3; i++){
           		int k = 0;
				if((i == 0) | (i == 2))
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}                                                                                   
        timer4counter = 0;
    }

    // this part will move everything in obs_area one pixel to the left on screen.
    // everytime this function is called, this part should be executed, but if spawn is done, this should execute after spawn
    for (i = 0; i < 3; i++){
        for (j = 0; j < 137; j++){
            obs_area[(i*138) + j] = (255 & obs_area[(i*138) + (j+1)]);
        }
        obs_area[(i*138) + 137] = 255;
    }
}

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

void map_update(void){ //* by David
//what is meant to be done here is to combine all the new information on the top three pages of the display and show it.
//this is done once with every flag event from timer 2 (if possible, 30 times per second?)
    int i = 0;
    int j = 0;
// first clear the map
	for (i = 0; i < 384; i++){
		map[i] = 255;
	}

//check for hits
	for(i = 0; i < 3; i++){
		uint8_t crashTest = 0;
		for(j = 0; j < 19; j++){
			crashTest = (ufo_area[(i*19)+j] | obs_area[(i*138) + (j+10)]); 	// both ufo and obstacle is represented by zeros.
			if((crashTest != 255)){											// a bitwise or will generate 255 unless there is an overlap
				scene = 2;													// eg. crash
			}
		}
	}
// spawn in new obs area over the map
	for (i = 0; i < 128; i++){
		for (j = 0; j < 3; j++){
			map[(j*128) + i] = (map[(j*128) + i] & obs_area[(j*138) + i]);
		}
	}

// add on top of this also the spawn the updated ufo area in it's set place
	for(i = 0; i < 3; i++){
		for(j = 0; j < 19; j++){
			map[((i*128) + (j+10))] = (map[((i*128) + (j+10))] & ufo_area[(i*19) + j]);
		}
	}
// Display everything with the borrowed and slightly modified display image function.
    display_image(0,map);
}

/* This function is called repetitively from the main program */
void labwork( void )
{
	int buttons = getbtns();
  	/* start of test code */



	/* end of test code */


	/* start of scenes */

	if(scene == -1){
		labinit(); 	// should reset the code back to normal, 
					// if not then there could be a value that is kept outside of mipslabwork.c thats causimg problems

		scene = 0; // move to the main menu scene the next cycle 
	}
	else

	if(scene == 0){
		// should display main menu
		// if buttons are pressed set scene = 1...
		scene = 1; // move to the game scene the next cycle 
	}
	else 
	
	if(scene == 1){
		// the main game should run in here
		// if game over occurs set scene = 2
	}
	else 
	
	if(scene == 2){
		// should display game over screen
		// should also display the score 
		// when button is pressed: scene = -1 to run init all over again which should reset the game back to norm
	}

	/* end of scenes */

	//* uses rng to spawn obstacles:
	/* 
	if(timesObsMoved > 30){ // if obstacles has moved 25 pixels //todo: might be able to scale this to difficulity
		timesObsMoved = 0;	// reset timesObsMoved
		spawn_obstacle(randomInt(NULL, 6)); // and spawn random obstacles //! might fuck up, heard srand doesnt work on chipkit
	}
	*/

	// gameSpeed();
	// laneRedirect();
	// score = (100*timesObsMoved) + (100*timesObsMoved*(gameSpeedUpEvents/2));


	// if(btnOut | 0b0010)	// if the down button is pressed
	// 	move_ufo(1);	// move the UFO down
	// else				// else
	// if(btnOut | 0b1000)	// if the up button is pressed
	// 	move_ufo(-1);	// move the UFO up

	//display_update();
}