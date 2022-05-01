/*
 * AeronLCDSetup.h
 *
 *  Libraries for the 16x4 LCD that handles LCD pin initialization, LCD
 *  initialization, Systick initialization, Systick delays for milliseconds
 *  and microseconds, functions for writing to the LCD
 *
 *  Created on: Mar 28, 2022
 *      Author: aeron
 */

#ifndef AERONLCDSETUP_H_
#define AERONLCDSETUP_H_

#include "msp.h"
#include "stdio.h"

//LCD pins using P5.4 - P5.7 for D4 - D7
#define D4 BIT4
#define D5 BIT5
#define D6 BIT6
#define D7 BIT7

//RS and E using P2.5 for RS and P2.6 for E
#define RS BIT5
#define E BIT6

//initialization functions
void Pin_Int();                                         //Pin initialization
void LCD_Int();                                         //LCD initialization
void SysTick_Int();                                     //Systick initialization

//functions systick timer
void Delay_ms(uint16_t microsec);                       //delays in milliseconds
void Delay_us(uint16_t ms);                             //delays in microseconds

//functions LCD
void PulseEnablePin();                                  //pulse E
void pushNibble(uint8_t nibble);                        //push nibble
void pushByte(uint8_t byte);                            //push byte (4 bit)
void write_command(uint8_t command);                    //write the command to LCD
void dataWrite(uint8_t data);                           //write data to LCD
void lcdSetText(char* text, int x, int y);              //sets the text on LCD
void lcdSetInt(int val, int x, int y);                  //converts int then sets the text on LCD
void lcdSetFloat(float val, int x, int y);              //converts float then sets the text on LCD

#endif /* AERONLCDSETUP_H_ */
