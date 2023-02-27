#include <pic32mx.h>
#include <stdio.h>
#include <stdint.h>
#include "projectlib.h"

//! timers 3 och 5 är reserverade för i fall om 32 bitars timers behövs (då de mergar med timer 2 och 4)

void timer2init(){   

    T2CON = 0x0; // disables timer before config - necessary

    T2CONSET = 0x70;    // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
    TMR2 = 0x0;         // clears timer counter reg. 
    PR2 = 0x7a12;       // 16 bit register - since timer only uses 4 bytes
                        // - lower number means slower clock (int value defines tick rate)
  
    IPC(2) = 0x1c;      // priority 7 - aka highest prioirty    (0b 0001 1100)
    IPCSET(2) = 0x3;    // sub prio 3 - aka highest subpriority (0b xxxx xx11)
    IECSET(0) = 0x100;  // enable timer 2 flag

    IPCSET(2) = 0x8e000000; // prio 7 subprio 2 
    IECSET(0) = 0x800;      // enable external interupts 2

    IFSCLR(0) = 0x100; // clear timer interupt

    IFSCLR(0) = 0x800; // clear external interupts 2 flag

    T2CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems

    enable_interrupt(); // enables interuppts via enableint.S
}

void timer4init(){

    T4CON = 0x0; // disables timer before config - necessary

    T4CONSET = 0x70;    // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
    TMR4 = 0x0;         // clears timer counter reg. 
    PR4 = 0x7a12;       // 16 bit register - since timer only uses 4 bytes
                        // - lower number means slower clock (int value defines tick rate)
  
    IPC(4) = 0x4;       // priority 2 (0b 0100)
    IPCSET(4) = 0x1;    // sub prio 1 (0b xxx1)
    IECSET(0) = 0x100;  // enable timer 2 flag

    IPCSET(4) = 0x8e000000; // prio 7 subprio 2 
    IECSET(0) = 0x800;      // enable external interupts 2

    IFSCLR(0) = 0x100; // clear timer interupt

    IFSCLR(0) = 0x800; // clear external interupts 2 flag

    T4CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems
}


void buttonInit(){
  TRISDSET = 0xfe0; // enables inputs of button 2, 3 and 4 
}

void debugInit(){
    volatile int* PortEPointer = (volatile int*) 0xbf886100; // Pointer points to Port E, register TRISE
}

