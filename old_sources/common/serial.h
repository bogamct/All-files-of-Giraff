#include <stdbool.h>
#include <common/vartypes.h>
#include "uart/uart.h"

static u16 m_uartError = 0;
u08 uartBlockingGetChar(void);

static inline void uartInit( u16 p_baudrateFromMacro ) {
  uart_init( p_baudrateFromMacro );
  sei();
}

static inline u16 uartGetErrorAndClear(void) {
  u16 uartError = m_uartError;
  m_uartError = 0;
  return uartError;
}

static inline u16 uartReceiveBufferRemaining() {
  return uart_getrxsize() - uart_getrxcount();
}

static inline void uartFlowControlOn( u16 p_remainingBeforeXOFFSent ) {
  uart_softwareflowcontrol( p_remainingBeforeXOFFSent );
}

static inline bool uartIsCharAvailable(void) {
  return uart_getrxcount() > 0;
}

static inline void uartPutString( const char *s ) {
  uart_puts( s );
}

static inline void uartPutString_P( const char *s ) {
  uart_puts_p( s );
}
static inline void uartPutChar( u08 p_char ) {
  uart_putc( p_char );
}

