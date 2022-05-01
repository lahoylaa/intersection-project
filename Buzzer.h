/*
 * Buzzer.h
 *
 *  Library for the Piezo Buzzer that handles the functions to control the buzzer
 *  to emit the desired tone and flash that tone at the specified frequencies of
 *  1Hz, 3Hz and 10Hz
 *
 *  Created on: Apr 16, 2022
 *      Author: aeron
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#define RATE10Hz 6250
#define RATE3Hz 20833
#define RATE1Hz 62500

//functions
void Buzzer_Int(int dutyCycles);        //timerA PWM to emit the buzzer tone
void ToneRate(int frequencyRate);       //timerA interrupt to set the flash rate of tone
void TA2_N_IRQHandler();                //interrupt handler

//global variables
int buzzerTone;                         //variable for buzzer tone

#endif /* BUZZER_H_ */
