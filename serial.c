#include <xc.h>
#include "serial.h"

void initUSART4(void) {
	//code to set up USART4 for Reception and Transmission =
    RC0PPS = 0x12; // Map EUSART4 TX to RC0
    RX4PPS = 0x11; // RX is RC1   
    
    BAUD4CONbits.BRG16 = 0; 	//set baud rate scaling
    TX4STAbits.BRGH = 0; 		//high baud rate select bit
    SP4BRGL = 51; 			//set baud rate to 51 = 19200bps
    SP4BRGH = 0;			//not used

    RC4STAbits.CREN = 1; 		//enable continuos reception
    TX4STAbits.TXEN = 1; 		//enable transmitter
    RC4STAbits.SPEN = 1; 		//enable serial port
	//see readme for detials
}

//function to wait for a byte to arrive on serial port and read it once it does 
char getCharSerial4(void) {
    while (!PIR4bits.RC4IF);//wait for the data to arrive
    return RC4REG; //return byte in RCREG
}

//function to check the TX reg is free and send a byte
void sendCharSerial4(char charToSend) {
    while (!PIR4bits.TX4IF); // wait for flag to be set
    TX4REG = charToSend; //transfer char to transmitter
}


//function to send a string over the serial interface
void sendStringSerial4(char *string){
	//Hint: look at how you did this for the LCD lab 
    while(*string != 0){  // While the data pointed to isn't a 0x00 do below (strings in C must end with a NULL byte) 
		sendCharSerial4(*string++); 	//Send out the current byte pointed to and increment the pointer
	}
}