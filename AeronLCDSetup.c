/*
 * AeronLCDSetup.c
 *
 *  Libraries for the 16x4 LCD that handles LCD pin initialization, LCD
 *  initialization, Systick initialization, Systick delays for milliseconds
 *  and microseconds, functions for writing to the LCD
 *
 *  Created on: Mar 28, 2022
 *      Author: aeron
 */

#include "AeronLCDSetup.h"
#include "stdio.h"
#include "msp.h"

void Pin_Int(){
    //P5.4 - P5.7 for LCD D4 - D7
    P5SEL0 &=~ (D4 | D5 | D6 | D7);                   //set to GPIO
    P5SEL1 &=~ (D4 | D5 | D6 | D7);
    P5DIR |= (D4 | D5 | D6 | D7);                     //Output
    P5OUT &=~ (D4 | D5 | D6 | D7);                    //initially low

    //P2.5 and P2.6 for RS and E
    P2SEL0 &=~ (RS | E);                              //set to GPIO
    P2SEL1 &=~ (RS | E);
    P2DIR |= (RS | E);                                //Output
    P2OUT &=~ (RS | E);                               //initially low
}
void LCD_Int(){
    P2OUT &=~ RS;                                     //set RS to low

    //reset sequence
    write_command(0x03);
    Delay_ms(10);
    write_command(0x03);
    Delay_us(100);
    write_command(0x03);
    Delay_ms(10);

    //Setting the 4-BIT mode
    write_command(0x02);
    Delay_us(100);
    write_command(0x02);
    Delay_us(100);

    //2 line, 5x7 format
    write_command(0x08);
    Delay_us(100);

    //Display ON, cursor ON, blinking
    write_command(0x0F);
    Delay_us(100);
    write_command(0x0C);    //F
    Delay_us(100);
    write_command(0x06);    //6
    Delay_us(100);
}

void SysTick_Int(){
    SysTick->CTRL = 0;                              //disable systick
    SysTick->LOAD = 0x00FFFFFF;                     //max reload value
    SysTick->VAL = 0;                               //any write to clear
    SysTick->CTRL = 0x00000005;                     //enable systic
}

void Delay_us(uint16_t microsec){
    SysTick->LOAD = ((microsec * 3) - 1);           //1 microseconds
    SysTick->VAL = 0;                               //any write to CVR clears it
    while((SysTick->CTRL & 0x00010000) == 0);       //flag
}

void Delay_ms(uint16_t ms){
    SysTick->LOAD = ((ms * 3000) - 1);              //1 millisecond
    SysTick->VAL = 0;                               //any write to CVR clears it
    while((SysTick->CTRL & 0x00010000) == 0);       //flag
}

void PulseEnablePin(){
    P2OUT &=~ E;                                    //makes sure pulse starts out at 0v
    Delay_us(10);
    P2OUT |= E;
    Delay_us(10);
    P2OUT &=~ E;
    Delay_us(10);
}

void pushNibble(uint8_t nibble){
    P5OUT &=~ 0xF0;                                  //clears P5.4 - P5.7

    Delay_us(10);

    P5OUT |= (nibble & 0x0F) << 4;

    //P5OUT |= (nibble & 0x0F);                     //port pins P5.0 - P5.3 wired to D4-D7
    PulseEnablePin();
}

void pushByte(uint8_t byte){
    uint8_t nibble;

    //nibble = (byte & 0xF0) >> 4;
    nibble = (byte & 0xF0) >> 4;                    //MSB (most significant byte)
    pushNibble(nibble);

    nibble = byte & 0x0F;                           //LSB (least significant byte)
    pushNibble(nibble);

    Delay_us(100);
}

void write_command(uint8_t command){
    P2OUT &=~ RS;                                   //command RS = 0

    Delay_ms(10);
    pushByte(command);
}

void dataWrite(uint8_t data){
    P2OUT |= RS;                                    //data RS = 1

    Delay_ms(10);
    pushByte(data);
}

void lcdSetText(char* text, int x, int y){
    int i;

    if(x <16){
        x |= 0x80;                                  //set LCD for first line write
        switch(y){
        case 0:
            x |= 0x00;                              //set LCD for the first line write
            break;
        case 1:
            x |= 0x40;                              //set LCD for second line write
            break;

        case 2:
            x |= 0x10;                              //set LCD for third line write
            break;

        case 3:
            x |= 0x50;                              //set LCD for fourth line write
            break;

        case 5:
            x |= 0x20;                              //set LCD for second line write reverse
            break;
        }
        write_command(x);
    }

    i = 0;
    while(text[i] != '\0'){
        dataWrite(text[i]);
        i++;
    }
}

void lcdSetInt(int val, int x, int y){
    char number_string[16];

    sprintf(number_string, "%d ", val);              //converts the integer to a string
    lcdSetText(number_string, x, y);
}

void lcdSetFloat(float val, int x, int y){
    char number_string[16];
    sprintf(number_string, "%0.1fC", val);              //converts the integer to a string
    lcdSetText(number_string, x, y);
}



