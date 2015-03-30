#ifndef _SIMULATORHAL_H_
#define _SIMULATORHAL_H_

#include "kbhit.h"
#include <common/vartypes.h>

s32 getSimulatedServoPosition();
int getSimulatedServoHomeSwitchState();
void simulateAlteredPowerOfServo(s32 servoError);
void flushSimulatedServoPosition();

void uartInit();
int uartIsCharAvailable();
char uartBlockingGetChar();
void uartPutChar( u08 p_char );
void uartPutString( const char *s );



void encoderInit();
long encoderGetPosition(unsigned char encoderNum);
void encoderSetPosition(unsigned char encoderNum, long position);
int strcmp_P (const char * string1, const char * string2);
char *  ltoa ( long value, char * buffer, int radix );
char * 	dtostrf (double __val, char __width, char __prec, char *__s);
void _delay_ms(unsigned int ms);
void encoder_simulator(unsigned char encoderNum, long position);
void encoderOff(void);
void simulate_timer_interrupt(u16* p_timer);


// SPI stuff
void spiDisableMaster();
int spiIsCharAvailable();
unsigned char spiGetErrors();
void spiClearErrors();
void spiInitMaster();
u08 spiGetChar();
void spiMasterTick();
void spiPutChar( char p_char );
void spiPutString( char* p_str );


#endif // _SIMULATORHAL_H_
