/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog
   This file was modified 2023-03-01 by Alvin Pettersson and David Haendler 

   For copyright and licensing, see file COPYING */

void* stdout; 		  // need to compile with stdlib.h		(Alvin)
#include <stdlib.h>	  // need stdlib.h for rand and srand 	(Alvin)

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

/* start of global variables */ //! changes in here should also be made at the end of the labinit function

int timer2counter = 0; // used to count timer 2 flag events
int timer4counter = 0; // used to count timer 4 flag events

int moreThen = 200; // the increment value for gameSpeed //! changes in here should also be made in gameSpeed, should be 200

int gameSpeedUpEvents = 0; // amount of times PR4 has changed its value

int timesObsMoved = 0; // integer that keeps count of the amount of times the obstacles has been moved
int score = 0; // double of the players score //! should be 0

int characterLane = 0;	// used to limit movement for ufo up or down.

int obsCounter = 1;

int scene = 1; // used to switch off game logic when a game over event occurs 

/* end of global variables */


//areas
uint8_t map[384];
uint8_t obs_area[414]; //it is biggger than map. goes from ((0 to 2) *138). spot (((0 to 2) *138)) until(( (0 to 2) *138) + 127) overlaps with map
uint8_t ufo_area[57]; // goes from (0 to 2) *19) area is placed in (((0 to 2) *19) + 10)
	
volatile int* PortEPointer = (volatile int*) 0xbf886110; // Pointer goes to Port E, register PORTE (LEDs), used for debug purposes


int makeRandomInt(char incl0, int toInt){ // generates a random number from 1 (or 0 if incl0 > 0) to toInt  
	//* https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
	
	int internIncl0 = 1;

	if(!incl0) // if incl0 is 0
		internIncl0 = 0; // set internIncl0 = 1 
	
	unsigned int seed = TMR2; // seed value is equal the current tick value
	int returnRandomInt = 0;  

	srand(seed); // seeds rand() with TMR2 value
	returnRandomInt = (rand() % toInt) + incl0; // generates a number between (either 0 or 1 depending on toInt) and toInt
	return returnRandomInt;
}


/* Interrupt Service Routine */ 
void user_isr( void )
{
	if(scene == 1){
		int buttons = getbtns();

		if(IFS(0) & 0x100){  // if int.ext.2 flag is 1 
			if(buttons)
				move_ufo(buttons);
			
			map_update();
			
			timer2counter++;
			IFSCLR(0) = 0x100;
			score = (100*timesObsMoved) + (100*timesObsMoved*(gameSpeedUpEvents/2)); // equation for score calculation
		}


		if(IFS(0) & 0x10000){
			int spawnInt = makeRandomInt(0,6);
			move_obs(spawnInt);

			timer4counter++;
			IFSCLR(0) = 0x10000;
		}
	}
}

void setup_ufo_area(void) { //* by David
    // setting up the area first time. called once in init before setting up the ufo on the map (setup_ufo();)
    int i = 0;
    int j = 0;

    for (i = 0; i < 3; i++){  // "i" decides in which page we are
        for (j = 0; j < 19; j++){	// "j" decides in which column we are
           if (i == 0){				//condition for initial spawn, so we won't get more than one ufo image.
                ufo_area[(i*19) + j] = (ufo[j]); //since a 1 on the screen means black. the 0s in ufo will be visible by bitwise AND
            }
            else{
                ufo_area[(i*19) + j] = 255; // rest of the area is black. 255 is 1111 1111 in binary. with current display_image function, that means all black.
           }
        }
    }
}

// this function sets the map as initially black.
void setup_map(void){   //* by David
	int i = 0;
    int j = 0;

	for (i = 0; i < 3; i++){
        for (j = 0; j < 128; j++){
            map[(i*128) + j] = 255;  // fill each column (j) of each page(i) with 1's.
		}
	}
	display_image(0, map); // display the image.
	return;
}

void setup_ufo(void){   //* by David
    // setting up the ufo_area on the map. should only be called once. goes in labinit
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
void setup_obs_area (void){   //* by David
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
	PR2 = (((0x7a12)/4)*3); // 16 bit register - since timer only uses 4 bytes - lower number means slower clock(??) - int value seems to change tick rate

	IPC(2) = 0x1f; // priority 7 sub prio 3 - aka highest priority
	IECSET(0) = 0x100; // enable timer 2 flag

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

	T4CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems
	/* end of timer 4 init */


	enable_interrupt(); // enables interuppts via labwork.S //* Alvins

	display_update();
	setup_map();
	setup_ufo_area();
	setup_ufo();	

	/* start of variable reset */

	timer2counter = 0;
	timer4counter = 0;

	moreThen = 200;

	gameSpeedUpEvents = 0;

	timesObsMoved = 0;
	score = 0; 

	characterLane = 0;

	obsCounter = 1;

	/* end of variable reset */

	int i = 0;		
	for(i = 0; i < sizeof(obs_area); i++)	// fills obs_area with 1's. when array is initilzed it will be all zeroes untill value is set. that will make the screen white.
		obs_area[i] = 255;					 //* by David

	return;
}


void gameSpeed(){ // code should lower the value of PR4(tickrate 4) when TMR2(counter 2) reaches procedural values //* Alvins
	
	if(moreThen < timer2counter){ // checks if timerCount is within a set parameter
		PR4 = (0x7a12 / (1 + (gameSpeedUpEvents))); // should increase tickrate every time if the above statement is true
		gameSpeedUpEvents++; // advances gameSpeedUpEvents one, could be used to display the games current level
		moreThen += 200; // increases the requirment for timerCount in the 'if' statement
	}
}

void move_ufo (int button){  //* by David
// this function will move the ufo in the direction that is indicated by the argument by one pixel at the time for as long as the buttun is held.

	if((button & 0b001) && (button & 0b100)){ // if both move left and right are pressed: default button
		return;
	}

	int tempClane = characterLane; // first we copy character lane to see where we are moving from

	// the following if statements check which buttun is pressed. that indicates in what direction we are moving
	// the check with tempClane is to decide if we are allowed to move further in that direction.
	// assuming a move, characterLane is then adjusted accordingly
	if((button & 0b100) && (tempClane > 0)){ // move up if btn 4 is pressed
		characterLane --;
	}
									
	if((button & 0b001) && (tempClane < 16)){ // move down if btn 2 is pressed
		characterLane++;
	}
	// tempClane i uppdated with a the new value
	tempClane = characterLane;
	// we create variables used to represent "black space" above or below our ufo.
	uint8_t bl_sp_abv;
	uint8_t bl_sp_blw;

	// we create temporary arrays to carry an image each of the ufo but shifted the right amount depending on our position.
	uint8_t temp0 [19];
	uint8_t temp1 [19];
	uint8_t temp2 [19];

	int i = 0;
	int j = 0;

	// in this next segment we have our movment logic.
	for (i = 0; i < 19; i++){ // whatever we do, we want to do it 19 times to fill the entire width of the ufo_area.
		if(tempClane < 8){	//depending on where we are moving to, we need different images.
			// first we set how much black space we need around the ufo image
			// the space above is more straightforward
			bl_sp_abv = (254 >> (8 - tempClane));
			bl_sp_blw = 128;
			// the space below is more cmoplex. it is x1= x0 /2 +x0, x2= x1 /2 +x0. and so on. 
			for (j = 0; j < (7 - tempClane); j++){
				bl_sp_blw = ((bl_sp_blw / 2) + 128);
			}
			// then we decide how much we need to shift out images
			temp0[i] = ((ufo[i] << (tempClane)) | bl_sp_abv);
			temp1[i] = ((ufo[i] >> (8 - tempClane)) | bl_sp_blw);
			temp2[i] = 255; // since we are moving here in the uppwe two lanes, the third one is all black.
		}
			if(((tempClane > 7) && (tempClane < 16))){  // same thing but adjustet for the two lower lanes.
			uint8_t bl_sp_abv = (254 >> (16 - tempClane));
			bl_sp_blw = 128;
			for (j = 0; j < (15 - tempClane); j++){
				bl_sp_blw = ((bl_sp_blw / 2) + 128);
			}
			temp0[i] = 255;
			temp1[i] = (ufo[i] << ((tempClane - 8))| bl_sp_abv);
			temp2[i] = (ufo[i] >> ((16 - tempClane)) | bl_sp_blw);
		}
			if(tempClane == 16){// edge case of being completely in the bottom lane.
				temp0[i] = 255;
				temp1[i] = 255;
				temp2[i] = ufo[i];
			}
	}		
	
	// after we have assigned the proper images to our Temps, we write them in their respective lane into our ufo area
	for (i = 0; i< 19; i++){
		ufo_area[i] = temp0[i];
		ufo_area[19+i] = temp1[i];
		ufo_area[38+i] = temp2[i];
	}
	// now we are ready to send our updated area to the map update function.
	return;
}

void move_obs(int spawnObs) {   //* by David
//This function will upgrade the obs area from row to move all current obs one row to the left
// should be executed once at every flag event of timer 4 
// also spawns obstacles
    int i = 0;
    int j = 0;

   // spawn part
   // if 70 flags, then spawn obstacle:
   // there is 6 different spawnObs. an obstacle spwans in either page 0, 1, 2, 0+1, 0+2 or 1+2;
   // we use the bigger array that is the obstacle area to spawn obstacles out of screen. 
   // this way they will seemingly fly in from outside the edge.
    if (timer4counter == 70){
		if(spawnObs == 1){
			obsCounter+= 2;
			for (i = 0; i < 1; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (spaceRock[k]); 
					k++;                                      
				}
			}
		}

		if(spawnObs == 2){
			obsCounter += 4;
			for (i = 1; i < 2; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++;                                      
				}
			}

		}

		if(spawnObs == 3){
			obsCounter += 2;
			for (i = 2; i < 3; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}   

		if(spawnObs == 4){
			obsCounter += (-3);
			for (i = 0; i < 2; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}      

		if(spawnObs == 5){
			obsCounter += (-3);
			for (i = 1; i < 3; i++){
           		int k = 0;
				for (j = 126; j < 136; j++){
					obs_area[j + (i*137)] = (255 & spaceRock[k]); 
					k++; 
				}
			}
		}                             

		if(spawnObs == 6){
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

	timesObsMoved++;
}


void map_update(){ //* by David
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
			crashTest = (ufo_area[(i*19)+j] | obs_area[(i*138) + (j+10)]); 	// since both the ufo and obstacles are presented on the map
			if((crashTest != 255)){											// by zeros, a bitwise OR will show of if there has been a hit.
				scene = 2;// go to exist scene.
			}
		}
	}

	/* written by Alvin, diplays score */
		int tempScore = score; // sets current score as in a temporary int for safe manipulation
		
		char scoreLabel[17] = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
								// declares a char array that is filled with 0
		char scoreText[8] = "Score: "; // declares a char array with the characters "Score: "
		
		for(i = 0 ; scoreText[i] != '\0' ; i++) // loops until scoreText[i] is empty
			scoreLabel[i] = scoreText[i]; // sets the character in scoreText to scoreLabel in the same position

		// then 
		for(i  = 17; i > 7 ; i--){ // loops until it reaches the "Score: " chars
			scoreLabel[i] = ((tempScore % 10) + '0'); // takes the first decimal in the tempScore int
			tempScore = tempScore / 10; // moves the decimals to the right by division
		}


		display_string(3, scoreLabel); // readies char array for display at the bottom part of the screen
		display_update(); // displays score
	/* end of score display */

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
	if (scene == 1){
		gameSpeed();
	}
}