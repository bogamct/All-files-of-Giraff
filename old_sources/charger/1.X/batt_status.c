#include <stdbool.h>
#include <avr/eeprom.h>
#include <common/vartypes.h>
#include "charger_status.h"
#include "batt_status.h"
#include "batt_info.h"

// note that in batt_status_set, this constant is assumed to be 
// 16, because we convert the counter to 0-15 by doing &f to save
// code space
#define NUM_BAT_STATUS 16
#define MARK1 0xDEAD
#define MARK2 0xBEEF

u32 m_battCounter=0;
extern u08 m_warnings;

struct batt_status_type {
  uint16_t mark1; // should be MARK1
  float savedColoumbs;
  uint32_t counter;
  uint16_t mark2; // should be MARK2
};

struct batt_status_type batt_status[NUM_BAT_STATUS] EEMEM;

void batt_status_read( struct batt_status_type* p_status, u08 p_index ) {
  eeprom_read_block( p_status, &batt_status[p_index], sizeof( struct batt_status_type ));
}

void batt_status_write( struct batt_status_type* p_status, u08 p_index ) {
  eeprom_write_block( p_status, &batt_status[p_index], sizeof( struct batt_status_type ));
}

void batt_status_set( float p_savedColoumbs ) {
  struct batt_status_type status;
  
  status.mark1 = MARK1;
  status.savedColoumbs = p_savedColoumbs;
  status.counter = m_battCounter;
  status.mark2 = MARK2;
  
  batt_status_write( &status, m_battCounter & 0xf );
  m_battCounter++;
}

float batt_status_init() {
  u32 highestCounter=0;
  float coloumbOfHighestCounter=0;
  struct batt_status_type status;

  for (int i=0; i < NUM_BAT_STATUS;i++) {
    batt_status_read( &status, i );
    if ( status.mark1 != MARK1 || status.mark2 != MARK2 ) {
      highestCounter=0;
      coloumbOfHighestCounter=BATT_EMPTY_WATT_SECONDS;
      m_warnings |= WARNING_EEPROM_NOT_INITIALIZED;
      break;
    }
    if ( status.counter > highestCounter ) {
      highestCounter = status.counter;
      coloumbOfHighestCounter = status.savedColoumbs;
    }
  }

  // if any of the EEPROM locations are invalid, we need to reset it
  if ( highestCounter==0 ) {
    m_battCounter=0;
    for (int i=0; i < NUM_BAT_STATUS;i++) {
      batt_status_set( BATT_EMPTY_WATT_SECONDS );
    }
  } else {
    m_battCounter = highestCounter+1;
  }
  return coloumbOfHighestCounter;
}
