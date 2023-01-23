#include <xc.h>
#include "card.h"
#include "color.h"
#include "i2c.h"

// Function to initialize clickerboard LED pins and buggy lights
void LightInit(void){
    TRISGbits.TRISG1=0; //Red light
    TRISFbits.TRISF7=0; //Blue light
    TRISAbits.TRISA4=0; //Green ligt
    LATGbits.LATG1=0; //Red light
    LATFbits.LATF7=0; //Blue light
    LATAbits.LATA4=0; //Green ligt
}

// Function to control the clickerboard LEDs
void Light(unsigned char colorcode){
    if (colorcode==0){ //White light
        LATGbits.LATG1=1; //Red light
        LATFbits.LATF7=1; //Blue light
        LATAbits.LATA4=1; //Green ligt
    }
    if (colorcode==1){ //Red light
        LATGbits.LATG1=1; //Red light
        LATFbits.LATF7=0; //Blue light
        LATAbits.LATA4=0; //Green ligt
    }
    if (colorcode==2){ //Blue light
        LATGbits.LATG1=0; //Red light
        LATFbits.LATF7=1; //Blue light
        LATAbits.LATA4=0; //Green ligt
    }
    if (colorcode==3){ //Green light
        LATGbits.LATG1=0; //Red light
        LATFbits.LATF7=0; //Blue light
        LATAbits.LATA4=1; //Green ligt
    }
    if (colorcode==4){ //No light
        LATGbits.LATG1=0; //Red light
        LATFbits.LATF7=0; //Blue light
        LATAbits.LATA4=0; //Green light
    }
}

// Function to read color and return the color
unsigned char readcard(void){
    struct RGB_val colorL;
    levels(0,&colorL);
    //BLUES
    if (colorL.R<1.3*colorL.G & colorL.R<1.3*colorL.B){return 2;} // Blue
    if (colorL.R<1.6*colorL.G & colorL.R<1.6*colorL.B){return 6;} // Light blue
    //BLACK
    if(colorL.R+colorL.G+colorL.B<1000){return 8;}
    //GREEN
    if (colorL.R<colorL.G){return 1;}
    
    // RED ORANGE PINK YELLOW
    if (colorL.R>colorL.B*1.6 & colorL.R>colorL.G*1.6){ //red, orange, pink, yellow
        //test for pink
        levels(2,&colorL);
        if(colorL.B>4.5*colorL.R){return 4;}
        // test for yellow
        levels(3,&colorL);
        if(colorL.G>5.5*colorL.R){return 3;}
        // test for orange
        if(colorL.G>3.8*colorL.R){return 5;}
        //Otherwise return red
        return 0;
    }
    
    return 7; // White
}

// Function to read the RGB reading
void levels(int i,struct RGB_val *colorL){
        Light(i); // Turn light on according 
        __delay_ms(900);
        colorL->R = color_read_Red(); // Read the RGB reading and return the structure
        colorL->B = color_read_Blue();
        colorL->G = color_read_Green();
}

// Function to return the appropriate distance adjustment due to the difference in 
unsigned int adjusted_dist(int inst, unsigned int dist){
    int diff;
    if (inst==0) {diff = 1000;} // Red
    if (inst==1) {diff = 700;} // Green
    if (inst==2) {diff = 1400;} // Blue
    if (inst==3) {diff = 700;} // Yellow
    if (inst==4) {diff = 700;} // Pink
    if (inst==5) {diff = 700;} // Orange
    if (inst==6) {diff = 700;} // Light blue
    if (inst==7) {diff = 700;} // White
    if (inst==8) {diff = 1400;} // Black
    if (dist<diff) {return 1;} else {return (dist - diff);}
}
