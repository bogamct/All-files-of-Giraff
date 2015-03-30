#include <string.h>
#include <stdio.h>
#include "numEncoder.h"
#include "crc.h"

u08 toHex(u08 v) {
  if (v < 10)
    return v + '0';
  else
    return v - 10 + 'A';
}

void encodeByte( u08 p_byte, char *p_dst ) {
  *(p_dst+0) = toHex( p_byte >> 4 );
  *(p_dst+1) = toHex( p_byte & 0xf );
}

void encodeFloat( char* p_buf, char* p_str, float p_num ) {
  u08* numPtr = (u08*) &p_num;

  char* ptr = p_buf + strlen( p_buf );
  strcpy( ptr, p_str );
  ptr += strlen( p_str );
  ptr[0] = 'F';
  ptr[1] = '#';
  encodeByte( numPtr[0], ptr+2 );
  encodeByte( numPtr[1], ptr+4 );
  encodeByte( numPtr[2], ptr+6 );
  encodeByte( numPtr[3], ptr+8 );
  ptr[10]=0;
}

void encodeS16( char* p_buf, char* p_str, s16 p_num ) {
  u08* numPtr = (u08*) &p_num;

  char* ptr = p_buf + strlen( p_buf );
  strcpy( ptr, p_str );
  ptr += strlen( p_str );
  ptr[0] = 'S';
  ptr[1] = '#';
  encodeByte( numPtr[0], ptr+2 );
  encodeByte( numPtr[1], ptr+4 );
  ptr[6]=0;
}

void encodeU16( char* p_buf, char* p_str, u16 p_num ) {
  u08* numPtr = (u08*) &p_num;

  char* ptr = p_buf + strlen( p_buf );
  strcpy( ptr, p_str );
  ptr += strlen( p_str );
  ptr[0] = 'U';
  ptr[1] = '#';
  encodeByte( numPtr[0], ptr+2 );
  encodeByte( numPtr[1], ptr+4 );
  ptr[6]=0;
}

void addCRCAndCR( char *p_buf ) {
  u16 crc = 0xffff;
  char *ptr = p_buf;
  while (*ptr) {
    crc = crcUpdate( crc, *ptr );
    ptr++;
  }

  //ptr now points to the zero byte at the end of the string

  encodeU16( ptr, " ", crc );    
  ptr[7]=13;
  ptr[8]=0;
}
