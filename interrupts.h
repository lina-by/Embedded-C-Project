#ifndef _interrupts_H
#define _interrupts_H

#include <xc.h>

#define _XTAL_FREQ 64000000

// Function prototypes
void Interrupts_init(void);
void __interrupt(high_priority) HighISR();


// Flags for interrupt
char clearADCflag = 0; // flag to be raised when the clear ADC sends an interrupt
char timerflag = 0; // flag to be raised when Timer0 overflows

#endif
