#include <pic32mx.h>
#include <stdio.h>
#include <stdint.h>
#include "timertlib.h"

void main(){
    // run inits:
    time2init();
    time4init();
    debugInit();
    buttonInit();

    volatile int* PortEPointer = (volatile int*) 0xbf886100; // Pointer points to Port E, register TRISE

    while(1){ // while loop runs indefinetely
        if(getbtns){ // if button press
            *PortEPointer++; // advance PortEPointer(LEDs)
        }
    }
}