/*
 * LEDStoplight.h
 *
 *  Library for handling initialization for all the LEDs on the system.
 *  This includes the traffic light LEDs, crosswalk LEDs. Also has the
 *  functions to set timer32_1 for the time cycle of the LED sequencing
 *
 *  Created on: Mar 21, 2022
 *      Author: aeron
 */

#ifndef LEDSTOPLIGHT_H_
#define LEDSTOPLIGHT_H_

//main road - Port 4
#define GREENLED BIT2
#define YELLOWLED BIT1
#define REDLED BIT0

//secondary road - Port 4
#define GREENLEDsec BIT4
#define YELLOWLEDsec BIT5
#define REDLEDsec BIT6

//main crosswalk - Port 3
#define WHITELED BIT2
#define AMBERLED BIT6

//secondary crosswalk - Port 3
#define WHITELEDsec BIT5
#define AMBERLEDsec BIT3;

//Functions
void LEDPin_Int();                              //pin initialization for the LEDs
void TIMER32_1_Int();                           //Timer32_1 initialization
void T32_INT1_IRQHandler();                     //Timer32_1 handler initialization

//global variable
int timerDone;                                  //timer32_1 flag

#endif /* LEDSTOPLIGHT_H_ */
