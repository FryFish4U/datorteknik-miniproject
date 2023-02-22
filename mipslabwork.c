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
  volatile int* bigRedArrowE = (volatile int*) 0xbf886100; // Arrow goes to Port E, register TRISE
  TRISDSET = 0x00000fe0;
  
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

  enable_interrupt(); // enables interuppts via labwork.s

  return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{
  prime = nextprime( prime );
  display_string ( 0, itoaconv( prime ));
  display_update();
}
