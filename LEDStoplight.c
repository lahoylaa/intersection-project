/*
 * LEDStoplight.c
 *
 *  Library for handling initialization for all the LEDs on the system.
 *  This includes the traffic light LEDs, crosswalk LEDs. Also has the
 *  functions to set timer32_1 for the time cycle of the LED sequencing
 *
 *  Created on: Mar 21, 2022
 *      Author: aeron
 */

#include "LEDStoplight.h"
#include "msp.h"
#include "stdio.h"
#include "AeronLCDSetup.h"

void LEDPin_Int()                                       //LED pin initialization
{
    //P4.0 - P4.6
    P4SEL0 &=~ 0xFF;                                    //sets to GPIO
    P4SEL1 &=~ 0xFF;
    P4DIR |= 0xFF;                                      //sets to output
    P4OUT &=~ 0xFF;                                     //sets to low

    //white and amber LED for main and secondary
    P3SEL0 &=~ 0x6C;                                    //set to GPIO
    P3SEL1 &=~ 0x6C;
    P3DIR |= 0x6C;                                      //Output
    P3OUT &=~ 0x6C;                                     //Initally set to LOW
}

void TIMER32_1_Int()
{
    TIMER32_1->CONTROL = 0b11100011;                    //sets timer 1 for enabled,
                                                        //periodic
                                                        //with interrupt
                                                        //No prescaler, 32 bit mode
                                                        //on shot mode
}

void T32_INT1_IRQHandler()
{
    TIMER32_1->INTCLR = 1;                              //clears the interrupt flag
    timerDone = 1;                                      //timer32_1 flag
    TIMER32_1->LOAD = 3000000;                          //set countdown of 1 second on 3MHz
}



