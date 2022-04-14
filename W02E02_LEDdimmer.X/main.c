/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 21 November 2020, 09:40
 */

#define F_CPU 3333333

#include <avr/io.h>
#include <util/delay.h>
// Granule, aka. time slice, is calculated above as 7us
#define TIME_SLICE_US 7
// Button pin is pulled up and pressing the button down grounds the pin.
// This way the line value follows the button state:
//     button down = line down (low)
//     button up   = line up   (high)
typedef enum 
{
    DOWN = 0,
    UP
} state_t;

void pwm_period(uint8_t duty)
{
    uint8_t off_duty = UINT8_MAX - duty;
    
    //Duty cycle
    if(duty)
    {
        //Turn LED ON
        PORTF.OUTSET = PIN5_bm;
        //Time slices for duty
        while(duty--){
            _delay_us(7);
        }
    }
    //Rest of the period
    if(off_duty)
    {
        //Turn LED OFF
        PORTF.OUTCLR = PIN5_bm;
        //Time slices for off-duty
        while(off_duty--)
        {
            _delay_us(7);
        }
    } 
}

int main(void)
{
    //Track button state changes 
    state_t btn_is = UP, btn_was = UP;
    //Chance value of 'duty' UP or Down
    state_t duty_goes = UP;
    //The duty starts at 50 %
    uint8_t duty = 0x80;
    
    PORTF.DIRSET = PIN5_bm;  //PF5 is LED
    PORTF.DIRCLR = PIN6_bm;  //PF6 is BUTTON
    
    while(1)
    {
        btn_is = (PORTF.IN & PIN6_bm)?UP:DOWN;
        if(btn_is == DOWN)
        {
            //If just pressed down, toggle direction
            if(btn_is != btn_was)
                duty_goes = (duty_goes == UP)?DOWN:UP;
            //increase or decrease duty
            if(duty_goes == UP && duty < UINT8_MAX)
                duty++;
            else if(duty_goes == DOWN && duty > 0x00)
                duty--;
        }
        btn_was = btn_is;
        pwm_period(duty);
    }
}
