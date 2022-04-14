/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 18 December 2020, 16:01
 */



#define F_CPU    3333333
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)


#define ARR_SIZE  1000

#include <avr/io.h>
#include <xc.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>  
#include <stdio.h>


/*--------GOLABL-------*/
volatile uint16_t adcValue;


/*---------<--USART-START-->---------*/
void USART0_sendChar(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART0.TXDATAL = c;
}

int USART0_printChar(char c, FILE *stream)
{ 
    USART0_sendChar(c);
    return 0; 
}

static FILE USART_stream = FDEV_SETUP_STREAM(USART0_printChar, NULL, _FDEV_SETUP_WRITE);

void USART0_init(void)
{
    PORTA.DIR |= PIN0_bm;
    
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600); 
    
    USART0.CTRLB |= USART_TXEN_bm;  
    
    stdout = &USART_stream;
}

/*--------->--USART-END--<---------*/

/*---------<--ADC-START-->---------*/
void ADC0_init(void)
{
    /* Disable digital input buffer */ /*PE0 is where LDR is connected to*/
    PORTE.PIN0CTRL &= ~PORT_ISC_gm;
    PORTE.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    
    /* Disable pull-up resistor */
    PORTE.PIN0CTRL &= ~PORT_PULLUPEN_bm;
    
    /*Select reference voltage as 2.5V*/
    VREF.CTRLA = VREF_ADC0REFSEL_2V5_gc;
 

    ADC0.CTRLC = ADC_PRESC_DIV16_gc      /* CLK_PER divided by 4 */
               |ADC_REFSEL_INTREF_gc;  /* Internal reference */
    
    ADC0.CTRLA = ADC_ENABLE_bm          /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc;   /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS  = ADC_MUXPOS_AIN8_gc;
    
    /* Enable FreeRun mode */
    ADC0.CTRLA |= ADC_FREERUN_bm;
}


 

uint16_t ADC0_read(void)
{
    /* Clear the interrupt flag by writing 1: */
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    
    return ADC0.RES;
}



void ADC0_start(void)
{
    /* Start conversion */
    ADC0.COMMAND = ADC_STCONV_bm;
}

 
 

bool ADC0_conersionDone(void)
{
    return (ADC0.INTFLAGS & ADC_RESRDY_bm);
}

/*--------->--ADC-END--<----------*/


/*-------RTC--START--------*/

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
}//Copy-paste from StopWatch

/*--------RTC-END--------*/

/*---------<--ISR-START-->----------*/
 ISR(RTC_PIT_vect)
{
    RTC.PITINTFLAGS = RTC_PI_bm;
   
    //printf("%d;", adcVal);
    
            
        
    
    //LCD_String_xy(0,4,int2str(adcValue));
}



/*--------->--ISR-END--<----------*/


int main(void)
{
    /* Initlization all stuff here */
    USART0_init();
    ADC0_init();
    ADC0_start();
    RTC_init();
    
    RTC.PITINTCTRL |= RTC_PI_bm; /*Enable RTC*/
    RTC.PITCTRLA |= RTC_PERIOD_CYC32768_gc; /*enable RTC clock cycle 32768*/
    RTC.PITCTRLA &= ~(RTC_PITEN_bm); /*stop RTC*/
    
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc);
   
    uint16_t arr[1000];
    uint16_t    idx = 0;
   
    //sei();
    while(1)
    {   
        //sleep_mode();
        if (ADC0_conersionDone())
        {
          
            adcValue = ADC0_read();
            if(idx<ARR_SIZE)
            {
                arr[idx++] = adcValue;
                
            }
                
        }
        
        _delay_ms(1);
       
        if (idx >= ARR_SIZE)
        {
            for (uint16_t i = 0; i < ARR_SIZE; i++)
            {
                printf("%u;", arr[i]);
            }
                
            while(1);
        } 
         
    }

}


