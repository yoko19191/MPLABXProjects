/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 14 November 2020, 17:09
 */

#define F_CPU 3333333
#include <avr/io.h>
#include <util/delay.h>

//function pwm_period()

void pwm_period(uint8_t duty){        //duty 0-255
    //LED OFF TIME
    for(int i=255-duty;i>0;i--){
        _delay_us(8);
        VPORTF.OUT &= ~PIN5_bm;
    }
    //LED ON TIME
    for(; duty>0;duty--){
        _delay_us(8);
        VPORTF.OUT |= PIN5_bm;
    }
    
}

int main(void) {
    
    //SETUP PIN5
    PORTF.DIRSET = PIN5_bm;
    
    while (1) {
        uint8_t count=255;
        
        //test(255); //255 DARK 0 BRIGHT
        //GET BRIGHT 
        while(count>0)
        {
            count--;
            pwm_period(count);
        }
        //GET DARK
        while(count<255)
        {
            count++;
            pwm_period(count);
        }
        
        
    }
}
