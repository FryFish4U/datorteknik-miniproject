/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

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

/* Interrupt Service Routine */
void user_isr( void )
{
  if(IFS(0) & 0x800){  // if int.ext.2 flag is 1 
     mytime += 3;      // then +3 seconds passes
     IFSCLR(0) = 0x800;
  }

  if(IFS(0) & 0x100){
    timeOutCount++;

    if(timeOutCount >= 10){
        time2string( textstring, mytime );
        display_string( 3, textstring );
        display_update();
        tick( &mytime );

        timeOutCount = 0;
    }
    IFSCLR(0) = 0x100;
  }
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

  
  /* init timer 4: */
  T4CON = 0x0; // disables timer before config - necessary

  T4CONSET = 0x70;    // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
  TMR4 = 0x0;         // clears timer counter reg. 
  PR4 = 0x7a12;       // 16 bit register - since timer only uses 4 bytes
                      // - lower number means slower clock (int value defines tick rate)

  IPC(4) = 0x4;       // priority 2 (0b 0100)
  IPCSET(4) = 0x1;    // sub prio 1 (0b xxx1)
  IECSET(0) = 0x100;  // enable timer 2 flag

  IPCSET(4) = 0x8e000000; // prio 7 subprio 2 

  // todo: lista ut huru man ska interrupta med t4

  T4CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems

  enable_interrupt(); // enables interuppts via labwork.s

  return;
}


void showUfo(void){	// funktion för att ladda in ufot i game map.
	int w = 0;
	for( w ; w < 19 ; w++){
		gameMap[(characterLane*128) + (10 + w)] = (gameMap[(characterLane*128) + (10 + w)] & ufo[w]);
	} 
	
	//display_update();
	display_image(0, gameMap);
    return;
}

int pressedBtns = -1; // int which getbtns should write to. Must be defaulted to -1 after use.

void laneRedirect(){

	pressedBtns = getbtns();

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
} 

void gameSpeed(){ // code should lower the value of PR4(tickrate 4) when TMR2(counter 2) reaches procedural values 

	int timerCount = TMR2; // saves the value of TMR2(counter 2) in an int for stability, in case it changes value in the middle of function

	if(moreThen < timerCount){ // checks if timerCount is within a set parameter
		PR4 = (0x7a12 / (1 + ((gameSpeedUpEvents + 1) / 10))); // should increase tickrate by 0.1 every time if statement is true
		gameSpeedUpEvents++; // advances gameSpeedUpEvents one, could be used to display the games current level
		moreThen += 2000; // increases the requirment for timerCount in the 'if' statement
	}
}

volatile uint32_t delayTest;

void move_ufo(int direction){ // will accept input to decide whiche direction to move. 

	if (direction < 0){		//move upwards
		int timeOutCount = 0;
		int blanksBelow = 128;	// to create blanks below the ship as it leaves the lane
		int blanksAbove = 127;	// to create blanks above the ship as it spawns in
		int shiftAbove = 7;		// how much to shift the ufo template when spawning
		int shiftBelow = 1;		// how much to shift the ufo template when despawing
		int j = 8;				// increment the outer loop
		int i;					// a counter
		uint8_t temp0[19];		// tempporary image array. for the page we move into
		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

		
			

		while (j > 0){	// loops 8 times. 1 for each pixel in a page.

			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);	
				temp1[i] = ((ufo[i] >> shiftBelow)| blanksBelow);	
			}

			int w = 0;
			for( w ; w < 19 ; w++){	// this for loop copies out temp value into the map at the right lane
				gameMap[(characterLane*128) + (10 + w)] = ((gameMap[(characterLane*128) + (10 + w)] | 255/* or 255 to reset the image*/ )
				& temp0[w]);
				gameMap[((characterLane + 1)*128) + (10 + w)] = ((gameMap[((characterLane + 1)*128) + (10 + w)] | 255 )	& temp1[w]);
			} 
			blanksAbove = (blanksAbove /2); 	// decrements how much blank space should be used above
			blanksBelow = ((blanksBelow/2) + 128);// increments how much blank space should be used below 
			shiftAbove -- ;						// decrements how much of the ufo template to remove above
			shiftBelow ++;						// increments how much of the ufo template to remove below
			j --;								// counts itterations. 

			//display_update();
			display_image(0, gameMap);

			delayTest = 0;      //! ska bytas mot timer och interrupt med counter
			while (delayTest < 100000){
				delayTest ++;
			}
		}

	}
	if (direction > 0){		//move downwards
		int timeOutCount = 0;
		int blanksBelow = 254;	// to create blanks below the ship as itspawns  
		int blanksAbove = 1;	// to create blanks above the ship as it leaves the lane
		int shiftAbove = 1;		// how much to shift the ufo template when spawning
		int shiftBelow = 7;		// how much to shift the ufo template when despawing
		int j = 8;				// increment the outer loop
		int i;					// a counter
		uint8_t temp0[19];		// tempporary image array. for the page we move into
		uint8_t temp1[19];		// tempporary image array. for the page we are moving from

		
			

		while (j > 0){	// loops 8 times. 1 for each pixel in a page. //! ska skrivas om för att flytta neråt!!

			for (i = 0; i < 19; i++){ // this for loop fills our temp array with a partial image of the ufo. more with every itteration.
				temp0[i] = ((ufo[i] << shiftAbove) | blanksAbove);
				temp1[i] = ((ufo[i] >> shiftBelow) | blanksBelow);	
			}

			int w = 0;
			for( w ; w < 19 ; w++){	// this for loop copies out temp value into the map at the right lane
				gameMap[((characterLane - 1)*128) + (10 + w)] = ((gameMap[((characterLane - 1)*128) + (10 + w)] | 255 )	& temp0[w]);
				gameMap[((characterLane)*128) + (10 + w)] = ((gameMap[((characterLane)*128) + (10 + w)] | 255 )	& temp1[w]);
			} 
			blanksAbove = ((blanksAbove*2)+1); 	// decrements how much blank space should be used above
			blanksBelow = ((blanksBelow*2)- 256);	// increments how much blank space should be used below 
			shiftAbove ++;						// decrements how much of the ufo template to remove above
			shiftBelow --;						// increments how much of the ufo template to remove below
			j --;								// counts itterations. 

			//display_update();
			display_image(0, gameMap);

			delayTest = 0;  //! ska bytas mot timer och interrupt med counter
			while (delayTest < 1000000){
				delayTest ++;
			}
		}
	}
	return;
}

void setup_gameMap(void){
	int row;
	int column;
	for(row = 0 ; row < 4 ; row++){			//this loop is for filling the map with things.
		for(column = 0 ; column < 128 ; column++){			
				gameMap[(row*128) + column] = 255;		
		}
	}
}

void spawn_obstacle (int lane){ // spawns a spaceRock at the end of the map
    int i = 0;
	int column;
	for(column = 127 ; column > 117 ; column--){			
		gameMap[(lane*128) + column] = gameMap[((lane*128) + column) | 255] & spaceRock[i];
        i++;

        display_image(0, gameMap);

		delayTest = 0;      //! ska bytas mot timer och interrupt med counter
		while (delayTest < 100000){
			delayTest ++;	
	    }   
    }
}

void explode(int lane){ //! testa funktionen

    /*int i;
    int j;
	for (i = 0; i < 8; i++){
		for (j = 0; j < 10; j++){
			if( i == 0){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp1[j];
			}
			if( i == 1){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp1[j] & exp2[j]);
			}
			if( i == 2){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp2[j];
			}
			if( i == 3){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp2[j] & exp3[j]);
			}
			if( i == 4){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp3[j];
			}
			if( i == 5){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp3[j] & exp4[j]);
			}
			if( i == 6){
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & exp4[j];
			}
			else{
				gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255);
			}
		}
 		display_image(0, gameMap);

        delayTest = 0;      //! ska bytas mot timer och interrupt med counter
        while (delayTest < 900000){
			delayTest++;
		}*/

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
    // for(i = 0; i < 7; i++){
    //     if(i % 2 == 0){
    //         for(j = 0; j < 10; j++){
                
    //     }
           
	//     }
    // }  
    //     else{
    //         for(j = 0; j < 10; j++){
    //                 gameMap[(lane*128) + j] = (gameMap[(lane*128) + j] |255) & (exp1[i][j] & exp1[i+1][j]);
    //         }
    //         display_image(0, gameMap);

    //         delayTest = 0;      //! ska bytas mot timer och interrupt med counter
    //         while (delayTest < 100000){
    //             delayTest ++;	
    //         }
    //     } 
    // }
}
/* This function is called repetitively from the main program */
void labwork( void )
{
  /* testing */
	setup_gameMap();
	showUfo();
	characterLane = 2;
 	move_ufo(1);
	spawn_obstacle (2);


  prime = nextprime( prime );
  display_string ( 0, itoaconv( prime ));
  display_update();
}
