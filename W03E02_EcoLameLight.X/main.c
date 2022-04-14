/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 24 November 2020, 15:07
 */

//reference code
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h> 


int main(void) 
{
    //PF5 is LED(output and inverted)
    PORTF.DIRSET = PIN5_bm;
    PORTF.PIN5CTRL = PORT_INVEN_bm;
    //PF6 is push button(input, inverted, interrupt enabled)
    //Setting a pin as input is unnecessary after startup,
    //but done here anyway for the the same of completeness.
    PORTF.DIRCLR = PIN6_bm;
    //Clearing PORT_ISC with group mask is unnecessary(default: 0),
    PORTF.PIN6CTRL = (PORTF.PIN6CTRL & ~PORT_ISC_gm) |
                        PORT_INVEN_bm| PORT_ISC_BOTHEDGES_gc;
    
    //We can use power-down mode - no peripherals needed
    set_sleep_mode(SLPCTRL_SMODE_PDOWN_gc);
    //Do not forget to enable interrupts
    sei();
    while (1) {
        // Interrupt wakes the CPU up each time it is triggered
        // Because we have nothing to do in the superloop,
        // we simply re-enter sleep mode after each ISR return.
        sleep_mode();
    }
}

ISR(PORTF_PORT_vect)
{
    // Clearing an interrupt flag is done by writing '1' over the
    // corresponding INTFLAG bit. The statement below clears them all.
    // Because SFRs are volatile, the register IS read and written back.
    PORTF.INTFLAGS = PORTF.INTFLAGS;
    //when button is down, LED is on
    if(PORTF.IN & PIN6_bm)
    {
        PORTF.OUTSET = PIN5_bm;
    }
    else
    {
        PORTF.OUTCLR = PIN5_bm;
    }
    
}
