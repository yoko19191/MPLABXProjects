/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on November 14, 2020, 2:27 PM
 */

//reference code
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

static bool is_sleeping;
ISR(INT0_vect)
{
    PORTF.OUTSET = PIN5_bm;
    is_sleeping = true;
}

int main(void) {
    /* Replace with your application code */
    PORTF.DIRSET = PIN5_bm;
    PORTF.DIRCLR = PIN6_bm;
    while (1) {
        if(PORTF.IN & PIN6_bm){
            PORTF.OUTSET = PIN5_bm;
        }
        else
        {
            PORTF.OUTCLR = PIN5_bm;
        }
        
        if(is_sleeping){
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
        }
    }
}
