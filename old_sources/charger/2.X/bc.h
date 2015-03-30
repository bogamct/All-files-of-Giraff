#ifndef BC_H
#define BC_H

#include <common/vartypes.h>
#include <stdbool.h>

// Struct definition
//***************************************************************************
typedef struct {
  char sec;                    // global seconds
  char min;                    // global minutes
  char hour;                   // global hour
} TimeStruct;


//***************************************************************************

u08 indexToCmd(u08);

extern void fastCharge();
extern void initFastCharge();
extern void trickleCharge();
extern bool dockDetected();

TimeStruct m_time;

// fwd declarations
bool getSetTimer(volatile u16* p_timer, u16 p_resetValue );
void updateRTCEverySec();
void error();
void init();
void notDocked();
void sendStateChange();
void doImportantStuff();

#endif // BC_H
