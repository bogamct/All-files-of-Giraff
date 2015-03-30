#ifndef VARTYPES_H
#define VARTYPES_H

typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned long  u32;
typedef   signed long  s32;
typedef unsigned long long u64;
typedef   signed long long s64;

#define MAX_U08	255
#define MAX_U09 511
#define MAX_U16	65535
#define MAX_U32	4294967295

#define MIN_S08	-128
#define MAX_S08	127
#define MIN_S16	-32768
#define MAX_S16	32767
#define MIN_S32	-2147483648
#define MAX_S32	2147483647

//#define bool u08
//#define true 1
//#define false 0

#endif
