#include <avr/interrupt.h>
#include "serial.h"

u08 uartBlockingGetChar(void) {
  u16 c;
  do {
    c = uart_getc();
  } while ( c & UART_NO_DATA );
  m_uartError |= (c & 0xff00);
  return (u08) c;
}






