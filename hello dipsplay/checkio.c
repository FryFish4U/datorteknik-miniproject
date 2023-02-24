#include <stdio.h>
#include <stdint.h>
#include <pic32mx.h>
#include <porjectLib.h>

int getsw(void){ //! bör inte behövas men finns 
    int switchesOn = (PORTD >> 8) & 0xf; // value = switches turned on in binary
    return switchesOn;
}

int getbtns (void){                     // Only checks button 4 to 2 (RD7-RD5), button 1 is in PORTE and a hassle to implement
    int buttonsOn = (PORTD >> 5) & 0x7; // value = buttons pressed in binary
    return buttonsOn;
}