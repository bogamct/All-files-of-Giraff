#include "crc.h"

u16 crcUpdate( u16 p_crc, u08 p_data ) {
  //return _crc_ccitt_update( p_crc, p_data );
  p_data ^= p_crc & 0xff;
  p_data ^= p_data << 4;

  return (( ((u16)p_data << 8) | (p_crc >> 8)) ^ (u08)(p_data >> 4) ^ ((u16)p_data << 3));
}


