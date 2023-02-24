#include <pic32mx.h>
#include <stdio.h>
#include <stdint.h>
#include <porjectLib.h>

int main(){

    T2CON = 0x0; // disables timer before config - necessary

    T2CONSET = 0x70;    // should be 1:256 due to clock being 8 MHz and timer not having an implemented 1:128 prescaler
    TMR2 = 0x0;         // clears timer counter reg. 
    PR2 = 0x7a12;       // 16 bit register - since timer only uses 4 bytes
                        // - lower number means slower clock (int value defines tick rate)
  
    IPC(2) = 0x1f;      // priority 7 sub prio 3 - aka highest priority
    IECSET(0) = 0x100;  // enable timer 2 flag

    IPCSET(2) = 0x8e000000; // prio 7 subprio 2 
    IECSET(0) = 0x800;      // enable external interupts 2

    IFSCLR(0) = 0x100; // clear timer interupt

    IFSCLR(0) = 0x800; // clear external interupts 2 flag

    T2CONSET = 0x8000; // starts timer - adviced to start at end to minimalize problems

    enable_interrupt(); // enables interuppts via labwork.s

    return;
}