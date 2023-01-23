#include <xc.h>
#include "dc_motor.h"

// Define constants for motor calibration
#define MAX_Power 12
#define STEP_time 690
#define SQUARE_time 890
#define TURN_delay 800
#define TURN_Power 40
#define TURN_time 149

// Function to initialize buggy pins
void buggy_init(void)
{
    // Buggy headlight and turn signal initialization
    LATDbits.LATD3 = 1; //Beam headlight
    LATFbits.LATF0 = 0; //Left turn signal
    LATHbits.LATH0 = 0; //Right turn signal
    LATDbits.LATD4 = 0; // Brake lights
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD3 = 0;
    TRISFbits.TRISF0 = 0;
    TRISHbits.TRISH0 = 0;
}

// Function to check on the battery voltage
void check_batt(void)
{
    // BAT Vsense pin
    TRISFbits.TRISF6 = 1;
    ANSELFbits.ANSELF6 = 1;
    LATDbits.LATD7 = 0;
    TRISDbits.TRISD7 = 0;
    
    // Set up the ADC module - check section 32 of the datasheet for more details
    ADREFbits.ADNREF = 0; // Use Vss (0V) as negative reference
    ADREFbits.ADPREF = 0b00; // Use Vdd (3.3V) as positive reference
    ADPCH=0b101110; // Select channel RF6 for ADC
    ADCON0bits.ADFM = 0; // Left-justified result (i.e. no leading 0s)
    ADCON0bits.ADCS = 1; // Use internal Fast RC (FRC) oscillator as clock source for conversion
    ADCON0bits.ADON = 1; // Enable ADC
    
    // Measure Vsense
    // Vsense should be 3.7 / 3 = 1.233V
    // (1.233/3.3) * 255 = 95.3
    unsigned int tmpval;
    ADCON0bits.GO = 1; // Start ADC conversion
    while (ADCON0bits.GO); // Wait until conversion done (bit is cleared automatically when done)
    tmpval = ADRESH;
    if (tmpval < 100) {LATDbits.LATD7 = 1;} // Turn on brake lights if voltage significantly lower than 3.7V
}

// function initialise T2 and CCP for DC motor control
void initDCmotorsPWM(DC_motor *mL, DC_motor *mR){
    //initialise your TRIS and LAT registers for PWM  
    TRISEbits.TRISE2 = 0;
    TRISEbits.TRISE4 = 0;
    TRISCbits.TRISC7 = 0;
    TRISGbits.TRISG6 = 0;
    
    LATEbits.LATE2 = 0;
    LATEbits.LATE4 = 0;
    LATCbits.LATC7 = 0;
    LATGbits.LATG6 = 0;
    
    //configure PPS to map CCP modules to pins
    RE2PPS=0x05; //CCP1 on RE2
    RE4PPS=0x06; //CCP2 on RE4
    RC7PPS=0x07; //CCP3 on RC7
    RG6PPS=0x08; //CCP4 on RG6

    // timer 2 config
    T2CONbits.CKPS=0b10; // 1:16 prescaler
    T2HLTbits.MODE=0b00000; // Free Running Mode, software gate only
    T2CLKCONbits.CS=0b0001; // Fosc/4

    // Tpwm*(Fosc/4)/prescaler - 1 = PTPER
    // 0.0001s*16MHz/16 -1 = 99
    T2PR=99; //Period reg 10kHz base period
    T2CONbits.ON=1;
    
    //setup CCP modules to output PMW signals
    //initial duty cycles 
    CCPR1H=0; 
    CCPR2H=0; 
    CCPR3H=0; 
    CCPR4H=0; 
    
    //use tmr2 for all CCP modules used
    CCPTMRS0bits.C1TSEL=0;
    CCPTMRS0bits.C2TSEL=0;
    CCPTMRS0bits.C3TSEL=0;
    CCPTMRS0bits.C4TSEL=0;
    
    //configure each CCP
    CCP1CONbits.FMT=1; // left aligned duty cycle (we can just use high byte)
    CCP1CONbits.CCP1MODE=0b1100; //PWM mode  
    CCP1CONbits.EN=1; //turn on
    
    CCP2CONbits.FMT=1; // left aligned
    CCP2CONbits.CCP2MODE=0b1100; //PWM mode  
    CCP2CONbits.EN=1; //turn on
    
    CCP3CONbits.FMT=1; // left aligned
    CCP3CONbits.CCP3MODE=0b1100; //PWM mode  
    CCP3CONbits.EN=1; //turn on
    
    CCP4CONbits.FMT=1; // left aligned
    CCP4CONbits.CCP4MODE=0b1100; //PWM mode  
    CCP4CONbits.EN=1; //turn on
    
    mL->power=0; 						//zero power to start
    mL->direction=1; 					//set default motor direction
    mL->brakemode=1;						// brake mode (1 = slow decay, 0 = fast decay)
    mL->posDutyHighByte=(unsigned char *)(&CCPR1H);  //store address of CCP1 duty high byte
    mL->negDutyHighByte=(unsigned char *)(&CCPR2H);  //store address of CCP2 duty high byte
    mL->PWMperiod=T2PR; 			//store PWMperiod for motor (value of T2PR in this case)
    mR->power=0; 						//zero power to start
    mR->direction=1; 					//set default motor direction
    mR->brakemode=1;						// brake mode (slow decay)
    mR->posDutyHighByte=(unsigned char *)(&CCPR3H);  //store address of CCP1 duty high byte
    mR->negDutyHighByte=(unsigned char *)(&CCPR4H);  //store address of CCP2 duty high byte
    mR->PWMperiod=T2PR; 			//store PWMperiod for motor (value of T2PR in this case)
    
}

// function to set CCP PWM output from the values in the motor structure
void setMotorPWM(DC_motor *m)
{
    unsigned char posDuty, negDuty; //duty cycle values for different sides of the motor
    
    if(m->brakemode) {
        posDuty=m->PWMperiod - ((unsigned int)(m->power)*(m->PWMperiod))/100; //inverted PWM duty
        negDuty=m->PWMperiod; //other side of motor is high all the time
    }
    else {
        posDuty=0; // other side of motor is low all the time
        negDuty=((unsigned int)(m->power)*(m->PWMperiod))/100; //PWM duty
    }
    
    if (m->direction) {
        *(m->posDutyHighByte)=posDuty;  //assign values to the CCP duty cycle registers
        *(m->negDutyHighByte)=negDuty;       
    } else {
        *(m->posDutyHighByte)=negDuty;  //do it the other way around to change direction
        *(m->negDutyHighByte)=posDuty;
    }
}

//function to stop the robot gradually 
void stop(DC_motor *mL, DC_motor *mR)
{
    while ((mL->power > 0) && (mL->power > 0)){
        mL->power--;
        mR->power--;
        __delay_us(500);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    // Delay function to make sure the buggy is fully stopped physically
    __delay_ms(700);
} 

//function to make the robot turn left 
void Left45(DC_motor *mL, DC_motor *mR)
{  
    (mL->direction) = 0; // Left motors go forward
    (mR->direction) = 1; // Right motors go backward
    while ((mL->power < TURN_Power) && (mL->power < TURN_Power)){
        mL->power++;
        mR->power++;
        __delay_us(TURN_delay);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    // Delay to turn
    __delay_ms(TURN_time);
    stop(mL,mR);
}

//function to make the robot turn right 
void Right45(DC_motor *mL, DC_motor *mR)
{
    (mL->direction) = 1; // Left motors go forward
    (mR->direction) = 0; // Right motors go backward
    while ((mL->power < TURN_Power) && (mL->power < TURN_Power)){
        mL->power++;
        mR->power++;
        __delay_us(TURN_delay);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    // Delay to turn
    __delay_ms(TURN_time);
    stop(mL,mR);
}

//function to make the robot go straight
void fullSpeed(DC_motor *mL, DC_motor *mR, char direction)
{
    (mL->direction) = direction; // Both Motors go Forward
    (mR->direction) = direction;
    while ((mL->power < MAX_Power) && (mL->power < MAX_Power)){
        mL->power++;
        mR->power++;
        __delay_ms(1);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
}

//function to make the robot ram into the wall
void ram_wall(DC_motor *mL, DC_motor *mR, char direction)
{
    (mL->direction) = direction; // Both Motors go Forward
    (mR->direction) = direction;
    while ((mL->power < 35) && (mL->power < 35)){
        mL->power++;
        mR->power++;
        __delay_us(500);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    __delay_ms(900);
    while ((mL->power > 25) && (mL->power > 25)){
        mL->power--;
        mR->power--;
        __delay_us(500);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    __delay_ms(600);
    stop(mL,mR);
}

// Function to make the robot take a step
void step(DC_motor *mL, DC_motor *mR, char direction)
{
    (mL->direction) = direction; // Both Motors go forward
    (mR->direction) = direction;
    while ((mL->power < MAX_Power) && (mL->power < MAX_Power)){
        mL->power++;
        mR->power++;
        __delay_us(900);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    __delay_ms(STEP_time); __delay_ms(STEP_time); __delay_ms(STEP_time);
    stop(mL,mR);
}

// Function to make the robot go half a square
void square(DC_motor *mL, DC_motor *mR, char direction)
{
    (mL->direction) = direction; // Both Motors go forward
    (mR->direction) = direction;
    while ((mL->power < MAX_Power) && (mL->power < MAX_Power)){
        mL->power++;
        mR->power++;
        __delay_us(900);
        setMotorPWM(mL);
        setMotorPWM(mR);
    }
    __delay_ms(SQUARE_time); __delay_ms(SQUARE_time); __delay_ms(SQUARE_time);
    stop(mL,mR);
}

// Function to execute instruction based on the
/*
 * red=0
 * green=1
 * blue=2 
 * yellow=3
 * pink=4
 * orange=5
 * light blue=6
 * white=7
 * black = 8
 */

// Function to take in the instruction and call corresponding motor functions
void navigate(unsigned char inst, DC_motor *mL, DC_motor *mR)
{
    if (inst == 0) {Right45(mL,mR); Right45(mL,mR);} // Red   Turn Right 90
    if (inst == 1) {Left45(mL,mR); Left45(mL,mR);} // Green	Turn Left 90
    if (inst == 2) { // Blue Turn 180
        Left45(mL,mR); Left45(mL,mR); Left45(mL,mR); Left45(mL,mR); ram_wall(mL,mR,0); step(mL,mR,1);}
    if (inst == 3) {square(mL,mR,0); square(mL,mR,0); Right45(mL,mR); Right45(mL,mR);} // Yellow	Reverse 1 square and turn right 90
    if (inst == 4) {square(mL,mR,0); square(mL,mR,0); Left45(mL,mR); Left45(mL,mR);} // Pink	Reverse 1 square and turn left 90
    if (inst == 5) {Right45(mL,mR); Right45(mL,mR); Right45(mL,mR);} // Orange	Turn Right 135
    if (inst == 6) {Left45(mL,mR); Left45(mL,mR); Left45(mL,mR);} // Light blue	Turn Left 135
    if (inst == 7) {endflag = 1;} // White	Finish (return home)
    if (inst == 8) {endflag = 1;} // Black	Maze wall colour
}

// Function to take in the instruction and call corresponding motor functions
void navigateback(unsigned char inst, DC_motor *mL, DC_motor *mR)
{
    if (inst == 0) {Left45(mL,mR); Left45(mL,mR);} // Red   Turn Left 90
    if (inst == 1) {Right45(mL,mR); Right45(mL,mR);} // Green	Turn Right 90
    if (inst == 2) {Left45(mL,mR); Left45(mL,mR); Left45(mL,mR); Left45(mL,mR);} // Blue	Turn 180
    if (inst == 3) {Left45(mL,mR); Left45(mL,mR); square(mL,mR,1); square(mL,mR,1); } // Yellow	Reverse 1 square and turn right 90
    if (inst == 4) {Right45(mL,mR); Right45(mL,mR); square(mL,mR,1); square(mL,mR,1); } // Pink	Reverse 1 square and turn left 90
    if (inst == 5) {Left45(mL,mR); Left45(mL,mR); Left45(mL,mR);} // Orange	Turn Left 135
    if (inst == 6) {Right45(mL,mR); Right45(mL,mR); Right45(mL,mR);} // Light blue	Turn Right 135
    if (inst == 7) {} // White	Finish (return home)
    if (inst == 8) {} // Black	Maze wall colour
}


// Optimized function to navigate back for the blue instruction
//the function is called at the instruction preceding the blue card
void OptiUturn(char *instcounter, char *instructions, unsigned int *forwardtime, DC_motor *mL, DC_motor *mR){
    navigateback(*(instructions+*instcounter),mL,mR); //executes the first instruction
    if (*(forwardtime+*instcounter-1) < *(forwardtime+*instcounter)){ //if the buggy went further than it came from before reading the blue card
        set_timer(65535- (*(forwardtime+*instcounter) - *(forwardtime+*instcounter-1)));
        *(forwardtime+*instcounter-1) = 1;
    } else{
        navigateback(2,mL,mR); //does the Uturn right away
        set_timer(65535- (*(forwardtime+*instcounter-1)- *(forwardtime+*instcounter)));
        (*instcounter)--;
    }
}

// Optimized function to navigate back for the yellow instruction
//the function is called at the instruction preceding the yellow card
void OptiYellow(char *instcounter, char *instructions, unsigned int *forwardtime, DC_motor *mL, DC_motor *mR){
    navigateback(*(instructions+*instcounter),mL,mR);
    if (*(forwardtime+*instcounter-1)>2500){ //if there is no need to go forward to the yellow card before turning right
        *(instructions+*instcounter-1)=0; //executes the turn right away
        *(forwardtime+*instcounter-1)=*(forwardtime+*instcounter-1)-2500; //adjusts time accordingly
        set_timer(65535-*(forwardtime+*instcounter));
    }else{ //the buggy must go forward to the yellow card to find its way back
        set_timer(65535-*(forwardtime+*instcounter));
        *(forwardtime+*instcounter-1)=1;
    } 
}

// Optimized function to navigate back for the pink instruction
//the function is called at the instruction preceding the pink card
void OptiPink(char *instcounter, char *instructions, unsigned int *forwardtime, DC_motor *mL, DC_motor *mR){
    navigateback(*(instructions+*instcounter),mL,mR);
    if (*(forwardtime+*instcounter-1)>2500){ //if there is no need to go forward to the pink card before turning left
        *(instructions+*instcounter-1)=1; //executes the turn right away
        *(forwardtime+*instcounter-1)=*(forwardtime+*instcounter-1)-2500; //adjusts time accordingly
        set_timer(65535-*(forwardtime+*instcounter));
    }else{ //the buggy must go forward to the pink card to find its way back
        set_timer(65535-*(forwardtime+*instcounter));
        *(forwardtime+*instcounter-1)=1;
    } 
}

// Function to calibrate the buggy before it starts
void calibration(DC_motor *mL, DC_motor *mR){
    TRISFbits.TRISF3=1;
    TRISFbits.TRISF2=1;
    ANSELFbits.ANSELF2=0; //turn off analogue input on pin RF2 : left button
    ANSELFbits.ANSELF3=0; //turn off analogue input on pin RF3 : right button
    while(PORTFbits.RF2 && PORTFbits.RF3); // Wait for a button press
    __delay_ms(1000); // Wait for a bit
    int i;
    for (i=0;i<8;i++){Left45(mL,mR);} // Turn 8 times
    while(PORTFbits.RF2 && PORTFbits.RF3); // Wait for a button press again 
    __delay_ms(1000);
}
