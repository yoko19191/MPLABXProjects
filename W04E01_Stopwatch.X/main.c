/*
 * File:   main.c
 * Author: Guanghang Chen
 *
 * Created on 23 November 2020, 15:25
 */

#define F_CPU 3333333
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/sleep.h>
#include <stdio.h>


uint16_t count = 0;        //count every second

inline void LED0_toggle(void);
void RTC_init(void);
void USART0_init(void);
void USART0_sendChar(char c);
void USART0_sendString(char *str);

//copy-paste from gitlab
//utilities for RTC: 
void RTC_init(void)
{
    uint8_t temp;
    
    temp = CLKCTRL.XOSC32KCTRLA;  //Init Oscillator in RTC
    temp &= ~CLKCTRL_ENABLE_bm; //Disable oscillator 
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp); //register protection
    
    while(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm)
    {
        ; /* Wait until XOSC32KS becomes 0 */
    }
    
    /* SEL = 0 (Use External Crystal): */
    temp = CLKCTRL.XOSC32KCTRLA;
    temp &= ~CLKCTRL_SEL_bm;
    /* Writing to protected register */
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    
    /* Enable oscillator: */
    temp = CLKCTRL.XOSC32KCTRLA;
    temp |= CLKCTRL_ENABLE_bm;
    /* Writing to protected register */
    ccp_write_io((void*)&CLKCTRL.XOSC32KCTRLA, temp);
    
    /* Initialize RTC: */
    while (RTC.STATUS > 0)
    {
        ; /* Wait for all register to be synchronized */
    }

    /* 32.768kHz External Crystal Oscillator (XOSC32K) */
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;

    /* Run in debug: enabled */
    RTC.DBGCTRL = RTC_DBGRUN_bm;
    
    RTC.PITINTCTRL = RTC_PI_bm; /* Periodic Interrupt: enabled */
    
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 32768 */
                 | RTC_PITEN_bm; /* Enable: enabled */
}//RTC_init  (Nothing change)

inline void LED0_toggle(void)
{
    PORTB.OUTTGL |= PIN5_bm;
}//LED_toggle (Nothing change)

ISR(RTC_PIT_vect)
{
    /* Clear flag by writing '1': */
    RTC.PITINTFLAGS = RTC_PI_bm;
    
    LED0_toggle();
          
    
    count++; //count ++ every second
    
}//ISR(RTC_PIT_vect)

//IMPORTANT
ISR(PORTF_PORT_vect)
{
    // Clearing an interrupt flag
    PORTF.INTFLAGS = PORTF.INTFLAGS;
    
    if(RTC.PITCTRLA & RTC_PI_bm)//if program is running 
    {
        RTC.PITCTRLA &= ~(RTC_PITEN_bm);//stop()
    }
    else        //if program is stop
    {
        if(count != 0) //and count !=0, reset count
        {
            count = 0;
            printf("RESET\r\n");//reset the counted seconds
        }
        else
        {
            RTC.PITCTRLA |= RTC_PITEN_bm;
        }
    }
    
}//ISR(PORTF_PORT_vect)


//copy-paste from Getting...USART.pdf ch4 
//utilities for send string via USART

int USART0_printChar(char c, FILE *stream)
{
    USART0_sendChar(c); 
    return 0;
}
//regard printChar as stream
static FILE USART_stream = FDEV_SETUP_STREAM(USART0_printChar, NULL, _FDEV_SETUP_WRITE);

void USART0_sendChar(char c)
{    
    while (!(USART0.STATUS & USART_DREIF_bm))
    {        
        ;    
    }            
    USART0.TXDATAL = c;
}//USART0_sendChar
void USART0_sendString(char *str)
{    
    for(size_t i = 0; i < strlen(str); i++) 
    {        
        USART0_sendChar(str[i]);    
    }
}//USART_sendingString

void USART0_init(void)
{    PORTA.DIR &= ~PIN1_bm;    
     PORTA.DIR |= PIN0_bm;    
     USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);    
     USART0.CTRLB |= USART_TXEN_bm; 
     
     stdout = &USART_stream; //replace standard output stream with user define stream, easy for use printf();
}//USART0_init

int main(void) 
{   
    USART0_init();
   //Initialize all stuff
    PORTF.DIRSET = PIN5_bm; //LED
    PORTF.DIRCLR = PIN6_bm; //Button
    PORTF.PIN6CTRL = PORT_ISC_FALLING_gc; // reset button status
    
    RTC.PITINTCTRL |= RTC_PI_bm; //Enable RTC
    RTC.PITCTRLA |= RTC_PERIOD_CYC32768_gc; //enable RTC clock cycle 32768
    RTC.PITCTRLA &= ~(RTC_PITEN_bm); //stop RTC
    
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc); //"sleeping is in the initialization part of your main function"
    
    /* Enable Global Interrupts */
    sei();
    while (1) 
    {   
        //Go to sleep after Every ISR turn
        sleep_mode();
        if((RTC.PITCTRLA & RTC_PI_bm) && (count!=0))
        {
            printf("%d\r\n", count);
        }
        
    }
}
