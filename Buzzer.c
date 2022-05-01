/*
 * Buzzer.c
 *
 *  Library for the Piezo Buzzer that handles the functions to control the buzzer
 *  to emit the desired tone and flash that tone at the specified frequencies of
 *  1Hz, 3Hz and 10Hz
 *
 *  Created on: Apr 16, 2022
 *      Author: aeron
 */

#include "msp.h"
#include "Buzzer.h"

void Buzzer_Int(int dutyCycles){
    //P6.6 initialization
    P6SEL0 |= BIT6;
    P6SEL1 &=~ BIT6;
    P6DIR |= BIT6;

    //TimerA2.3 initialization
    TIMER_A2->CTL = TASSEL_2 |                      //smclk
                    MC_1     |                      //up mode
                    TACLR    |                      //clear timer
                    ID_3;                           //divide by 8

    TIMER_A2->CCR[0] = dutyCycles;
    TIMER_A2->CCR[3] = dutyCycles / 2;              //50% duty cycle

    TIMER_A2->CCTL[3] = TIMER_A_CCTLN_OUTMOD_7;
}

void ToneRate(int frequencyRate){
    //P6.7 initialization
    P6SEL0 |= BIT7;
    P6SEL1 &=~ BIT7;
    P6DIR |= BIT7;

    //TA2.4
    TIMER_A2->CCR[0] = frequencyRate;

    TIMER_A2->CTL = TASSEL_2    |                   //SMCLK
                    MC_2        |                   //continous mode
                    ID_3;                           //8 divider

    TIMER_A2->EX0 = 5;                              //6 divider
                                                    //48 total dividers

    TIMER_A2->CCTL[4] = CCIE;                       //enable interrupt
}

void TA2_N_IRQHandler(){

    if(buzzerTone == 500){                          //if the buzzer is on turn it off
        buzzerTone = 0;
        Buzzer_Int(buzzerTone);
    }
    else if(buzzerTone == 0){                       //if the buzzer is off turn it on
        buzzerTone = 500;
        Buzzer_Int(buzzerTone);
    }

    TIMER_A2->CCTL[4] &=~ CCIFG;                    //clear the interrupt flag
}

