#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw(void){
    int funnyGuy = (PORTD >> 8) & 0xf; // value = switches turned on in binary
    return funnyGuy;
}

int getbtns (void){
    int susMan = (PORTD >> 5) & 0x7; // value = buttons pressed in binary
    return susMan;
}