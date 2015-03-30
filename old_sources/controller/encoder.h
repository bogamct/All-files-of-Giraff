#ifndef ENCODER_H
#define ENCODER_H

#ifndef UNIT_TEST

#include <common/vartypes.h>

#include "bittwiddle.h"

// constants/macros/typdefs

// defines for processor compatibility
// quick compatiblity for mega128, mega64 
//#ifndef MCUCR
//	#define	MCUCR	EICRA
//#endif

// Set the total number of encoders you wish to support
#define IMSK EIMSK

// constants/macros/typdefs

// defines for processor compatibility
// chose proper Interrupt Mask (IMSK)
#ifdef EIMSK
	#define IMSK	EIMSK	// for processors mega128, mega64
#endif
#ifdef GICR
	#define IMSK	GICR	// for mega16,32,etc
#endif
// default
#ifndef IMSK
	#define IMSK	GIMSK	// for other processors 90s8515, mega163, etc
#endif

#endif // UNIT_TEST

//! Encoder state structure
//   stores the position and other information from each encoder
typedef struct struct_EncoderState {	
	s32 position;		///< position
} EncoderStateType;


// functions

//! encoderInit() initializes hardware and encoder position readings
//		Run this init routine once before using any other encoder function.
void encoderInit(void);

//! encoderOff() disables hardware and stops encoder position updates
void encoderOff(void);

//! encoderGetPosition() reads the current position of the encoder 
s32 encoderGetPosition(u08 encoderNum);

//! encoderSetPosition() sets the current position of the encoder
void encoderSetPosition(u08 encoderNum, s32 position);

#endif
