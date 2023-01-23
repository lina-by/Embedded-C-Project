// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#define OptiBack 1 // determines if the way back if shortened 

#include <xc.h>
#include "stdio.h"
#include "dc_motor.h"
#include "i2c.h"
#include "color.h"
#include "interrupts.h"
#include "timers.h"
#include "card.h"

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  

void main(void){
    // Initialization & Motor structures
    struct DC_motor motorL, motorR;
    initDCmotorsPWM(&motorL, &motorR);
    Interrupts_init();
    Timer0_init();
    color_click_init();
    buggy_init();
    LightInit();

    // Arrays to store the path
    unsigned int forwardtemp;
    unsigned int forwardtime[20];
    char instructions[20];
    char instcounter = 0;
    char inst;    
    
    //Calibrate turn
    calibration(&motorL,&motorR);
    
    while(1){
        // Turn on light and set new integration time
        Light(0); // Turn on all light
        __delay_ms(200); // Wait for a bit
        color_writetoaddr(0x01, 0xF6); // Set to 24ms integration time
        __delay_ms(500); // Wait for a bit
        
        // Set new threshhold, clear interrupt &  reset flag
        set_high_threshold(color_read_Clear()+15); // Set new threshold
        clear_interrupt(); // Clear Interrupt
        clearADCflag = 0; // Reset Flag
        
        // Set timer to 0 and start moving forward
        set_timer(0); // Set timer to 0
        fullSpeed(&motorL,&motorR,1); // Move forward
        
        // Do not procede until the ADC raises a flag
        while (!clearADCflag);
        stop(&motorL,&motorR); // Stop the buggy
        forwardtemp = get16bitTMR0val(); // Store the timer register value temporarily
        ram_wall(&motorL,&motorR,1); // Ram into wall

        // Read colour and store instruction
        color_writetoaddr(0x01, 0xD5); // Set to 101ms integration time
        __delay_ms(200);
        inst = readcard();  // Read the card and retrieve instruction
        forwardtemp = adjusted_dist(inst, forwardtemp); // adjust the difference in detection distance
        instructions[instcounter] = inst;  // Store instruction
        forwardtime[instcounter] = forwardtemp;
        
        // Take a step back and execute instruction
        step(&motorL,&motorR,0);
        navigate(inst,&motorL,&motorR); // Execute instruction
        instcounter++; // Increment instruction counter
        
        // Navigate back to the original path
         while (endflag == 1) {
            Light(4); // Turn off all light
            instcounter--;
            // Navigate the way back using the optimized path function for pink, blue and yellow
            if (instructions[instcounter-1]==2 & OptiBack){OptiUturn(&instcounter, &instructions, &forwardtime, &motorL,&motorR);}            
            else if (instructions[instcounter-1]==3 & OptiBack){OptiYellow(&instcounter, &instructions, &forwardtime, &motorL,&motorR);}
            else if (instructions[instcounter-1]==4 & OptiBack){ OptiPink(&instcounter, &instructions, &forwardtime, &motorL,&motorR);}
             // For other instructions, execute the regular way backwards
            else{
                navigateback(instructions[instcounter],&motorL,&motorR); //executes the opposite of the instructions stored in the array
                set_timer(65535-forwardtime[instcounter]); // Set timer so that the timer interrupt triggers after travelling specified distance
            }
            fullSpeed(&motorL,&motorR,0); // Travel backwards
            while(!timerflag); // Wait for timer flag to be raised
            stop(&motorL,&motorR); // Stop the buggy
            timerflag = 0; // Unset timer flag
            while (instcounter == 0) {LATDbits.LATD3 = 0;} //when the buggy has reached its starting point, the beam light turns off
        }
    }
}
