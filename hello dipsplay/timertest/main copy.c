#include <pic32mx.h>
#include <stdio.h>
#include <stdint.h>
#include "timertlib.h"

void main(){
    volatile int* PortEPointer = (volatile int*) 0xbf886100; // Pointer points to Port E, register TRISE

    while(1){ // indef. while loop
        if(IFS(0) & 0x100){ // when timer interupt flag is 1...
            *PortEPointer++; // advance PortEPointer
            IFSCLR(0) = 0x100; // clear timer interupt flag
            }
        if(getbtns) // when a button is pressed...
            *PortEPointer += -4; // subtract 4 from PortEPointer 
    }
}