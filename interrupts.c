#include <xc.h>
#include "interrupts.h"
#include "serial.h"

/************************************
 * Function to turn on interrupts and set if priority is used
 * Note you also need to enable peripheral interrupts in the INTCON register to use CM1IE.
************************************/
void Interrupts_init(void)
{
	// turn on global interrupts, peripheral interrupts and the interrupt source
    // Priority > Enabling > Global
    
    IPR0bits.TMR0IP = 1;    // Set timer interrupt as high priority Necessary?
    PIE0bits.TMR0IE = 1;    //enable timer0 interrupt
    PIE0bits.IOCIE = 1;     // Enable interrupt on change
    IOCBNbits.IOCBN1 = 1;   // Enable negative edge trigger IOC for register B PIN 1
    INTCONbits.PEIE = 1;    //turn on peripheral interrupt
    INTCONbits.GIE = 1;     //turn on interrupts globally
    TRISBbits.TRISB1 = 1;   // Set pin B1 as input
    ANSELBbits.ANSELB1 = 0; // Turn of ADC
}

/************************************
 * High priority interrupt service routine
 * Make sure all enabled interrupts are checked and flags cleared
 * ISR Checks the comaprator and Timer0 Interrupt
************************************/
void __interrupt(high_priority) HighISR()
{
    // Timer0 Interrupt
    if (PIR0bits.TMR0IF){
        timerflag = 1; // Raise timer flag
        PIR0bits.TMR0IF = 0;
    }
    
    // RB! (Colour clicker) Interrupt
    if (IOCBFbits.IOCBF1 = 1){  
        clearADCflag = 1; // Raise clear ADC flag
        IOCBFbits.IOCBF1 = 0; // Reset flag
    }
}