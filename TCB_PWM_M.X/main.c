/*
 * File:   main.c
 * Author: dtek0068
 *
 * Created on 18 December 2020, 13:31
 */

#define F_CPU    3333333


#define TCB_CMP_VALUE  (0x80FF)


#include <avr/io.h>
#include <xc.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>  
#include <stdio.h>


#define TCB_CMP_EXAMPLE_VALUE   (0x80FF)

void CLOCK_init (void);
void PORT_init (void);
void TCB3_init (void);

void CLOCK_init (void)
{
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    /* Enable Prescaler and set Prescaler Division to 64 */
    CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm;
    
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    /* Select 32KHz Internal Ultra Low Power Oscillator (OSCULP32K) */
    CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
    
    /* Wait for system oscillator changing to finish */
    while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm)
    {
        ;
    }
}

void PORT_init (void)
{
    PORTB_DIR |= PIN5_bm;
    PORTB_OUT |= PIN5_bm;
}

void TCB3_init (void)
{
    /* Load CCMP register with the period and duty cycle of the PWM */
    TCB3.CCMP = TCB_CMP_EXAMPLE_VALUE;

    /* Enable TCB3 and Divide CLK_PER by 2 */
    TCB3.CTRLA |= TCB_ENABLE_bm;
    TCB3.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
    
    /* Enable Pin Output and configure TCB in 8-bit PWM mode */
    TCB3.CTRLB |= TCB_CCMPEN_bm;
    TCB3.CTRLB |= TCB_CNTMODE_PWM8_gc;
}

int main(void)
{
    CLOCK_init();
    PORT_init();
    TCB3_init();
    
    while (1)
    {
        ;
    }
}
