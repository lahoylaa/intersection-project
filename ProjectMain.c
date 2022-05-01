/*
 * Project: Intersection Traffic System
 * EGR 227 Section 101
 * Date: 4/24/2022
 * Instructor: Professor Kandalaft
 * Name: Aeron Lahoylahoy and Jacob Doezema
 *
 * This program is an automatic traffic light system that sequences colored LEDs (traffic lights)
 * using a finite state program. The sequencing of the traffic lights are based on a timed cycle.
 * Emergency vehicles equipped with an IR emitter (14Hz emergency and 10hz transit) overrides the
 * light timing cycle, causing the detected road stay green until the signal is lost. An pedestrain
 * crosswalk button is implemented where the red light on the desired road has an extended time of
 * 23 seconds. Audible assist is included when the crosswalk button is held for 3 seconds on the
 * main road and 2 seconds on the secondary road.
 */

#include "msp.h"
#include "stdio.h"
#include "LEDStoplight.h"
#include "CrossWalk.h"
#include "AeronLCDSetup.h"
#include "Buzzer.h"
#include "IRReceiver.h"
//#include "SegmentDisplay.h"
#include "math.h"

//state name
enum stateName
{
    GREEN, YELLOW, RED, BUTTON, EMERGENCY
};

//intially set the state to the main road GREEN
enum stateName state = GREEN;

//functions
void SegmentPin_Int();                                                          //7-Segment display initialization
void MainDisplayCount();                                                        //7-Segment display countdown for main road
void SecondaryDisplayCount();                                                   //7-Segment display countdown for secondary road
void ButtonMainDisplayCount();                                                  //7-Segment display countdown for main road when button is pressed
void ButtonSecondaryDisplayCount();                                             //7-Segment display countdown for secondary road when button is pressed

/** 7-Segment Display   **/
int number[] = { 0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,    //0 - 4
                 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01100111,    //5 - 9
                 0b10111111, 0b10000110, 0b11011011, 0b11001111, 0b11100110,    //10 - 14
                 0b11101101 };                                                  //15

void main()
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;                                 // stop watchdog timer

    /**            LCD              **/
    Pin_Int();                                                                  //Pin initialization for the LCD
    SysTick_Int();                                                              //SysTick initialization
    LCD_Int();                                                                  //LCD initialization
    lcdSetText("Main Road :", 0, 0);                                            //outputs main road text on LCD
    lcdSetText("Secondary Road:", 0, 2);                                        //outputs the secondary road text on LCD

    /**        LEDStoplight         **/
    LEDPin_Int();                                                               //pin initialization for all the LEDs (Green, yellow, red, amber, white)
    TIMER32_1_Int();                                                            //Timer32_1 initialization

    /**         CrossWalk           **/
    ButtonPin_Int();                                                            //pin initialization for all the buttons
    TIMER32_2_Int();                                                            //Timer32_2 initialization

    //Segment Display
    SegmentPin_Int();                                                           //7-Segment pin initialization

    /** IR Sensor   **/
    IRReceiver_Int();                                                           //pin initialization for the Main road IR receiver
    IRReceiver2_Int();                                                          //pin initialization for the Secondary road IR receiver

    //on board led for testing
    P1SEL0 &= ~ BIT0;                                                           //set GPIO
    P1SEL1 &= ~ BIT0;
    P1DIR |= BIT0;                                                              //Output
    P1OUT &= ~ BIT0;                                                            //initially set to LOW

    //interrupts priority
    NVIC->IP[9] |= 0x00;                                                        //highest priority IR receivers
    NVIC->IP[15] |= 0x11;                                                       //second priority button hold
    NVIC->IP[40] |= 0x22;                                                       //second priority button press
    NVIC->IP[35] |= 0x22;                                                       //second priority button press
    NVIC->IP[13] |= 0x22;                                                       //second priority Buzzer interrupt
    NVIC->IP[26] |= 0x33;                                                       //third priority Timer32_2
    NVIC->IP[25] |= 0x55;                                                       //least priority Timer32_1

    //enabling interrupts
    NVIC_EnableIRQ(T32_INT1_IRQn);                                              //enable timer32_1 interrupt
    NVIC_EnableIRQ(T32_INT2_IRQn);                                              //enable timer32_2 interrupt
    NVIC_EnableIRQ(TA2_N_IRQn);                                                 //buzzer interrupt
    NVIC_EnableIRQ(TA0_N_IRQn);                                                 //IR interrupt
    NVIC_EnableIRQ(PORT1_IRQn);                                                 //button interrupt
    NVIC_EnableIRQ(PORT6_IRQn);                                                 //button interrupt
    NVIC_EnableIRQ(TA3_N_IRQn);                                                 //button hold interrupt

    __enable_interrupt();                                                       //global interrupt enabled
    TIMER32_2->LOAD = 3000000;                                                  //set load to timer32_2 to 1 seconds

    buttonState = 10;                                                           //buttonState initially at 10 (can't use 0 because GREEN == 0)

    buzzerTone = 500;                                                           //750Hz tone at 8 divide on SMCLK
    Buzzer_Int(0);                                                              //buzzer is turn off initially

    while (1)
    {
        switch (state)
        {
        case GREEN:                                                             //Green LED state

            printf("\n Green State \n");
            P4OUT &= ~0xFF;                                                     //set all pins to low
            P4OUT |= GREENLED;                                                  //turn Green LED on MAIN road
            P4OUT |= REDLEDsec;                                                 //turn RED LED on secondary road
            displayCount = 15;                                                  //sets the displayCount for the secondary 7-Segment display

            TIMER32_1->LOAD = 3000000 * 12;                                     //set load to 12s
            lcdSetText("Green     ", 0, 1);                                     //output to LCD the state of main road
            lcdSetText("Red       ", 0, 3);

            timerDone = 0;                                                      //reset timer flag

            while (!timerDone)                                                  //wait 12 seconds
            {
                //printf("\n period %d\n", period2);
                lcdSetInt(count, 10, 1);                                        //shows the main road time
                lcdSetInt(secCount, 10, 3);                                     //shows the secondary road time

                if (button4 || button1 || button2 || button3)                   //checks if the button has been pressed
                {
                    buttonState = GREEN;                                        //sets the buttonState to the current state
                }

                //checks to see if the IR sensors detects anything main or secondary road
                if (detect14HzMain || detect10HzMain || detect14HzSec || detect10HzSec)
                {
                    emergencyFlag = 1;                                          //set the emergency flag to 1
                    break;                                                      //break out the while loop
                }
                SecondaryDisplayCount();                                        //displays the secondary road 7-Segment Display
                SecondaryCrossWalk();                                           //turns on the white and amber led on the secondary road
            }

            if (emergencyFlag)                                                  //if emergency has been detected
            {
                currentStateMain = GREEN;                                       //set the main current state
                currentStateSec = RED;                                          //sets the secondary current state
                state = EMERGENCY;                                              //goes to the EMERGENCY state
            }
            else
            {
                TIMER_A3->CCTL[1] &=~ CCIFG;                                    //clear the interrupt flag
                count = 0;                                                      //reset the main road count
                timerDone = 0;                                                  //reset timer flag
                state = YELLOW;                                                 //change state to YELLOW
            }
            break;

        case YELLOW:                                                            //Yellow LED state

            printf("\n Yellow State\n");
            TIMER32_1->LOAD = 3000000 * 3;                                      //set load to 3 seconds
            P4OUT &= ~0xFF;                                                     //set all pins to low
            P4OUT |= YELLOWLED;                                                 //Yellow on MAIN road
            P4OUT |= REDLEDsec;                                                 //Red on Secondary road

            lcdSetText("Yellow    ", 0, 1);                                     //output state to LCD
            lcdSetText("Red       ", 0, 3);

            timerDone = 0;                                                      //reset the timerDone flag

            while (!timerDone)                                                  //wait 3 seconds
            {
                lcdSetInt(count, 10, 1);                                        //shows the main road count
                lcdSetInt(secCount, 10, 3);                                     //shows the secondary road count

                if (button4 || button1 || button2 || button3)                   //checks if the button has been pressed
                {
                    buttonState = YELLOW;                                       //set the state of button to the current state of the road
                }

                //checks if the IR sensors detect an frequency
                if (detect14HzMain || detect10HzMain || detect14HzSec || detect10HzSec)
                {
                    printf("\n Detected the IR \n");
                    emergencyFlag = 1;                                          //set emergency flag to 1
                    break;                                                      //break out of the while loop
                }

                SecondaryDisplayCount();                                        //displays the secondary road 7-Segment display
                SecondaryCrossWalk();                                           //turns on the white and amber led on the secondary road


            }

            if (emergencyFlag)                                                  //if the emergency flag is set
            {
                currentStateMain = YELLOW;                                      //set the current main road state
                currentStateSec = RED;                                          //set the current state of the secondary road
                state = EMERGENCY;                                              //goes to the EMERGENCY state
            }
            else
            {
                TIMER_A3->CCTL[1] &=~ CCIFG;                                    //clear the interrupt flag
                P3OUT &= ~0x6C;                                                 //turn all the white and amber led
                P7OUT &= ~0xFF;                                                 //turns off the secondary road 7-Segment
                delayCount = 0;                                                 //resets the delaCount for the 7-Segment display
                count = 0;                                                      //reset the main road count
                walkCount = 0;                                                  //resets the count for the amber led to toggle and white led
                secCount = 0;                                                   //resets the counter for the secondary road lcd
                timerDone = 0;                                                  //reset timer flag
                state = RED;                                                    //change state to RED
            }
            break;

        case RED:                                                               //Red LED state

            P4OUT &= ~0xFF;                                                     //set all pins to low
            //Delay_ms(250);
            P4OUT |= REDLED;                                                    //turn Red LED on MAIN road
            P4OUT |= GREENLEDsec;                                               //turn Green LED on secondary road
            displayCount = 8;                                                   //sets the display count to 9 for the main road 7-Segment display

            lcdSetText("Red       ", 0, 1);                                     //outputs the state of the main road to LCD
            lcdSetText("Green     ", 0, 3);                                     //outputs the state of the secondary road to the LCD

            if (buttonState == GREEN || buttonState == YELLOW)                  //checks which state the button has been pressed in
            {
                TIMER32_1->LOAD = 3000000 * delayCrossWalk;                     //loads the timer32 to 23 total seconds

                timerDone = 0;                                                  //resets the timerDone flag

                while (!timerDone)                                              //delays the state for 23 seconds
                {
                    lcdSetInt(count, 10, 1);                                    //shows the main road count
                    lcdSetInt(secCount, 10, 3);                                 //shows the secondary road count

                    if (count == 20)                                            //if the count is 20 the last 3 seconds the light turns yellow
                    {
                        P4OUT &= ~ GREENLEDsec;                                 //secondary green turns off
                        P4OUT |= YELLOWLEDsec;                                  //secondary yellow turns on
                        secCount = 0;                                           //the secCount resets for yellow
                        lcdSetText("Yellow    ", 0, 3);                         //outputs the state to LCD
                    }

                    /** BUZZER  **/
                    if (buttonHold)                                             //checks if the button is held
                    {
                        if (count == 0)
                        {
                            ToneRate(6250);                                     //sets the tone rate to 10Hz
                        }

                        if (count == 3)
                        {
                            ToneRate(62500);                                    //set the tone rate to 1Hz
                        }

                        if (count == 18)
                        {
                            ToneRate(20833);                                    //set the tone rate to 3Hz
                        }

                    }

                    //checks if the IR sensor detects the frequency
                    if (detect14HzMain || detect10HzMain || detect14HzSec || detect10HzSec)
                    {
                        emergencyFlag = 1;                                      //sets the emergency flag to 1
                        break;                                                  //break out of the while loop
                    }
                    ButtonCrossWalk();                                          //changes the amber and white led according to button press
                    ButtonMainDisplayCount();                                   //displays the countdown 15 seconds on 7-Segment display


                }
            }
            else                                                                //crosswalk button has not been pressed
            {
                TIMER32_1->LOAD = 3000000 * 8;                                  //set load to 8 seconds

                timerDone = 0;                                                  //timerDone reset to 0
                count = 0;

                while (!timerDone)                                              //delay for 8 seconds
                {
                    lcdSetInt(count, 10, 1);                                    //show the main road count
                    lcdSetInt(secCount, 10, 3);                                 //show the secondary road count

                    if (count == 5)                                             //changes secondary road to yellow
                    {
                        P4OUT &=~ GREENLEDsec;                                  //turns off the green led on the secondary road
                        P4OUT |= YELLOWLEDsec;                                  //turns on the yellow led on the secondary road
                        secCount = 0;                                           //resets the secCount to 0 for yellow
                        lcdSetText("Yellow    ", 0, 3);                             //displays the state to the LCD
                    }

                    if (button4 || button1 || button2 || button3)               //checks if the button has been pressed
                    {
                        TimerA3Interrupt(periodLoad2s);                         //loads the button timer interrupt to check if the button is held down for 2 seconds
                        buttonState = RED;                                      //sets the button state to the road state
                    }

                    //checks if the IR sensor detects any frequency
                    if (detect14HzMain || detect10HzMain || detect14HzSec || detect10HzSec)
                    {
                        emergencyFlag = 1;                                      //emergency flag set to 1
                        break;                                                  //break out of the while loop
                    }
                    MainCrossWalk();                                            //changes the amber and whited led on main crosswalk
                    MainDisplayCount();                                         //displays the countdown for the main road 7-Segment display
                }
            }

            if (emergencyFlag)                                                  //checks if the emergency flag was set
            {
                currentStateMain = RED;                                         //set the current main road state
                currentStateSec = GREEN;                                        //set the current secondary road state
                state = EMERGENCY;                                              //go to EMERGENCY state
            }
            else if (buttonState == RED)                                        //checks if the button has been pressed
            {
                P3OUT &=~ 0x6C;                                                 //turns off all of the white and amber LEDs
                P7OUT &=~ 0xFF;                                                 //turns off all of the 7-Segment display for the main/secondary road
                P9OUT &=~ 0xFF;
                /**Resets all counters and flag**/
                button1 = 0;                                                    //resets all the button presses
                button2 = 0;
                button3 = 0;
                button4 = 0;
                Buzzer_Int(0);                                                  //clears the buzzer turns off the tone
                delayCount = 0;                                                 //resets the counter for displaying 7-Segment displays
                buttonPressed = 0;                                              //button flag reset
                count = 0;                                                      //reset count
                walkCount = 0;                                                  //resets the counter for amber led toggle and white led
                secCount = 0;                                                   //reset the secondary road count
                timerDone = 0;                                                  //timer32_1 flag reset
                state = BUTTON;                                                 //go to BUTTON case
            }
            else
            {
                /**Resets all counters and flag**/
                button1 = 0;                                                    //resets all the button presses
                button2 = 0;
                button3 = 0;
                button4 = 0;
                buttonHold = 0;                                                 //resets the buttonHold flag
                buttonState = 10;                                               //reset the buttonState flag
                TIMER_A3->CCTL[1] &=~ CCIFG;                                    //clear the interrupt flag for the button hold
                Buzzer_Int(0);                                                  //clears the buzzer turns off the tone
                P3OUT &= ~0x6C;                                                 //turn off the white and amber led
                P9OUT &= ~0xFF;                                                 //turn off the main road 7-Segment displays
                delayCount = 0;                                                 //resets the counter for displaying 7-Segment displays
                buttonPressed = 0;                                              //button flag reset
                count = 0;                                                      //reset count
                walkCount = 0;                                                  //resets the counter for amber led toggle and white led
                secCount = 0;                                                   //reset the secondary road count
                timerDone = 0;                                                  //timer32_1 flag reset
                state = GREEN;                                                  //change state to GREEN
            }
            break;

        case BUTTON:                                                            //BUTTON state for when the button is pressed when MAIN road is RED

            P4OUT &= ~0xFF;                                                     //turns off all the LEDs
            P4OUT |= REDLEDsec;                                                 //turns the secondary road RED led on
            P4OUT |= GREENLED;                                                  //turns on the main road GREEN led on

            lcdSetText("Green     ", 0, 1);                                     //outputs the state to LCD
            lcdSetText("Red       ", 0, 3);

            P3OUT &=~ 0x6C;                                                     //turns off all the white and amber LED on both roads (main/secondary)
            P7OUT &=~ 0xFF;                                                     //turn off all the 7-Segment displays on both roads
            P9OUT &=~ 0xFF;

            TIMER32_1->LOAD = 3000000 * delayCrossWalk;                         //loads the timer with 23 seconds
            timerDone = 0;                                                      //timerDone reset to 0

            anotherCount = 0;
            while (!timerDone)                                                  //delays for 23 seconds
            {
                lcdSetInt(anotherCount, 10, 1);                                 //shows the main road count
                lcdSetInt(secCount, 10, 3);                                     //shows the secondary road count

                if(count == 20){                                                //changes to yellow for the last 3 seconds
                    P4OUT &=~ GREENLED;                                         //turns off the green led
                    P4OUT |= YELLOWLED;                                         //turns on the yellow led
                    anotherCount = 0;                                           //reset the secCount
                    lcdSetText("Yellow    ", 0, 1);                             //displays the state to the LCD
                }

                /** Buzzer  **/
                if (buttonHold)                                                 //if the button is held down
                {
                    if (count == 0)
                    {
                        ToneRate(6250);                                         //set the rate of the buzzer to 10Hz
                    }

                    if (count == 3)
                    {
                        ToneRate(62500);                                        //set the rate of the buzzer to 1Hz
                    }

                    if (count == 18)
                    {
                        ToneRate(20833);                                        //set the rate of the buzzer to 3Hz
                    }
                }

                //checks if the IR sensor detects a frequency
                if (detect14HzMain || detect10HzMain || detect14HzSec || detect10HzSec)
                {
                    emergencyFlag = 1;                                          //set the emergency flag to 1
                    break;                                                      //break out of the while loop
                }

                ButtonSecondaryCrossWalk();                                     //changes the white and amber led base of button press on the secondary road
                ButtonSecondaryDisplayCount();                                  //displays the countdown on the secondary road

            }

            if (emergencyFlag)                                                  //if the emergency flag is 1
            {
                currentStateMain = GREEN;                                       //set the current main road state
                currentStateSec = RED;                                          //set the current secondary road state
                state = EMERGENCY;                                              //go to EMERGENCY case
            }
            else
            {
                P3OUT &=~ 0x6C;                                                 //turns off all the white and amber LEDs on both roads
                P7OUT &=~ 0xFF;                                                 //turns off all the 7-Segment display on both roads
                P9OUT &=~ 0xFF;
                /**Resets all counters and flag**/
                button1 = 0;                                                    //resets all the button presses
                button2 = 0;
                button3 = 0;
                button4 = 0;
                anotherCount = 0;                                               //reset anotherCount to 0
                buttonState = 10;                                               //reset buttonState
                TimerA3Interrupt(0);                                            //clears the timer interrupt to check if the button was held
                Buzzer_Int(0);                                                  //clears the buzzer turns off the tone
                P3OUT &= ~0x6C;                                                 //turn off the white and amber led
                P9OUT &= ~0xFF;                                                 //turn off the main road 7-Segment displays
                delayCount = 0;                                                 //resets the counter for displaying 7-Segment displays
                buttonPressed = 0;                                              //button flag reset
                count = 0;                                                      //reset count
                walkCount = 0;                                                  //resets the counter for amber led toggle and white led
                secCount = 0;                                                   //reset the secondary road count
                timerDone = 0;                                                  //timer32_1 flag reset
                state = RED;                                                    //change state to RED
            }
            break;


        case EMERGENCY:

            P7OUT &= ~0xFF;                                                     //turn off the display segments
            P9OUT &= ~0xFF;
            P3OUT &= ~ WHITELED;                                                //turn off the white led on the main road
            P3OUT &= ~ WHITELEDsec;                                             //turn off the whited led on the secondary road
            P3OUT |= AMBERLED;                                                  //turn on the amber led on the main road
            P3OUT |= AMBERLEDsec;                                               //turn on the amber led on the secondary road


            //emergency
            if(detect14HzMain){
                lcdSetText("!EMERGENCY!", 0, 1);                                //outputs "EMERGENCY" on the main road
            }

            if(detect14HzSec){
                lcdSetText("!EMERGENCY!", 0, 3);                                //outputs "EMERGENCY" on the secondary road
            }

            //transit
            if(detect10HzMain){
                lcdSetText("!TRANSIT!  ", 0, 1);                                //outputs "TRANSIT" on the main road
            }

            if(detect10HzSec){
                lcdSetText("!TRANSIT!  ", 0, 3);                                //outputs "TRANSIT" on the secondary road
            }

            //ir receiving from main
            if (detect14HzMain || detect10HzMain)                               //IR receiving from main road
            {
                mainRoad = 1;                                                   //set mainRoad to 1

                if (currentStateMain == GREEN || currentStateMain == YELLOW)
                {
                    P4OUT &= ~0xFF;                                              //turn off all leds
                    P4OUT |= GREENLED;                                           //turn on green led
                    P4OUT |= REDLEDsec;                                          //turn on red led

                    while (emergencyFlag)                                        //keep this state while the emergency flag is 0
                    {

                        if ((detect14HzMain == 0) || (detect10HzMain == 0))      //if the frequency is not detected
                        {
                            emergencyFlag = 0;                                   //set the emergency flag to 0
                        }

                        Delay_ms(500);                                           //delay for 0.5s
                        detect14HzMain = 0;                                      //set detect14Hz on main flag to 0
                        detect10HzMain = 0;                                      //set detect10Hz on main flag to 0
                    }

                }

                if (currentStateMain == RED)
                {
                    P4OUT &= ~0xFF;                                              //turns off all leds
                    P4OUT |= REDLED;                                             //turn on the red led
                    P4OUT |= YELLOWLEDsec;                                       //turn on the yellow led

                    TIMER32_1->LOAD = 3000000 * 3;                               //load the timer with 3 seconds
                    timerDone = 0;                                               //reset the timer flag
                    while (!timerDone)                                           //delay for 3 seconds
                    {
                        ;
                    }

                    P4OUT &= ~ YELLOWLEDsec;                                     //turn off the yellow led on the secondary road
                    P4OUT &= ~ REDLED;                                           //turn off the red led on the main
                    P4OUT |= REDLEDsec;                                          //turn on the red led on the secondary road
                    P4OUT |= GREENLED;                                           //turn on the green led on the main road

                    while (emergencyFlag)                                        //keep the green on while the emergency flag is 1
                    {
                        if ((detect14HzMain == 0) || (detect10HzMain == 0))      //if the frequency is not detected
                        {
                            emergencyFlag = 0;                                   //set emergency flag to 0
                        }

                        Delay_ms(500);                                           //delay for 0.5secs
                        detect14HzMain = 0;                                      //reset detect14Hz on main road
                        detect10HzMain = 0;                                      //reset detect10Hz on main road
                    }

                }
            }

            //ir receiving from secondary
            if (detect14HzSec || detect10HzSec)                                  //if a frequency is read in the secondary road
            {

                if (currentStateSec == GREEN || currentStateSec == YELLOW)
                {
                    P4OUT &= ~0xFF;                                              //turn off all leds
                    P4OUT |= REDLED;                                             //turn red led
                    P4OUT |= GREENLEDsec;                                        //turn green led

                    while (emergencyFlag)                                        //keeps the green led while emergency is 1
                    {

                        if ((detect14HzSec == 0) || (detect10HzSec == 0))        //if any frequency is not detected on the secondary road
                        {
                            emergencyFlag = 0;                                   //emergency flag is set to 0
                        }

                        Delay_ms(500);                                           //delay for 0.5 seconds
                        detect14HzSec = 0;                                       //resets detect14Hz to 0
                        detect10HzSec = 0;                                       //resets detect10Hz to 0
                    }
                }

                if (currentStateSec == RED)
                {
                    P4OUT &= ~0xFF;                                              //turns off all leds
                    P4OUT |= REDLEDsec;                                          //turns on red led on the secondary road
                    P4OUT |= YELLOWLED;                                          //turns on yellow led on the main road

                    TIMER32_1->LOAD = 3000000 * 3;                               //loads the timer with 3 seconds
                    timerDone = 0;                                               //reset the timer flag to 0

                    while (!timerDone)                                           //delay for 3 seconds
                    {
                        ;
                    }

                    P4OUT &= ~ YELLOWLED;                                        //turn off the yellow led on the main
                    P4OUT &= ~ REDLEDsec;                                        //turn off the red led on the secondary
                    P4OUT |= REDLED;                                             //turn on the red led on the main
                    P4OUT |= GREENLEDsec;                                        //turn on the green led on the secondary road

                    while (emergencyFlag)                                        //keep in green while the emergency flag is 1
                    {
                        if ((detect14HzSec == 0) || (detect10HzSec == 0))        //if frequency is not detected
                        {
                            emergencyFlag = 0;                                   //set emergency flag to 0
                        }

                        Delay_ms(500);                                           //delay for 0.5seconds
                        detect14HzSec = 0;                                       //reset detect14Hz on the main
                        detect10HzSec = 0;                                       //reset detect10Hz on the main
                    }
                }
            }

            if(mainRoad){
                P3OUT &= ~0x6C;                                                 //turn off the white and amber led
                P7OUT &= ~0xFF;                                                 //turn off the main road 7-Segment displays
                P9OUT &= ~0xFF;                                                 //turn off the main road 7-Segment displays
                delayCount = 0;                                                 //resets the counter for displaying 7-Segment displays
                buttonPressed = 0;                                              //button flag reset
                count = 0;                                                      //reset count
                walkCount = 0;                                                  //resets the counter for amber led toggle and white led
                secCount = 0;                                                   //reset the secondary road count
                timerDone = 0;                                                  //timer32_1 flag reset
                emergencyFlag = 0;                                              //reset emergency flag
                detect10HzMain = 0;                                             //reset detect10Hz on main
                detect14HzMain = 0;                                             //reset detect14Hz on main
                detect10HzSec = 0;                                              //reset detect10Hz on secondary
                detect14HzSec = 0;                                              //reset detect14Hz on secondary
                mainRoad = 0;                                                   //reset the mainRoad flag
                state = GREEN;
            }
            else {
                /**Resets all counters and flag**/
            P3OUT &= ~0x6C;                                                      //turn off the white and amber led
            P7OUT &= ~0xFF;                                                      //turn off the main road 7-Segment displays
            P9OUT &= ~0xFF;                                                      //turn off the main road 7-Segment displays
            delayCount = 0;                                                      //resets the counter for displaying 7-Segment displays
            buttonPressed = 0;                                                   //button flag reset
            count = 0;                                                           //reset count
            walkCount = 0;                                                       //resets the counter for amber led toggle and white led
            secCount = 0;                                                        //reset the secondary road count
            timerDone = 0;                                                       //timer32_1 flag reset
            emergencyFlag = 0;                                                   //reset emergency flag
            detect10HzMain = 0;                                                  //reset detect10Hz on main
            detect14HzMain = 0;                                                  //reset detect14Hz on main
            detect10HzSec = 0;                                                   //reset detect10Hz on secondary
            detect14HzSec = 0;                                                   //reset detect14Hz on secondary
            state = RED;
            }
            break;
        }
    }
}

void SegmentPin_Int(){
   //main road segment display
    P7SEL0 &=~ 0xFF;                                                             //set to GPIO
    P7SEL1 &=~ 0xFF;
    P7DIR |= 0xFF;                                                               //output
    P7OUT &=~ 0xFF;                                                              //initially set to low

    //secondary road segment display
    P9SEL0 &=~ 0xFF;                                                             //set to GPIO
    P9SEL1 &=~ 0xFF;
    P9DIR |= 0xFF;                                                               //output
    P9OUT &=~ 0xFF;                                                              //initially set to low
}

void MainDisplayCount()
{
    //countdown from 8 seconds

    P9OUT = number[displayCount];
}

void SecondaryDisplayCount()
{
    //countdown from 15 seconds

    if (secCount == 12)
    {
        displayCount = 3;
    }

    P7OUT = number[displayCount];
}

void ButtonMainDisplayCount()
{
    //countdowns from 15seconds
    if (count == 8)
    {
        displayCount = 15;
    }

    if (count >= 8)
    {
        P9OUT = number[displayCount];
    }
}

void ButtonSecondaryDisplayCount(){
    //countdown from 15 seconds

    if(count == 8){
        displayCount = 15;
    }

    if(count >= 8){
        P7OUT = number[displayCount];
    }
}



