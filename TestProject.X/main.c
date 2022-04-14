/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 15 December 2020, 22:14
 */


#include <avr/io.h>

int main(void) {
    /* Replace with your application code */
    while (1) {
        
        PORTC.DIR |= PIN0_bm;
        PORTC.DIR |= PIN2_bm;
    }
}
