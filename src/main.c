#define F_CPU 8000000
#define _bv(bit) (1 << bit)
#define BIT(x)	(1 << (x))

#define numOfPins 270

#include <asf.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


//1	PC0 resred
//2	PC7 pin row c PF0
//3	PC6 pin row b PE0
//4	PC5 pin row a PD0
//5	GND ground
//6	PG0 strobe
//7	PB2 data spi mosi
//8	PB1 spi clock

//PORTD &= ~(1 << n); // Pin n goes low
//PORTD |= (1 << n); // Pin n goes high

void setup(void);
void clearRegisters(void);
void writeRegisters(void);
void setRow(int row);
void setRegisterPin(int index, unsigned char value);
void loopBlock(void);
void setShiftingText(int, int, int);
void setStaticText(int, int, int);

unsigned char registers[numOfPins];

//Character array, 2D array contains the letters from the alphabet and some other characters
unsigned char character_map[40][7]  =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0 space
	0x0E,0x11,0x11,0x1F,0x11,0x11,0x11, //1 A
	0x0F,0x11,0x11,0x0F,0x11,0x11,0x0F, //2 B
	0x1C,0x02,0x01,0x01,0x01,0x02,0x1C, //3 C
	0x07,0x09,0x11,0x11,0x11,0x11,0x0F, //4 D
	0x1F,0x01,0x01,0x0F,0x01,0x01,0x1F, //5 E
	0x1F,0x01,0x01,0x0F,0x01,0x01,0x01, //6 F
	0x0E,0x11,0x01,0x1D,0x11,0x11,0x0E, //7 G
	0x11,0x11,0x11,0x1F,0x11,0x11,0x11, //8 H
	0x1F,0x04,0x04,0x04,0x04,0x04,0x1F, //9 I
	0x1F,0x10,0x10,0x10,0x10,0x11,0x0E, //10 J
	0x11,0x09,0x05,0x03,0x05,0x09,0x11, //11 K
	0x01,0x01,0x01,0x01,0x01,0x01,0x1F, //12 L
	0x11,0x1B,0x15,0x15,0x11,0x11,0x11, //13 M
	0x11,0x13,0x15,0x19,0x11,0x11,0x11, //14 N
	0x0E,0x11,0x11,0x11,0x11,0x11,0x0E, //15 O
	0x0F,0x11,0x11,0x0F,0x01,0x01,0x01, //16 P
	0x0E,0x11,0x11,0x11,0x15,0x09,0x16, //17 Q
	0x0F,0x11,0x11,0x0F,0x05,0x09,0x11, //18 R
	0x1E,0x01,0x01,0x0E,0x10,0x10,0x0F, //19 S
	0x1F,0x04,0x04,0x04,0x04,0x04,0x04, //20 T
	0x11,0x11,0x11,0x11,0x11,0x11,0x0E, //21 U
	0x11,0x11,0x11,0x11,0x11,0x0A,0x04, //22 V
	0x11,0x11,0x11,0x11,0x15,0x15,0x0A, //23 W
	0x11,0x11,0x0A,0x04,0x0A,0x11,0x11, //24 X
	0x11,0x11,0x0A,0x04,0x04,0x04,0x04, //25 y
	0x1F,0x10,0x08,0x04,0x02,0x01,0x1F, //26 Z
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, //27 space
	0x10,0x10,0x08,0x04,0x02,0x01,0x01, //28 /
	0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA, //29 XXX
	0x0E,0x11,0x19,0x15,0x13,0x11,0x0E, //30 0
	0x04,0x06,0x04,0x04,0x04,0x04,0x0E, //31 1
	0x0E,0x11,0x10,0x08,0x04,0x02,0x1F, //32 2
	0x0E,0x11,0x10,0x0C,0x10,0x11,0x0E, //33 3
	0x11,0x11,0x11,0x1F,0x10,0x10,0x10, //34 4
	0x1F,0x01,0x01,0x0E,0x10,0x11,0x0E, //35 5
	0x0E,0x01,0x01,0x0F,0x11,0x11,0x0E, //36 6
	0x1F,0x10,0x10,0x08,0x04,0x02,0x01, //37 7
	0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E, //38 8
	0x0E,0x11,0x11,0x1E,0x10,0x11,0x0E, //39 9
};

/* used to initialize the pins on the BIG avr, make this the fist function you call in your main()*/
void setup(void)
{
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRG = 0xFF;

	PORTC |= (1 << 0);
	clearRegisters();
	writeRegisters();
}

/* clear the register array by writing 0 on every position*/
void clearRegisters(void)
{
	for(int i = numOfPins-1; i >=0; i--)
	{
		registers[i] = 0;
	}
}

/* used for writing the register array to the registers on the board*/
void writeRegisters(void)
{
	PORTG &= ~(1 << 0);						// set strobe pin low, to make sure LEDS dont turn on/off while changing the data
	for(int i = 271; i >= 0; i--)
	{
		PORTB &= ~(1 << 1);					// set clock pin low
		unsigned char val = registers[i];
		if(val)
			PORTB |=(1 << 2);				// set data pin high
		else
			PORTB &= ~(1 << 2);				// set data pin low
		PORTB |= (1 << 1);					// set clock pin high
	}
	PORTG |= (1 << 0);						// set strobe pin high, to change the LEDS according to the data in the shiftregister
}


/*	used for setting a single pin high or low (pin high = LED on, pin low = LED off)
	int index is the LED you want to set, range from 0 to 269
	char value is the value you want to set, either 1(LED on) or 0 (LED off)
 */
void setRegisterPin(int index, unsigned char value)
{
	registers[index] = value;
}

/*	used to multiplex the rows on the board, 
*	int row ranging from 0 to 6 because there are 6 rows on each block
*/
void setRow(int row)
{
	PORTC = 0x1F;

	PORTC |= ((row & 1) << 5);
	PORTC |= ((row & 2) << 5);
	PORTC |= ((row & 4) << 5);
}

/*	the main of the program, put call the code you want to run somewhere in here */
int main (void)
{
	board_init();
	setup();

	while(1) // this loop will shift the word 'final' on the top row and the word 'assessment' on the bottom row
	{
		for(int shift = 10000; shift > 0; shift--)
		{
			for(int row = 0; row < 7; row++)
			{
				setRow(row);

				//ASSESMENT
				setShiftingText(row, shift, 6);
				setShiftingText(row, shift+6, 9);
				setShiftingText(row, shift+12, 14);
				setShiftingText(row, shift+18, 1);
				setShiftingText(row, shift+24, 12);

				//FINAL
				setStaticText(row, 196, 1);
				setStaticText(row, 202, 19);
				setStaticText(row, 208, 19);
				setStaticText(row, 214, 5);
				setStaticText(row, 220, 19);
				setStaticText(row, 226, 19);
				setStaticText(row, 232, 13);
				setStaticText(row, 238, 5);
				setStaticText(row, 244, 14);
				setStaticText(row, 250, 20);

				writeRegisters();
				clearRegisters();
				writeRegisters();
			}
			_delay_ms(10);
		}
	}
}

/*	used to shift text on the matrix
*	int row is the row you are writing on ranging from 0-6
*	int pos is the position you are writing the character on, ranging from 0-269
*	int character is the column on the character you are writing ranging from 0-4 (each character is 5x7 LED's)
*/
void setShiftingText(int row, int pos, int character)
{
	setRegisterPin(pos%85,character_map[character][row]&1);
	setRegisterPin(pos%85+1,character_map[character][row]&2);
	setRegisterPin(pos%85+2,character_map[character][row]&4);
	setRegisterPin(pos%85+3,character_map[character][row]&8);
	setRegisterPin(pos%85+4,character_map[character][row]&16);
}

/*	used to write static text on the matrix
*	int row is the row you are writing on ranging from 0-6
*	int pos is the position you are writing the character on, ranging from 0-269
*	int character is the column on the character you are writing ranging from 0-4 (each character is 5x7 LED's)
*/
void setStaticText(int row, int pos, int character)
{
	setRegisterPin(pos,character_map[character][row]&1);
	setRegisterPin(pos+1,character_map[character][row]&2);
	setRegisterPin(pos+2,character_map[character][row]&4);
	setRegisterPin(pos+3,character_map[character][row]&8);
	setRegisterPin(pos+4,character_map[character][row]&16);
}

/*	some funky test function to check if all leds on the matrix are working
*	one time shifts a block of 5x7 LED's accross the board, you can call this in a while loop to make it run forever
*/
void loopBlock(void)
{
	for(int col = 0; col < 270; col++)
	{
		for(int row = 0; row < 7; row++)
		{
			setRow(row);
			setRegisterPin(col, 1);
			setRegisterPin(col+1, 1);
			setRegisterPin(col+2, 1);
			setRegisterPin(col+3, 1);
			setRegisterPin(col+4, 1);
			writeRegisters();
			_delay_ms(1);
		}
		clearRegisters();
	}
}