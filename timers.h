#ifndef _timers_H
#define _timers_H

#include <xc.h>

#define _XTAL_FREQ 64000000

// Function prototypes
void Timer0_init(void);
unsigned int get16bitTMR0val(void);
void set_timer(unsigned int timerval);

#endif
