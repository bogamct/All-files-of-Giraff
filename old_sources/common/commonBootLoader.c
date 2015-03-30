#include <avr/interrupt.h>
#include "crc.h"
#include "serial.h"
#include "commonBootLoader.h"

u08 getCharWithDecoding(void) {
  u08 c = uartBlockingGetChar();    
  if ( c == ESCAPE ) {
    c += uartBlockingGetChar();
  }
  return c;
}

u16 getWord(void) {
  u08 c1, c2;
  u16 data;

  c1 = getCharWithDecoding();
  c2 = getCharWithDecoding();
  data = c1 << 8 | c2;
  return data;
}

inline u16 getWordWithCRC( u16* p_crc ) {
  u16 num;
  
  num = getWord();
  
  *p_crc = crcUpdate( *p_crc, num >> 8 );
  *p_crc = crcUpdate( *p_crc, num & 0xFF );
  return num;
}



