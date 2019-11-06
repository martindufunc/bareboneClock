#include <avr/io.h>
#include <avr/delay.h>
#include "myI2C.h"


void myI2C_init()
{
	PORT_USI |= (1<<PIN_USI_SDA);	// Enable pullup on SDA, to set high as released state.
	PORT_USI |= (1<<PIN_USI_SCL);	// Enable pullup on SCL, to set high as released state.
	DDR_USI |= (1<<PIN_USI_SCL);	// Enable SCL as output.
	DDR_USI |= (1<<PIN_USI_SDA);	// Enable SDA as output.
	USIDR = 0xFF;					// Preload dataregister with "released level" data.
	USICR = (0<<USISIE)|(0<<USIOIE)|(1<<USIWM1)|(0<<USIWM0)|(1<<USICS1)|(0<<USICS0)|(1<<USICLK)|(0<<USITC);
	USISR = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0); 
}

void myI2C_stop()
{
	PORT_USI &= ~(1<<PIN_USI_SDA);			// Pull SDA low.
	PORT_USI |= (1<<PIN_USI_SCL);			// Release SCL.
	while( !(PIN_USI & (1<<PIN_USI_SCL)) );	// Wait for SCL to go high.
	_delay_loop_2( T4_TWI );
	PORT_USI |= (1<<PIN_USI_SDA);			// Release SDA.
	_delay_loop_2( T2_TWI );
}

void myI2C_start()
{
	PORT_USI |= (1<<PIN_USI_SCL);			// Release SCL.
	while( !(PORT_USI & (1<<PIN_USI_SCL)) );// Verify that SCL becomes high.
	#ifdef TWI_FAST_MODE
	_delay_loop_2( T4_TWI );				// Delay for T4TWI if TWI_FAST_MODE
	#else
	_delay_loop_2( T2_TWI );				// Delay for T2TWI if TWI_STANDARD_MODE
	#endif
	PORT_USI &= ~(1<<PIN_USI_SDA);			// Force SDA LOW.
	_delay_loop_2( T4_TWI );
	PORT_USI &= ~(1<<PIN_USI_SCL);			// Pull SCL LOW.
	PORT_USI |= (1<<PIN_USI_SDA);			// Release SDA.
}

unsigned char myI2C_master_write(unsigned char *msg, unsigned char msgSize)
{
	unsigned char tempUSISR_8bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0);
	unsigned char tempUSISR_1bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0xE<<USICNT0);
	//myI2C_start();
	do 
	{
		PORT_USI &= ~(1<<PIN_USI_SCL);	        // Pull SCL LOW.
		USIDR = *(msg++);	                    // Setup data.
		myI2C_master_transfer(tempUSISR_8bit);	// Send 8 bits on bus.
		/* Clock and verify (N)ACK from slave */
		DDR_USI  &= ~(1<<PIN_USI_SDA);	        // Enable SDA as input.
		if(myI2C_master_transfer(tempUSISR_1bit) & (1<<TWI_NACK_BIT)) 
			return(0);
	}
	while(--msgSize);	
	//myI2C_stop();	
}

unsigned char myI2C_master_read(unsigned char *msg, unsigned char msgSize)
{
	unsigned char tempUSISR_8bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0);
	unsigned char tempUSISR_1bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0xE<<USICNT0);	
	//myI2C_start();
	do
	{
		DDR_USI  &= ~(1<<PIN_USI_SDA);			// Enable SDA as input.
		*(msg++) = myI2C_master_transfer(tempUSISR_8bit);
		/* Prepare to generate ACK (or NACK in case of End Of Transmission) */
		if( msgSize == 1) USIDR = 0xFF;
		else USIDR = 0x00;
		myI2C_master_transfer(tempUSISR_1bit);	// Generate ACK/NACK.
	}
	while(--msgSize);	
	//myI2C_stop();	
}

unsigned char myI2C_master_transfer(unsigned char temp)
{
	USISR = temp;								// Set USISR according to temp.
	temp = (0<<USISIE)|(0<<USIOIE)|(1<<USIWM1)|(0<<USIWM0)|(1<<USICS1)|(0<<USICS0)|(1<<USICLK)|(1<<USITC); // Interrupts disabled, Set USI in Two-wire mode, Software clock strobe as source, Toggle Clock Port.
	do
	{
		_delay_loop_2( T2_TWI );
		USICR = temp;							// Generate positive SCL edge.
		while( !(PIN_USI & (1<<PIN_USI_SCL)) );	// Wait for SCL to go high.
		_delay_loop_2( T4_TWI );
		USICR = temp;							// Generate negative SCL edge.
	}	while( !(USISR & (1<<USIOIF)) );		// Check for transfer complete.
	
	_delay_loop_2( T2_TWI );
	temp  = USIDR;								// Read out data.
	USIDR = 0xFF;								// Release SDA.
	DDR_USI |= (1<<PIN_USI_SDA);				// Enable SDA as output.
	return temp;								// Return the data from the USIDR
}
