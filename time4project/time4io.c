#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw(void){
    int switchesOn = (PORTD >> 8) & 0xf; // value = switches turned on in binary
    return switchesOn;
}

int getbtns (void){
    int buttonsOn = (PORTD >> 5) & 0x7; // value = buttons pressed in binary
    return buttonsOn;
}