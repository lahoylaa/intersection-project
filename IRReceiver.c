/*
 * IRSensor.c
 *
 *  Library for the IR receiver that handles functions for initializing
 *  the pins for TimerA capture interrupt and flags for 10Hz, 14Hz
 *  for both roads (main/secondary) and flag to get into the EMERGENCY state
 *
 *  Created on: Apr 11, 2022
 *      Author: aeron
 */

#include "msp.h"
#include "IRReceiver.h"
#include "AeronLCDSetup.h"
#include "stdio.h"
#include "math.h"

void IRReceiver_Int(){
    //P2.7 initialization
    P2SEL0 |= BIT7;
    P2SEL1 &=~ BIT7;
    P2DIR &=~ BIT7;

    //TimerA0.4 initialization
    TIMER_A0->CTL |= TASSEL_2 |                     //smclk
                     MC_2     |                     //continous mode
                     TACLR;                         //clear timer

    TIMER_A0->CCTL[4] = CM_1   |                    //rising edge capture
                        CCIS_0 |                    //use aclk
                        CCIE   |                    //enable capture interrupt
                        CAP    |                    //enable capture mode
                        SCS;                        //synchronous capture
}

void IRReceiver2_Int(){
    //P2.4 initialization
    P2SEL0 |= BIT4;
    P2SEL1 &=~ BIT4;
    P2DIR &=~ BIT4;

    //TimerA0.1 initialization
    TIMER_A0->CTL |= TASSEL_2 |                     //smclk
                     MC_2     |                     //continous mode
                     TACLR;                         //clear timer

    TIMER_A0->CCTL[1] = CM_1   |                    //rising edge capture
                        CCIS_0 |                    //use aclk
                        CCIE   |                    //enable capture interrupt
                        CAP    |                    //enable capture mode
                        SCS;                        //synchronous capture
}

void TA0_N_IRQHandler(){
    //TA0.4
    currentEdge = TIMER_A0->CCR[4];                 //set currentedge to rising edge
    period = currentEdge - lastEdge;                //subtracts the lastedge (previous rising edge) and currentedge
    lastEdge = currentEdge;                         //set lastedge to currentedge

    currentEdge2 = TIMER_A0->CCR[1];                //set currentEdge2 to rising edge
    period2 = currentEdge2 - lastEdge2;             //subtracts the lastEdge2 (previous rising edge) and currentEdg2
    lastEdge2 = currentEdge2;                       //set lastEdge2 to currentEdge2

    period = abs(period);                           //take absolute value of the period
    period2 = abs(period2);

    if((5000 < period) && (period < 29000)){        //14Hz detected on main road
        detect14HzMain = 1;
    }

    if((35000 < period) && (period < 38000)){       //10hz detected on main road
        detect10HzMain = 1;
    }

    if((5000 < period2) && (period2 < 29000)){      //14hz detected on secondary road
        detect14HzSec = 1;
    }

    if((35000 < period2) && (period2 < 38000)){     //10Hz detected on secondary road
        detect10HzSec = 1;
    }


    TIMER_A0->CCTL[1] &=~ CCIFG;                    //clear the interrupt flag (main road)
    TIMER_A0->CCTL[4] &=~ CCIFG;                    //clear the interrupt flag (seconadry road)
}

