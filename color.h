#ifndef _color_H
#define _color_H

#include <xc.h>

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  


/********************************************//**
 *  Function to initialise the colour click module using I2C
 ***********************************************/
void color_click_init(void);

/********************************************//**
 *  Function to write to the colour click module
 *  address is the register within the colour click to write to
 *	value is the value that will be written to that address
 ***********************************************/
void color_writetoaddr(char address, char value);

/********************************************//**
 *  Function to clear the interrupt for the clear ADC
 ***********************************************/
void clear_interrupt(void);

/********************************************//**
 *  Function to set the high threshold for clear ADC interrupt
 ***********************************************/
void set_high_threshold(unsigned int threshold);

/********************************************//**
 *  Function to read each color channel
 *	Returns a 16 bit ADC value representing colour intensity
 ***********************************************/
unsigned int color_read_Red(void);
unsigned int color_read_Blue(void);
unsigned int color_read_Green(void);
unsigned int color_read_Clear(void);

#endif
