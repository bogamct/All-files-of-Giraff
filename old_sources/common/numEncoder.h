#ifndef NUM_ENCODER_H
#define NUM_ENCODER_H

#include <common/vartypes.h>

void encodeFloat( char* p_buf, char* p_str, float p_num );
void encodeS16( char* p_buf, char* p_str, s16 p_num );
void encodeU16( char* p_buf, char* p_str, u16 p_num );
u08 toHex(u08 v);
void addCRCAndCR( char *p_buf );

#endif
