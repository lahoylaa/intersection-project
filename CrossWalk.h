/*
 * CrossWalk.h
 *
 *  Library for the crosswalk compoments. This include functions for the buttons,
 *  functions for the crosswalk signs (walking man and stop hand), and counters
 *  used throughout the code
 *
 *  Created on: Mar 25, 2022
 *      Author: aeron
 */

#ifndef CROSSWALK_H_
#define CROSSWALK_H_

#define delayCrossWalk 23                   //crosswalk has total delay of 23 seconds
#define periodLoad3s 12000                  //load for 3 seconds for button check
#define periodLoad2s 8000                   //load for 2 seconds for button check

//functions
void ButtonPin_Int();                       //pin initilization for the buttons used P1.5, P6.0, P6.4, P6.5
void PORT6_IRQHandler();                    //port 6 irq handler
void PORT1_IRQHandler();                    //port 1 irq handler

void MainCrossWalk();                       //turns on the white and amber led for the main road
void SecondaryCrossWalk();                  //turns on the white and amber led for the secondary road
void ButtonCrossWalk();                     //turns on the white and amber led for the main road when the button is pressed
void ButtonSecondaryCrossWalk();            //turns on the white and amber led for the secondary road when the button is pressed

void TimerA3Interrupt(int periodLoad);      //timerA interrupt to check if the button is held
void TA3_N_IRQHandler();                    //TimerA interrupt for P10.5

void TIMER32_2_Int();                       //Timer32_2 initialization
void T32_INT2_IRQHandler();                 //Timer32_2 handler initialization


//global variable
int buttonPressed;                          //flag for port interrupt (button is pressed)
int buttonHold;                             //flag for the button hold
int buttonState;                            //flag for the current state of the light when the button is pressed
int count;                                  //counts the time (1 count = 1 seconds)
int secCount;                               //counts the time for secondary road (1 count = 1 seconds)
int displayCount;                           //counter for 7-Segment display
int walkCount;                              //counter for crosswalk
int anotherCount;                           //counts the time for the secondary road in the BUTTON state
int delayCount;                             //count to toggle the amber led

//button flags
int button1, button2, button3, button4;

#endif /* CROSSWALK_H_ */
