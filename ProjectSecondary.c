/*
 * Project: Temperature and IR Emitter
 * EGR 227 Section 101
 * Date: 4/24/2022
 * Instructor: Professor Kandalaft
 * Name: Aeron Lahoylahoy and Jacob Doezema
 *
 * This program uses the TMP36 sensor to detect the current temperature of the room
 * and displays the reading to the LCD. The code also emits a 14Hz and 10Hz signal
 * using PWM.
 *
 */

#include "msp.h"
#include "AeronLCDSetup.h"

//Functions
void ADCPin_Int();                                 //ADC pin initialization
void ADC14_Int();                                  //ADC initialization
void ADC_Reading();                                //ADC reading
void TempC_Conversion();                           //temp in Celsius conversion
void IREmitter_Int();                              //14Hz IR emitter initialization
void IREmitter_Int10Hz();                          //10Hz IR emitter initialization

//global variables
static volatile uint16_t result;                    //ADC value
float nADC;                                         //ADC value in voltage
float tempC;                                        //ADC value in Celsius

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	/** ADC **/
	ADCPin_Int();
	ADC14_Int();

	/** LCD **/
    SysTick_Int();                                  //systick initialization
    Pin_Int();
    LCD_Int();                                      //lcd initialization
    lcdSetText("Current Temp. is ", 0, 1);          //LCD title

    /** IR **/
    IREmitter_Int();                                //14Hz initialization
    IREmitter_Int10Hz();                            //10Hz initialization

	while(1){
	            ADC_Reading();                      //reads the ADC
	            TempC_Conversion();                 //converts ADC value to Celsius
	            lcdSetFloat(tempC, 6, 2);             //displays to LCD
	            Delay_ms(2000);                     //delay for 2 seconds
	}
}

void ADCPin_Int(){
    //P5.5 which is A0
    P5SEL0 |= BIT5;
    P5SEL1 |= BIT5;
}

void ADC14_Int(){
    ADC14->CTL0 &=~ 0x00000002;                     //disables ADC converter
    ADC14->CTL0 |= 0x04200210;                      //S/H pulse mode, SMCLK, 16 samples clocks
    ADC14->CTL1 = 0x00000030;                       //14 bit resolution
    ADC14->CTL1 |= 0x00000000;                      //convert for mem0;
    ADC14->MCTL[0] = 0x00000000;                    //ADC14INCHx = 0 for mem[0]
    ADC14->CTL0 |= 0x00000002;                      //enables ADC converter
}

void ADC_Reading(){
    ADC14->CTL0 |= ADC14_CTL0_SC;                   //start conversation
    while(!ADC14->IFGR0 & BIT0);                    //wait for conversation to complete
    result = ADC14->MEM[0];                         //get the value from the ADC
    nADC = (result * 3.3) / 16384;
    printf("Value is: \n\t%d\n\t%f\n", result, nADC);   //prints out values on serial monitor
}
void TempC_Conversion(){
    float mV = nADC * 1000;
    tempC = (mV - 500) / 10;
}

void IREmitter_Int(){

    //2.4 0.1
    //P6.6 initialization
    P2SEL0 |= BIT4;
    P2SEL1 &=~ BIT4;
    P2DIR |= BIT4;

    TIMER_A0->CTL = TASSEL_2 |                      //smclk
                    MC_1     |                      //up mode
                    TACLR    |                      //clear timer
                    ID_3;                           //divide by 8

    TIMER_A0->CCR[0] = 26000;                       //14hz
    TIMER_A0->CCR[1] = 26000 / 2;                   //50% duty cycle



    TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;
}

void IREmitter_Int10Hz(){
    //P5.7 initialization
    P5SEL0 |= BIT7;
    P5SEL1 &=~ BIT7;
    P5DIR |= BIT7;

    //TimerA2.2 initialization
    TIMER_A2->CTL = TASSEL_2 |                      //smclk
                    MC_1     |                      //up mode
                    TACLR    |                      //clear timer
                    ID_3;                           //divide by 8

    TIMER_A2->CCR[0] = 37500;                       //10hz
    TIMER_A2->CCR[2] = 37500 / 2;                   //50% duty cycle

    TIMER_A2->CCTL[2] = TIMER_A_CCTLN_OUTMOD_7;
}

