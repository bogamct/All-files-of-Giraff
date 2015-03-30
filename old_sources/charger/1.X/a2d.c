#include "chargerInfo.h"
#include "a2d.h"
#include "a2d_hal.h"
#include "bc.h"
#include "charger_status.h"

#ifdef SIMULATE
extern float m_simulatedBattV;
extern float m_simulatedBattC;
extern float m_simulatedBattT;
extern float m_simulatedDockV;
extern float m_simulatedDockC;
#endif

#define ADC_STABILIZER_MIN_LOOP 4
#define DOCK_CURRENT_DELTA .05f
#define ADC_COUNT 6

u08 m_a2dIndex=0;
u08 m_offsetSet=0;
u16 m_a2d[ADC_COUNT];
u16 m_a2dStabilizerLoopCount=0;
float m_a2dSmoothed[ADC_COUNT];

extern u08 m_termination;
extern u08 m_chargeStatus;

u08 cmdToIndex( u08 p_cmd ) {
  switch ( p_cmd ) {
  case CMD_DOCK_CURRENT:return 0;
  case CMD_DOCK_VOLTAGE:return 1;
  case CMD_BATT_CURRENT:return 2;
  case CMD_BATT_VOLTAGE:return 3;
  case CMD_BATT_TEMPERATURE:return 4;
  case CMD_OFFSET_ADJUSTMENT:return 5;
  }
  m_termination = TERM_SOFTWARE_ERROR2;
  return 0;
}

u16 getA2DDebugValue() {
  return m_a2d[cmdToIndex( CMD_BATT_TEMPERATURE )];
}

void updateSmoothedValue( u08 p_a2dIndex, u16 p_valueU16 ) {
  u08 adcCommand = indexToCmd( p_a2dIndex );
  float valueFloat=0;

  switch ( adcCommand ) {
  case CMD_BATT_TEMPERATURE:
    valueFloat = a2dToTemperatureC( p_valueU16 );
    break;
  case CMD_BATT_VOLTAGE:
  case CMD_DOCK_VOLTAGE:
    valueFloat = a2dToVoltage( p_valueU16 );
    break;
  case CMD_BATT_CURRENT:
  case CMD_DOCK_CURRENT:
    valueFloat = a2dToCurrent( p_valueU16 );
    break;
  default:
    m_termination = TERM_SOFTWARE_ERROR4;
  }

  float part1, part2;
  if ( adcCommand == CMD_BATT_TEMPERATURE ) {
    // battery temperature A2D is very high impedance and so bounces a lot.  We average over longer time period because of this
    doImportantStuff();
    part1 = m_a2dSmoothed[p_a2dIndex]*0.95f;
    doImportantStuff();
    part2 = valueFloat*0.05f;
  } else if ( adcCommand == CMD_DOCK_CURRENT ) {
    part1 = m_a2dSmoothed[p_a2dIndex];
    part2 = +DOCK_CURRENT_DELTA;
    if ( valueFloat < part1 ) {
      part2 = -DOCK_CURRENT_DELTA;
    }
  } else {
    doImportantStuff();
    part1 = m_a2dSmoothed[p_a2dIndex]*0.90f;
    doImportantStuff();
    part2 = valueFloat*0.10f;
  }
  doImportantStuff();
  m_a2dSmoothed[p_a2dIndex] = part1 + part2;
}

void updateA2DIfStable( u16 p_adcValue ) {
  u08 offsetIndex = cmdToIndex( CMD_OFFSET_ADJUSTMENT );
  if ( m_a2dIndex == offsetIndex) {
    if (!m_offsetSet ) {
      m_a2d[m_a2dIndex] = p_adcValue;
      m_offsetSet = 1;
    }
  } else {
    u16 value = p_adcValue;
    // if we're reading the dock current or batt current, then we're using the differential amplifier (x20)
    // and need to subtract out the offset
    if ( m_a2dIndex == cmdToIndex( CMD_DOCK_CURRENT ) || m_a2dIndex == cmdToIndex( CMD_BATT_CURRENT ) ) {
      u16 offset = m_a2d[offsetIndex];
      value -= offset;
      // if the value is negative when viewed as a s16, add 1024 so that it says a valid 10-bit 2's complement number
      if ( (s16) value < 0 ) {
	value += 1024;
      }
    }
				       
    m_a2d[m_a2dIndex] = value;	
    updateSmoothedValue( m_a2dIndex, value );
  }
  
  m_a2dStabilizerLoopCount=0;
  m_a2dIndex++;
  if ( m_a2dIndex >= ADC_COUNT ) {
    m_a2dIndex=0;
  }
  
  u08 adcCommand = indexToCmd( m_a2dIndex );
  a2dChooseChannel( adcCommand );
}

void updateA2D() {
  if (a2dConversionFinished()) {
    u16 adcValue = a2dGetValue();
    m_a2dStabilizerLoopCount++;

    if (m_a2dStabilizerLoopCount > ADC_STABILIZER_MIN_LOOP) {
      updateA2DIfStable( adcValue );
      m_a2dStabilizerLoopCount=0;
    }

    a2dStartConversion();
  }
}

float getSmoothedValue( u08 p_cmdIndex ) {
  u08 a2dIndex = cmdToIndex( p_cmdIndex );
  return m_a2dSmoothed[ a2dIndex ];
}

float getBatteryTemperatureInC() {
#ifdef SIMULATE
  return m_simulatedBattT;
#else
  return getSmoothedValue( CMD_BATT_TEMPERATURE );
#endif
}

float getBatteryVoltage() {
#ifdef SIMULATE
  return m_simulatedBattV;
#else
  return getSmoothedValue( CMD_BATT_VOLTAGE );
#endif
}

float getBatteryCurrent() {
#ifdef SIMULATE
  return m_simulatedBattC;
#else
  return getSmoothedValue( CMD_BATT_CURRENT );
#endif
}

float getDockVoltage() {
#ifdef SIMULATE
  return m_simulatedDockV;
#else
  return getSmoothedValue( CMD_DOCK_VOLTAGE );
#endif
}

float getDockCurrent() {
#ifdef SIMULATE
  return m_simulatedDockC;
#else
  return getSmoothedValue( CMD_DOCK_CURRENT );
#endif
}

