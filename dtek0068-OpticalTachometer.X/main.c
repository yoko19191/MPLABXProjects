/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 18 December 2020, 11:42
 */

/**  PIN Assignment for LCD1602
 * D4 - PC4
 * D5 - PC5
 * D6 - PC6
 * D7 - PC7
 * EN - PC0
 * RS - PC1
 */



#include <avr/io.h>
#include <xc.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/sleep.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  

/*-------PRE-PROCESS------- */
#define F_CPU 3333333

#define LCD_Dir  PORTC_DIR			/* Define LCD data port direction */ 
#define LCD_Port PORTC_OUT			/* Define LCD data port */
#define EN 0    /* EN pin to PORTC_PIN0 */  
// (PORTC.IN & (1<<0)) 
#define RS 1     /* RS pin to PORTC_PIN1 */ 


/*------GOLBAL--VARIABLE------*/
volatile int adcValue;
volatile int breakNum;


/*-----------<---LCD--START--->-----------*/
/* LCD COMMAND MODE*/
void LCD_Command(unsigned char cmnd)
{
    LCD_Port = (LCD_Port & 0x0F ) | (cmnd & 0xF0);  /* Sending upper nibble */
    LCD_Port &= ~(1<<RS);         /* RS=0 */
    LCD_Port |= (1<<EN);          /* Enable pulse*/
    
    _delay_us(1);
    
    LCD_Port &= ~(1<<EN);         /* Clear bit in EN */
    
    _delay_us(200);
    
    
    LCD_Port = (LCD_Port & 0x0F) | (cmnd<<4); /*Sending lower nibble */
    LCD_Port |= (1<<EN); 
    _delay_us(1);
    LCD_Port &= ~(1<<EN);
    _delay_ms(2);
}


/* INITIALIZATION */
void LCD_init(void)
{
    LCD_Dir = 0xFF;            /* Make LCD port direction as o/p */
    
    _delay_ms(20);             /*LCD Power on delay always >15ms */
    
    LCD_Command(0x02);         /*Initializes LCD in 4-bit mode*/
    LCD_Command(0x28);         /*Configures LCD in 2-line 4-bit mode and 5*8 matrix*/
    LCD_Command(0x0c);         /*Display on, cursor off */
    LCD_Command(0x06);         /*Increment cursir (shift cursor to right)*/
    LCD_Command(0x01);         /*Clear display screen*/
    
    _delay_ms(2);              /* Clear Display needs 1.64ms */
}

/* FOR CLEARING LCD DISPLAY */
void LCD_Clear()
{
    LCD_Command(0x01);    /* Clear display */
    
    _delay_ms(2);
    
    LCD_Command(0x80);    /* Cursor at home position */
}


/* ACCEPTING CHARACTERS IN LCD */
void LCD_Char(unsigned char data)   /*unsigned char data*/
{   
    LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* Sending upper nibble */
    LCD_Port |= (1<<RS);      /* RS=1, data reg */
    LCD_Port |= (1<<EN);
    
    _delay_us(1);
    
    LCD_Port &= ~(1<<EN);
    
    _delay_us(200);
    
    LCD_Port = (LCD_Port & 0x0F) | (data << 4);   /* Sending lower nibble */
    LCD_Port |= (1<<EN);
    
    _delay_us(1);
    LCD_Port &= ~(1<<EN);
    
    _delay_ms(2);
}
/* Send string to LCD function */
void LCD_String(char *str)
{
    int i;
    for(i=0;str[i]!=0;i++)    /*call LCD_Char() until reach '\0'*/
    {
        LCD_Char(str[i]);
    }
}

/* Send string to LCD with xy position */
void LCD_String_xy(char row, char pos, char *str)
{
    if( (row==0) && (pos<16))
    LCD_Command( (pos & 0x0F) | (0x80) ); /*Command of first row and required position <16*/
    else if (row==1 && pos<16)
    LCD_Command( (pos & 0x0F) | (0xC0) ); /*Command of first row and required postition <16*/
    LCD_String(str);
    
}

 char* int2str(int data)
{   
    static char buffer[6];
    
    int num = data;
    itoa(num,buffer,10);
    
    return buffer;
}
 
/*------------->--LCD--END--<-------------*/

 /*-----------<---RTC-START--->-----------*/
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
 
 /*--------->------RTC-END----<-----------*/
 
 /*---------<-----ADC-START------>--------*/
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
 

/*--------->-----ADC-END------<----------*/
 
/*-----------<---ISR--START--->-----------*/
 ISR(RTC_PIT_vect)
{
    RTC.PITINTFLAGS = RTC_PI_bm;
   
         //adcValue = ADC0_read();
          
         /* In FreeRun mode, the next conversion starts automatically */
         LCD_String_xy(0,4,int2str(adcValue));
            
        
    
    //LCD_String_xy(0,4,int2str(adcValue));
}//new thread rtc_thread
 
ISR(ADC0_RESRDY_vect)
{
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    /*read and update ADC value*/
    
    if(ADC0_conersionDone())
    {
        ADC0_RES = ADC0_read();
        adcValue = ADC0_read();
    }
    adcValue = ADC0_read();
    
}
 
 
 
 /*---------->----ISR--END-----<---------*/
 
 
 
int main(void) 
{
    /* Initialization all stuff  here */
    RTC_init();
    LCD_init();
    ADC0_init();
    ADC0_start();
    /* Something will stay on the screen forever*/
    LCD_String_xy(0,0,"RPM:");
    LCD_String_xy(1,0,"Tachometer");
    
    
    RTC.PITINTCTRL |= RTC_PI_bm; /*Enable RTC*/
    RTC.PITCTRLA |= RTC_PERIOD_CYC32768_gc; /*enable RTC clock cycle 32768*/
    RTC.PITCTRLA &= ~(RTC_PITEN_bm); /*stop RTC*/
    
    
    set_sleep_mode(SLPCTRL_SMODE_IDLE_gc); 
    
    sei();
    while (1) {
        /*Go to sleep after Every ISR turn*/
        sleep_mode();
        
        if((RTC.PITCTRLA & RTC_PI_bm))
        {   
            /*UPDATE LCD EVERY SECOND*/
            //LCD_String_xy(0,4,int2str(adcValue));
        }
        
    }
    
}//main
