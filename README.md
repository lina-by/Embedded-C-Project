# Course project - Mine navigation search and rescue (Sion & Lina)

## Special Features

**1. Buggy stops before impact for all colours.**

**2. Path backwards optimized by removing any distances covered by 180 degree turns or reverses (Blue, Yellow, Pink).**

**3. Buggy rams into the walls after stopping for alignment and colour reading.**


## Video
Short Video with comments https://youtu.be/3WlJp_RUGr4

Full Video https://youtu.be/5VW3Jt2XfkQ

## Modifications
<img src="https://user-images.githubusercontent.com/115535186/207207783-3d871ab4-c2ba-4c2a-acd2-36fd71c3077e.png" width=50% height=50%>
<img src="https://user-images.githubusercontent.com/115535186/207207808-8b8c8034-8457-4f8e-a8ee-21cbde608acc.png" width=50% height=50%>

-   The buggy is modified with a shroud that covers the entire clicker board, with a smaller shroud that covers the RGB sensor of the colour clicker board.
-   The bigger shroud allows the colour reading environment to stay constant, blocking out most of the light from the surroundings when the buggy is flush with the card.
-   The smaller shroud prevents the light from the LEDs shining directly to the RGB sensor and allows only the light coming from the front to shine into the RGB sensor.

## Main Procedures
### Navigating to the white card
1.	Before the buggy moves forward, the integration time is set to 24ms and clear light reading is recorded.
2.	The high threshold for the clear ADC interrupt is set slightly above the reading taken and the interrupt is cleared.
3.	The timer registers are set to 0 and the buggy moves forward with all the LEDs on the clicker board turned on.
4.	When the buggy gets close to a wall, the clear reading increases due to the reflected light and the interrupt is triggered.
5.	The buggy records the timer value in an array and stops.
6.	The rams into the wall to re-align itself and obtain accurate colour readings.
7.	Integration time is set to 101ms and the buggy reads the light, calculates the correct instruction and records it in an array.
9.	The buggy moves slightly backwards and executes the instruction (Turn 90 right, Turn 90 left, etc.)
    -   If it is the Blue instruction, the buggy will ram backwards to re-align it after turning 180 and move slightly forward again.
10.	Return to step 1, unless the white or black card is reached.
### Navigating back from the white card
1.	When the buggy reaches a white card, a flag is raised to navigate the buggy back to where it travelled.
2.	Using the recorded timer value, the buggy travels in the opposite direction.
3.	To travel back the distance it travelled, the timer registers are set to the recorded timer value subtracted from the maximum timer register value (2^16).
4.	When the timer overflows, an interrupt is triggered, and a flag is raised.
5.	The buggy executes the reverse of the recorded instruction.
7.	Return to step 2 and execute the next instruction, unless the end of the stored array is reached.

## Interrupts for Collision Detection
-	Clear ADC interrupt in colour clicker board is used for collision detection.
-	To trigger an interrupt in the main board as well, a falling edge triggered interrupt is also set up on pin RB1.
```
    PIE0bits.IOCIE = 1;     // Enable interrupt on change
    IOCBNbits.IOCBN1 = 1;   // Enable negative edge trigger IOC for register B PIN 1
```
-	The persistence value was set to 5, to avoid false detection but to allow a fast enough detection.
-	When the interrupt is triggered, the ISR raises a flag for the main loop to notice.

###Collision detection procedures
-   Everytime before the buggy moves forward, the buggy records the clear light reading in front of it.
-   The clear interrupt high threshold is set slightly above the reading (+15 in our implementation), but it can be calibrated accordingly to the amount of light produced by the reflection.
```
        // Set new threshhold, clear interrupt &  reset flag
        set_high_threshold(color_read_Clear()+15); // Set new threshold
        clear_interrupt(); // Clear Interrupt
        clearADCflag = 0; // Reset Flag
```


## Colour Reading
Colour | Instruction
---------|---------
Red | Turn Right 90
Green | Turn Left 90
Blue | Turn 180
Yellow | Reverse 1 square and turn right 90
Pink | Reverse 1 square and turn left 90
Orange | Turn Right 135
Light blue | Turn Left 135 
White | Finish (return home)
Black | Maze wall colour

The colour reading is done in the following steps.
1.  It asseses the RGB values received when shining white light.
2.  If the values of blue read are significantly higher than that of red and green, the colour must be blue or lightblue.
3.  Depending on the ratio of red to green and red to blue, either light blue or blue is returned.
4.  If the overall light values are too small, black is returned
5.  If the value for green is higher than that of red, green is returned
6.  If the value for red is significantly higher than that of green and blue, further tests are done and either red, pink, orange or yellow will be returned.
    - To check for pink, blue light is shone, the RGB values are updated and if the ratio blue/red is over a certain theshold, pink is returned.
    - To check for yellow and orange, green light is shone, the RGB values are updated and if the ratio green/red is over a certain theshold, yellow is returned.
    - If the ratio green/red is over a certain lower theshold, orange is returned.
    - Otherwise red is retured.
7.  Otherwise white is returned.

## Optimizing the Path Back
-	3 Instructions are optimized for the way back.
    - Yellow, reverse 1 square and turn right
    - Pink, reverse 1 square and turn right
    - Blue, 180 turn
### Blue Card
-	For blue, the distance travelled to the blue card is subtracted from the distance travelled after the blue card.
-	Then, the buggy travels the reseulting distance away from the blue card.
### Pink and Yellow
-	For pink and yellow, the distance travelled before the card is compared to the distance value of 1 square.
-	If the distance is smaller than 1 square, the buggy travels back to the card and carries out the next reverse instruction.
-	If the distance is larger than 1 square, the distance of 1 square is subtracted and the buggy travels back to the direction opposite of the card.
## Timer and Interrupts
-	To record the time travelled by the buggy, timer0 is used along with timer2 for the DC motors.
-	Timer2 is set up identical to lab 6.
-	Timer0 is set up identical to lab 3, with the pre-scaler value of 1:32768 to give the maximum time value of 134 seconds, which is sufficient to record the time travelled from card to card.
-	The interrupt for Timer0 is set up, which will be used for the navigation back function

## Turning and Moving
-	The turning is done in multiples of 45-degree steps.
-	To align with the walls after reading the card, the buggy is also programmed with a ram function to ram the walls to align itself after detecting a collision.
-	The buggy travels forward and backwards with the maximum motor power of 12, so that the buggy does not collide with the wall when travelling forward.
-	It also allows a more accurate calibration for functions that require the buggy to travel a specified distance (Pink, Yellow, etc.)

## Route distance difference management
-   As the distance the buggy detects the wall is smaller than the distance it steps back after ramming into a card, the distance value needs to be adjusted accordingly.
-   And as the distance is different for each card, a function takes in the card color and distance value to subtracts a pre-measured distance from each of the distance values.

```
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
```

## Calibrating the buggy
Some functions of the buggy requires manual calibration.
-   The distance the buggy goes back after reading the card should be calibrated so that the buggy is at the middle of the square.
-   The functions that requires the buggy to go 1 square back also requires similar calibration.
-   As the turns are all in increments of 45, the 45 degree turn should be calibrated.
    -   A tip is to make the buggy turn 8 times to see if it makes a 360 degree turn and return to its original orientation.
-  The distance difference function explained above also needs to be calibrated.
    -   A tip is to measure the actual physical distance difference and converting it into the timer values.
    -   Generally dark colours such as blue and red requires more adjustment, as the buggy stops very close to the card.
- The color recognition function only needs to be calibrated once, as the shroud keeps the lighting environment constant.
- As explained previously, the collision detection feature also needs to be calibrated to account for the amount of reflection the cards produce.
    -   It is a balance between sensitivity and detection distance, as setting the extra value low would make the collision detection more sensitive and allow detection at a long range, but slight change in ambient lighting conditions could trigger it.
