//////////////////////////////////////////////////////////////////////////
// Project: RTC-7seg Digital Clock
// Author:  jrapisora(joselito.rapisora@gmail.com), Aug2013
// 
// To God be the Glory!
//
//////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "myI2C.h"
#define F_CPU 1000000L

//			PINNAME		PORTPIN		//PINNUM	
#define		RESET		PA2			//1			
#define		segG		PD0			//2			
#define		segB		PD1			//3			
//#define	NC			PA1			//4			
//#define	NC			PA0			//5			
#define		segD		PD2			//6			
#define		segA		PD3			//7			
#define		segF		PD4			//8			
#define		segE		PD5			//9			
//#define	GND			NA			//10		
#define		segC		PD6			//11		
#define		segP		PB0			//12		
#define		ce1			PB1			//13		
#define		ce2			PB2			//14		
#define		ce4			PB3			//15		
#define		ce3			PB4			//16		
#define		SDA			PB5			//17		
//#define	NC			PB6			//18		
#define		SCL			PB7			//19		
//#define	VCC			NA			//20		

#define		SEG_PORT	PORTD
#define		SEL_PORT	PORTB
#define		RTC_PORT	PORTB
#define		SEC_PORT	PORTB
#define		SEG_IOREG	DDRD
#define		SEL_IOREG	DDRB
#define		RTC_IOREG	DDRB
#define		SEC_IOREG	DDRB

#define		MESSAGEBUF_SIZE	3	
#define		RTCADDR			0xD0	//DS1307 address
#define		REFRESHRATE		0xBF	//60Hz refresh rate (60Hz x 4 = 240Hz interval between digits)

unsigned char msgBuff[MESSAGEBUF_SIZE];
unsigned char digitVal[4];
unsigned char digitSelect[] = {ce1,ce2,ce3,ce4};
unsigned char segmentOn[] = {0b10000001,0b11110101,0b10100010,0b10100100,0b11010100,0b10001100,0b10001000,0b11100101,0b10000000,0b10000100};
	
//unsigned char decToBcd(unsigned char val) { return ( (val/10*16) + (val%10) ); }
unsigned char bcdToDec(unsigned char val) { return ( (val/16*10) + (val%16) ); }


void sleep()
{
	MCUCR = (0<<SM0)|(0<<SM1)|(1<<SE);
	sei();
	sleep_cpu();
}

void readRTC()
{
	unsigned char x;
	msgBuff[0] = RTCADDR|0x00;      //slave address with write bit
	msgBuff[1] = 0x00;              //address pointer 0x00
	myI2C_start(); 
	myI2C_master_write(msgBuff, 2); //write msgBuff to slave resetting address pointer to 0x00
	msgBuff[0] = RTCADDR|0x01;      //slave address with read bit
	myI2C_start();                  //repeated start
	myI2C_master_write(msgBuff, 1); //write msgBuff to slave for request to read
	myI2C_master_read(msgBuff, 3);  //reading 3bytes to msgBuff starting from  0x00
	myI2C_stop();
	_delay_us(500);

	x = bcdToDec(msgBuff[2]);       //converting read value for each 7seg digit
	digitVal[0] = x / 10;
	digitVal[1] = x % 10;
	x = bcdToDec(msgBuff[1]);
	digitVal[2] = x / 10;
	digitVal[3] = x % 10;		
}

void updateDisplay()
{
	static unsigned char secBit;
	static unsigned char select;
	static unsigned char subSec;
	
	select >= 3 ? select = 0 : select++;              //roll digit select
	SEG_PORT = segmentOn[digitVal[select]];           //update 7-segment value
	SEL_PORT = (0<<ce1)|(0<<ce2)|(0<<ce3)|(0<<ce4);   //clear digit select
	SEL_PORT = (1<<digitSelect[select]);              //enable digit select
	secBit == 1 ? (SEC_PORT |= (1<<segP)) : (SEC_PORT &= ~(1<<segP));   //drive second led
	if(--subSec == 0)
	{
		subSec = 0xF1;
		secBit == 1 ? (secBit = 0) : (secBit = 1);    //toggle second bit
		myI2C_init();
		readRTC();                                    //read rtc once every sec
	}
}

void setup()
{
	//setup port
	SEG_IOREG = 0xff;
	SEL_IOREG = 0xff;
	SEC_IOREG = 0xff;
	SEG_PORT = 0x00;
	SEC_PORT = 0x00;
	SEL_PORT = 0x00;
	//setup timer
	TCCR0B = (0<<CS02)|(1<<CS01)|(1<<CS00);   //clk/64 prescaler
	TCNT0 = REFRESHRATE;
	TIMSK |= (1<<TOIE0);                      //enable interrupt
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = REFRESHRATE;	//reset timer
	updateDisplay();
}

int main(void)
{	
	setup();
    while(1) sleep();
}
