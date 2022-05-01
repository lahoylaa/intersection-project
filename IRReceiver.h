/*
 * IRSensor.h
 *
 *  Library for the IR receiver that handles functions for initializing
 *  the pins for TimerA capture interrupt and flags for 10Hz, 14Hz
 *  for both roads (main/secondary) and flag to get into the EMERGENCY state
 *
 *  Created on: Apr 11, 2022
 *      Author: aeron
 */

#ifndef IRRECEIVER_H_
#define IRRECEIVER_H_

//functions
void IRReceiver_Int();                      //main road IR receiver pin initialization
void IRReceiver2_Int();                     //secondary road IR receiver pin initialization
void TA0_N_IRQHandler();                    //timerA0 capture irq handler

//global variables
//variables for the main
unsigned volatile currentEdge, lastEdge, period;
int volatile currentStateMain, emergencyFlag, detect10HzMain, detect14HzMain;

//variables for the secondary
unsigned volatile currentEdge2, lastEdge2, period2;
int volatile currentStateSec, detect10HzSec, detect14HzSec;

int volatile mainRoad;                      //flag to check if its the main road or secondary road

#endif /* IRRECEIVER_H_ */
