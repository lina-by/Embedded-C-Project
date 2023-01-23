#include <xc.h>
#include "timers.h"

/************************************
 * Function to set up timer 0
************************************/
void Timer0_init(void)
{
    T0CON1bits.T0CS=0b010; // Fosc/4
    T0CON1bits.T0ASYNC=1; // see datasheet errata - needed to ensure correct operation when Fosc/4 used as clock source
    T0CON1bits.T0CKPS=0b1111; // 1:32768 (2 minutes)
    T0CON0bits.T016BIT=1;	//16 bit mode		
    // it's a good idea to initialise the timer registers so we know we are at 0
    TMR0H=0;            //write High reg first, update happens when low reg is written to
    TMR0L=0;
    T0CON0bits.T0EN=1;	//start the timer
}

/************************************
 * Function to return the full 16bit timer value
 * Note TMR0L and TMR0H must be read in the correct order, or TMR0H will not contain the correct value
************************************/
unsigned int get16bitTMR0val(void)
{
    unsigned int timerval = TMR0L; // Read TMR0L first
    // Shift TMR0H 8 bits left and add it to the TMR0L value
    timerval = timerval + (TMR0H << 8);
    return (timerval); // Return the value
}

/************************************
 * Function to set the timer value
************************************/
void set_timer(unsigned int timerval)
{
    TMR0H = timerval >> 8; // Write to high register first
    TMR0L = (unsigned char)(timerval); // Then write to low register
}
