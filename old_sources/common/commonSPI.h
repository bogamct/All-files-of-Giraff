#ifndef COMMON_SPI_H
#define COMMON_SPI_H

#include <stdbool.h>
#include <common/vartypes.h>

#define SPI_NO_DATA 0

#ifdef SPI_USES_USI_MODULE
void spiInitSlave( char p_spiMode );

#define USI_OUT_REG	PORTB	//!< USI port output register.
#define USI_IN_REG	PINB	//!< USI port input register.
#define USI_DIR_REG	DDRB	//!< USI port direction register.
#define USI_CLOCK_PIN	PB2	//!< USI clock I/O pin.
#define USI_DATAIN_PIN	PB0	//!< USI data input pin.
#define USI_DATAOUT_PIN	PB1	//!< USI data output pin.
#endif

#ifdef SPI_SLAVE_USES_SPI_MODULE
void spiInitSlave();

#define SPI_OUT_REG	PORTB	//!< SPI port output register.
#define SPI_IN_REG	PINB	//!< SPI port input register.
#define SPI_DIR_REG	DDRB	//!< SPI port direction register.
#define SPI_SCK_PIN	PB7		//!< SPI clock I/O pin.
#define SPI_MOSI_PIN	PB5	//!< SPI data input pin.
#define SPI_MISO_PIN	PB6	//!< SPI data output pin.
#endif

#ifdef SPI_USES_SPI_MODULE
void spiInitMaster();
void spiDisableMaster();
void spiMasterTick();

#define SPI_OUT_REG PORTB
#define SPI_IN_REG PINB
#define SPI_DIR_REG DDRB
#define SPI_MOSI_PIN PB5
#define SPI_MISO_PIN PB6
#define SPI_SCK_PIN PB7
#define SPI_SS_PIN PB4
#endif

bool spiIsCharAvailable();
u08 spiGetRXCount();
u08 spiGetTXCount();
u08 spiGetTXHead();
u08 spiGetTXTail();
u08 spiGetErrors();
void spiPutChar( char p_char );
void spiPutString( char* p_str );
u08 spiGetChar();
void spiClearErrors();
void spiSetTenBits(char* p_ptr, u16 p_num);
u16 spiGetTenBits(char* p_ptr);

#endif
