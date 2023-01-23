#ifndef _card_H
#define _card_H

#include <xc.h>
#include "color.h"
#include "i2c.h"


#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz

//definition of RGB structure
struct RGB_val { 
	unsigned int R;
	unsigned int G;
	unsigned int B;
};

// Function prototypes
void LightInit(void);
void Light(unsigned char colorcode);
unsigned char readcard(void);
void levels(int i,struct RGB_val *colorL);
unsigned int adjusted_dist(int inst, unsigned int dist);


#endif