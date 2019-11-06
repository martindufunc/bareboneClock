#ifndef MYI2C_H_
#define MYI2C_H_

//#define TWI_FAST_MODE

#define SYS_CLK   4000.0  // [kHz]

#ifdef TWI_FAST_MODE               // TWI FAST mode timing limits. SCL = 100-400kHz
#define T2_TWI    ((SYS_CLK *1300) /1000000) +1 // >1,3us
#define T4_TWI    ((SYS_CLK * 600) /1000000) +1 // >0,6us
#else                              // TWI STANDARD mode timing limits. SCL <= 100kHz
#define T2_TWI    ((SYS_CLK *4700) /1000000) +1 // >4,7us
#define T4_TWI    ((SYS_CLK *4000) /1000000) +1 // >4,0us
#endif

#define TWI_NACK_BIT  0       // Bit position for (N)ACK bit.


// Tiny2313
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB5
#define PORT_USI_SCL        PORTB7
#define PIN_USI_SDA         PINB5
#define PIN_USI_SCL         PINB7

void myI2C_init();
void myI2C_stop();
void myI2C_start();
unsigned char myI2C_master_write(unsigned char *msg, unsigned char msgSize);
unsigned char myI2C_master_read(unsigned char *msg, unsigned char msgSize);
unsigned char myI2C_master_transfer(unsigned char temp);


#endif /* MYI2C_H_ */
