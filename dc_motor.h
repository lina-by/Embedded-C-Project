#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H



#include <xc.h>
#include "timers.h"

#define _XTAL_FREQ 64000000

// Flag to be raised when the buggy reaches the end of the maze
char endflag = 0;

typedef struct DC_motor { //definition of DC_motor structure
    char power;         //motor power, out of 100
    char direction;     //motor direction, forward(1), reverse(0)
    char brakemode;		// short or fast decay (brake or coast)
    unsigned int PWMperiod; //base period of PWM cycle
    unsigned char *posDutyHighByte; //PWM duty address for motor +ve side
    unsigned char *negDutyHighByte; //PWM duty address for motor -ve side
} DC_motor;


//function prototypes
void initDCmotorsPWM(DC_motor *mL, DC_motor *mR); // function to setup PWM
void buggy_init(void);
void setMotorPWM(DC_motor *m);
void stop(DC_motor *mL, DC_motor *mR);
void Left45(DC_motor *mL, DC_motor *mR);
void Right45(DC_motor *mL, DC_motor *mR);
void fullSpeed(DC_motor *mL, DC_motor *mR, char direction);
void ram_wall(DC_motor *mL, DC_motor *mR, char direction);
void step(DC_motor *mL, DC_motor *mR, char direction);
void navigate(unsigned char inst, DC_motor *mL, DC_motor *mR);
void navigateback(unsigned char inst, DC_motor *mL, DC_motor *mR);
void square(DC_motor *mL, DC_motor *mR, char direction);
void calibration(DC_motor *mL, DC_motor *mR);
void OptiUturn(char *instcounter, char *instructions, unsigned int *forwardtime, DC_motor *mL, DC_motor *mR);
void OptiYellow(char *instcounter, char *instructions, unsigned int *forwardtime, DC_motor *mL, DC_motor *mR);
void OptiPink(char *instcounter, char *instructions, unsigned int *forwardtime, DC_motor *mL, DC_motor *mR);

#endif
