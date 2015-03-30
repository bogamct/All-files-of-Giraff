/*************************************************************************
Title:    example program for the Interrupt controlled UART library
Author:   Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:     $Id: test_uart.c,v 1.4 2005/07/10 11:46:30 Peter Exp $
Software: AVR-GCC 3.3
Hardware: any AVR with built-in UART, tested on AT90S8515 at 4 Mhz

DESCRIPTION:
          This example shows how to use the UART library uart.c

*************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/delay.h>
#include <avr/pgmspace.h>

#include "serial.h"

void (*jumpToBoot1)(void) = 0x3800;

void ledsInit(void) {
  DDRC = 1+2+4+8+16+32+64;
}

void ledsSet(unsigned num) {
  PORTC =~num;
}

int main(void) {
  u08 c;
  char buffer[7];
  int  num=1;
  bool isFlowControlOn = true;
  bool isHexModeOn = false;

  ledsSet(1);
  ledsInit();
  
  ledsSet(2);
  uartInit( UART_BAUD_SELECT( 9600, F_CPU ) );
  
  ledsSet(3);
  uartFlowControlOn( true );
  
  ledsSet(4);
  uartPutString("String stored in SRAM\n");
  
  ledsSet(5);    
  uartPutString("String stored in FLASH\n");
  
  ledsSet(6);        
  itoa( num, buffer, 10);       // convert interger into string (decimal format)         
  uartPutString(buffer);        // and transmit string to UART
  uartPutString( "\n" );
  
  ledsSet(7);    
  //while ( !uartIsCharAvailable() ) {
    //do nothing
  //}
  //c = uartBlockingGetChar();
  uartPutString( "You pressed: " );
  //uartPutChar( c );
  uartPutString( "\n" );
  
  for(;;) {
    ledsSet(11);    
    c = uartBlockingGetChar();
    //_delay_ms( 100 );
    ledsSet(12);    
    
    if ( c == 'V' ) {
      uartPutString( "testSerial: " );
      uartPutString( __DATE__ );
      uartPutString( " " );
      uartPutString( __TIME__ );
      uartPutString( "\n" );
    } else if ( c == 'B' ) {
      MCUCR = _BV(IVCE);
      MCUCR = _BV(IVSEL);               //move interruptvectors to the boot sector sector
      asm volatile("jmp 0x3800");
    } else if ( c == 'F' ) {
      //toggle flow control
      isFlowControlOn = !isFlowControlOn;
      uartFlowControlOn( isFlowControlOn );
      
      if ( isFlowControlOn ) {
	uartPutString( "Flow Control On\n" );
      } else {
	uartPutString( "Flow Control Off\n" );
      }
    } else if ( c == 'H' ) {
      isHexModeOn = !isHexModeOn;
      if ( isHexModeOn ) {
	uartPutString( "Hex Mode On\n" );
      } else {
	uartPutString( "Hex Mode Off\n" );
      }
    } else {
      if ( isHexModeOn ) {
	if ( c==10 || c==13 ) {
	  uartPutChar( c );
	} else {
	  uartPutChar( '<' );
	  itoa( c, buffer, 16);       // convert interger into string (decimal format)         
	  if ( strlen( buffer ) == 1 ) {
	    uartPutChar( '0' );
	  }
	  uartPutString(buffer);        // and transmit string to UART
	  uartPutChar( '>' );
	}
      } else {
	uartPutChar( c );
      }
    }
  }
}

