/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on November 13, 2020, 7:37 PM
 */


#include <avr/io.h>

int main(void) {
    /* Replace with your application code */
    PORTF.DIRSET = PIN5_bm; //Read LED Status
    PORTF.DIRCLR = PIN6_bm; //Read Button Status
    
    while (1) {
        
        if(PORTF.IN & PIN6_bm) //IF HIGH
        {
            PORTF.OUTSET = PIN5_bm;
        }
        else
        {
            PORTF.OUTCLR = PIN5_bm;
        }
        
        
        
    }
}
