/*
 * CrossWalk.c
 *
 *  Library for the crosswalk compoments. This include functions for the buttons,
 *  functions for the crosswalk signs (walking man and stop hand), and counters
 *  used throughout the code
 *
 *  Created on: Mar 25, 2022
 *      Author: aeron
 */

#include "msp.h"
#include "stdio.h"
#include "LEDStoplight.h"
#include "CrossWalk.h"
#include "AeronLCDSetup.h"

void ButtonPin_Int(){
    //P6.5
    P6SEL0 &=~ (BIT0 | BIT4 | BIT5);                                //sets to GPIO
    P6SEL1 &=~ (BIT0 | BIT4 | BIT5);
    P6DIR &=~ (BIT0 | BIT4 | BIT5);                                 //P6.0, P6.4, P6.5 are set to input
    P6REN |= (BIT0 | BIT4 | BIT5);                                  //enables internal resistor
    P6OUT |= (BIT0 | BIT4 | BIT5);                                  //pull up resistor
    P6IES |= (BIT0 | BIT4 | BIT5);                                  //set pin interrupt to trigger from high to low
    P6IE |= (BIT0 | BIT4 | BIT5);                                   //enable interrupt for P6.5

    P6IFG = 0;                                                      //clears all flag

    P1SEL0 &=~ BIT5;                                                //sets to GPIO
    P1SEL1 &=~ BIT5;
    P1DIR &=~ BIT5;                                                 //P1.5 is set to input
    P1REN |= BIT5;                                                  //enables internal resistor
    P1OUT |= BIT5;                                                  //pull up resistor
    P1IES |= BIT5;                                                  //set pin interrupt to trigger from high to low
    P1IE |= BIT5;                                                   //enables interrupt

    P1IFG = 0;                                                      //clears all flag
}

void PORT6_IRQHandler(){

    TIMER_A3->CCTL[1] &=~ CCIFG;                                    //clear the interrupt flag

    if(P6IFG & BIT0){                                               //if button 1 is pressed
        TimerA3Interrupt(periodLoad3s);                             //loads timer to check if button is held
        button1 = 1;                                                //set flag for button1
        P6IFG = 0;                                                  //resets the flag
    }

    if(P6IFG & BIT4){                                               //if button 2 is pressed
        TimerA3Interrupt(periodLoad3s);                             //loads timer to check if button is held
        button2 = 1;                                                //set the flag for button2
        P6IFG = 0;                                                  //resets the flag
    }
    if(P6IFG & BIT5){                                               //if button 3 is pressed
        TimerA3Interrupt(periodLoad3s);                             //loads the timer to check if button is held
        button3 = 1;                                                //set flag for button3
        P6IFG = 0;                                                  //reset the flag
    }

    P6IFG = 0;                                                      //reset all flag
}

void PORT1_IRQHandler(){
    if(P1IFG & BIT5){                                               //if button 4 is pressed
        TimerA3Interrupt(periodLoad3s);                             //loads the timer to check if button is held
        button4 = 1;                                                //set the flag for button4
        P1IFG = 0;                                                  //reset the flag
    }

    P1IFG = 0;                                                      //reset all flag
}

void SecondaryCrossWalk(){

    if(walkCount < 12){                                             //for 12 seconds
    P3OUT &=~ 0x6C;                                                 //turns off all white and amber LEDs
    P3OUT |= WHITELED;                                              //turn the main white LED on
    P3OUT |= AMBERLEDsec;                                           //turn the seconadry amber LED on
    }

    if(walkCount >= 12){                                            //last 3 seconds
        P3OUT &=~ WHITELED;                                         //turn off the white on main road

        if((delayCount % 1) == 0){
        P3OUT ^= AMBERLED;                                          //toggle the main road amber led
        }
    }

}

void MainCrossWalk(){
    if(walkCount < 5){                                              //for 5 seconds
        P3OUT &=~ 0x6C;                                             //turn off all the white and amber LED
        P3OUT |= WHITELEDsec;                                       //turn on white LED on the secondary road
        P3OUT |= AMBERLED;                                          //turn on amber LED on main road
    }
    //P3OUT |= AMBERLED;

    if(walkCount >= 5){                                             //last 3 seconds
        P3OUT &=~ WHITELEDsec;                                      //turn off secondary road white LED

        if((delayCount % 1) == 0){
        P3OUT ^= AMBERLEDsec;                                       //toggle the secondary road amber led
        }
    }

}

void ButtonCrossWalk(){
    P3OUT |= AMBERLED;                                              //turn on main road amber LED

    if(count >= 3 && count < 18){                                   //last 15 seconds
        P3OUT |= WHITELEDsec;                                       //turn on white led
    }

    if(count >= 18){                                                //last 5 seconds
        P3OUT &=~ WHITELEDsec;                                      //turn off secondary road white led

        if((delayCount % 1) == 0){
        P3OUT ^= AMBERLEDsec;                                       //toggle the secondary road amber led
        }
    }
}

void ButtonSecondaryCrossWalk(){
    P3OUT |= AMBERLEDsec;                                           //turn on secondary road amber LED

    if(count == 3){                                                 //after the first 3 seconds
        P3OUT |= WHITELED;                                          //turn on main road white LED
    }

    if(count >= 18){                                                //last 5 seconds
        P3OUT &=~ WHITELED;                                         //turn off main road white LED

        if((delayCount % 1) == 0){
            P3OUT ^= AMBERLED;                                      //toggle main road amber LED
        }
    }
}

void TimerA3Interrupt(int periodLoad){
    //TA3.1
    //P10.5 initialization
    P10SEL0 |= BIT5;
    P10SEL1 &=~ BIT5;
    P10DIR |= BIT5;

    //TA3.1
    TIMER_A3->CCR[1] = periodLoad;

    TIMER_A3->CTL = TASSEL_1    |                                  //ACLK
                    MC_2        |                                  //Continuous mode
                    ID_3;                                          //8 divider

    TIMER_A3->CCTL[1] = CCIE;                                      //enable interrupt
}

void TA3_N_IRQHandler(){

     if((P6IN & BIT0) == 0){                                       //button 1 held down
         buttonHold = 1;                                           //set flag for buttonHold
         TIMER_A3->CCTL[1] &=~ CCIFG;                              //clear the interrupt flag
     }

     if((P6IN & BIT4) == 0){                                       //button 2 held down
         buttonHold = 1;                                           //set flag for buttonHold
         TIMER_A3->CCTL[1] &=~ CCIFG;                              //clear the interrupt flag
     }

     if((P6IN & BIT5) == 0){                                       //button 3 held down
         buttonHold = 1;                                           //set flag for button hold
         TIMER_A3->CCTL[1] &=~ CCIFG;                              //clear the interrupt flag
     }

     if((P1IN & BIT5) == 0){                                       //button 4 held down
         buttonHold = 1;                                           //set flag for button hold
         TIMER_A3->CCTL[1] &=~ CCIFG;                              //clear the interrupt flag
     }

     TIMER_A3->CCTL[1] &=~ CCIFG;                                  //clear the interrupt flag
}

void TIMER32_2_Int(){
    TIMER32_2->CONTROL = 0b11100011;                               //sets timer 1 for enabled,
                                                                   //periodic
                                                                   //with interrupt
                                                                   //No prescaler, 32 bit mode
                                                                   //on shot mode
}

void T32_INT2_IRQHandler(){
    TIMER32_2->INTCLR = 1;                                          //clears the interrupt flag
    count++;                                                        //increment count by 1 every 1 second
    walkCount++;                                                    //increments counter by 1 every 1 second
    secCount++;                                                     //increments secondary count by 1 every 1 second
    delayCount++;                                                   //increments delaycount for toggling amber led by 1 every second
    anotherCount++;                                                 //increments count by 1 every 1 second for ouputting to LCD in BUTTON case
    displayCount--;                                                 //decrements display count by 1 every second
    TIMER32_2->LOAD = 3000000;                                      //set countdown of 1 second on 3MHz
}

