#ifndef SHARED_BOOT_LOADER_H
#define SHARED_BOOT_LOADER_H

#include <common/vartypes.h>

// Character used to escape XON, XOFF, and itself
#define ESCAPE 16

u08 getCharWithDecoding(void);
u16 getWord(void);
u16 crcUpdate( u16 p_crc, u08 p_data );
inline u16 getWordWithCRC( u16* p_crc );

#endif
