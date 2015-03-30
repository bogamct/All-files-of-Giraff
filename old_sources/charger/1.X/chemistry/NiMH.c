#include "../a2d.h"
#include "../timer.h"
#include "../batt_info.h"
#include "NiMH.h"
#include "stdio.h"
#include "../charger_status.h"

#define MAKE_ADJUST_INTERVAL_MICROS  50000
#define MAKE_ADJUST_TICKS (( MICROS_TO_TIMER_TICKS( MAKE_ADJUST_INTERVAL_MICROS ) ))

u08 m_fastFinishHour;
u08 m_fastFinishMinute;
u08 m_lastMinute;
u08 m_tricklePaused = 0;

#define SIZE_OF_HISTORICAL_ARRAY 3
// Store last SIZE_OF_HISTORICAL_ARRAY minute values
float m_lastMinuteBattTemperature[SIZE_OF_HISTORICAL_ARRAY];
float m_lastMinuteBattVoltage[SIZE_OF_HISTORICAL_ARRAY];

u16 m_upAdjustmentsInARow=0;
u16 m_downAdjustmentsInARow=0;
extern volatile u16 m_makeAdjustTimer;
extern u16 m_targetPWM;

void initFastCharge() {
  m_time.sec = 0x00;
  m_time.min = 0x00;
  m_time.hour = 0x00;

  m_fastFinishMinute = MAX_TIME_FAST_MINUTES;
  m_fastFinishHour = MAX_TIME_FAST_HOURS;
  
  m_lastMinute=0;

  // Initialize historical array to current values so readings are meaningful even before 4 minutes elapses
  int i;
  for (i=0; i<SIZE_OF_HISTORICAL_ARRAY; i++) {
    m_lastMinuteBattTemperature[i] = getBatteryTemperatureInC();
    m_lastMinuteBattVoltage[i] = getBatteryVoltage();
  }

  // Reset current adjustment error variables to zero each time we re-enter fast charge mode
  m_upAdjustmentsInARow=0;
  m_downAdjustmentsInARow=0;

  setPWM(0);
}

// Something the PWM stays at 505 for a long time in fast mode, so we need to disable the up adjustment limit
// when we are in fast mode
void adjustCurrent( float p_target, bool p_disableUpAdjustmentLimit ) {
  float battCurrent = getBatteryCurrent();
  u16 pwm = getPWM();

  if ( m_targetPWM != 1 ) {
    setPWM( m_targetPWM );
  } else {
    if ( battCurrent < p_target ) {
      // We don't let PWM go over 505 because once it goes past that, the charge-pump has trouble restarting if it has
      // overshot to 511 and tries to go down
      if ( pwm < 505 ) {
	setPWM(pwm+1);
	
	m_upAdjustmentsInARow++;
      } else if ( !p_disableUpAdjustmentLimit ) {
	m_upAdjustmentsInARow++;
      }
      
      m_downAdjustmentsInARow=0;
    } else if ( battCurrent > p_target ) {
      if ( pwm > 0 ) {
	setPWM(pwm-1);
      }
      m_downAdjustmentsInARow++;
      m_upAdjustmentsInARow=0;
    }
  }
}


void fastCharge() {
  if (undockDetected()) {
    return;
  }

  if (temperatureChargingErrorCondition()) {
    return;
  }

  if (maxVoltageErrorCondition()) {
    return;
  }

  if (maxCurrentAdjustmentErrorCondition()) {
    return;
  }

  if (maxTimeErrorCondition()) {
    return;
  }

  if ( dVdT_or_dTdT_Reached() ) {
    return;
  }

  if ( temperatureTooHotForFastCharge() ) {
    return;
  }

  if ( getSetTimer( &m_makeAdjustTimer, MAKE_ADJUST_TICKS ) ){
    adjustCurrent( FAST_CHARGE_CURRENT, true );
  }
}

void trickleCharge() {
  if (maxVoltageErrorCondition()) {
    return;
  }
  if (undockDetected()) {
    m_tricklePaused = 0;
    return;
  }

  float temperature = getBatteryTemperatureInC();  
  if (m_tricklePaused) {
    if (temperatureCoolOffErrorCondition()) {
      return;
    }
    
    if ( temperature < TRICKLE_PAUSE_TEMP_C_LOW ) {
      m_tricklePaused = 0;
    }
  } else {
    if ( temperature > TRICKLE_PAUSE_TEMP_C_HIGH ) {
      setPWM( 0 );
      m_upAdjustmentsInARow=0;
      m_downAdjustmentsInARow=0;
      m_tricklePaused = 1;
    } else {      
      if (temperatureChargingErrorCondition()) {
	return;
      }

      if ( getSetTimer( &m_makeAdjustTimer, MAKE_ADJUST_TICKS ) ) {
	adjustCurrent( BATT_TRICKLE_CHARGE_CURRENT, false );
      }
    }
  }
}



// Checks for various state transitions

bool temperatureChargingErrorCondition() {
  float temperature = getBatteryTemperatureInC();

  if (temperature < MIN_TEMP_C_ERROR) {
    m_termination=TERM_MIN_TEMP;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }

  if (temperature > MAX_TEMP_C_CHARGING_ERROR) {
    m_termination=TERM_MAX_TEMP;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }

  return false;
}

bool temperatureCoolOffErrorCondition() {
  float temperature = getBatteryTemperatureInC();

  if (temperature < MIN_TEMP_C_ERROR) {
    m_termination=TERM_MIN_TEMP;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }

  if (temperature > MAX_TEMP_C_COOLOFF_ERROR) {
    m_termination=TERM_MAX_TEMP;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }

  return false;
}


// TERM_MAX_TEMP - If temperature sensor rises above maximum, start trickle charging
bool temperatureTooHotForFastCharge() {
  float temperature = getBatteryTemperatureInC();

  if (temperature > MAX_TEMP_C_TERM) {
    m_termination=TERM_MAX_FAST_TEMP;
    m_chargeStatus=STATUS_TRICKLE;
    return true;
  }

  return false;
}

// TERM_MAX_VOLT - If the battery voltage sensor rises above maximum, a transition to the ERROR state must occur
bool maxVoltageErrorCondition() {
  float battVoltage = getBatteryVoltage();

  if (battVoltage > BATT_MAX_VOLTAGE) {
    m_termination=TERM_MAX_VOLT;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }
  return false;
}

// TERM_MAX_CURRENT_ADJUSTMENT - If current range settling time exceeds maximum, a transition to the ERROR state must occur
bool maxCurrentAdjustmentErrorCondition() {
  if (m_upAdjustmentsInARow > MAX_CURRENT_ADJUSTMENTS || m_downAdjustmentsInARow > MAX_CURRENT_ADJUSTMENTS ) {
    m_termination=TERM_MAX_CURRENT_ADJUSTMENT;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }
  return false;
}

// TERM_MAX_TIME - If current adjustment exceeds maximum, a transition to the ERROR state must occur
bool maxTimeErrorCondition() {
  // Check for error and charge termination conditions
  if ((m_time.hour == m_fastFinishHour) && (m_time.min ==  m_fastFinishMinute)) {
    m_termination=TERM_MAX_TIME;
    m_chargeStatus=STATUS_ERROR;
    return true;
  }
  return false;
}


// Add newest battery or voltage value to sliding window of values
void loadNewHistoricalValues (float* oldValues, float newValue) {
  oldValues[0] = oldValues[1];
  oldValues[1] = oldValues[2];
  oldValues[2] = newValue;
}

float oldestBatteryVoltage() {
  return m_lastMinuteBattVoltage[0];
}

float oldestBatteryTemperature() {
  return m_lastMinuteBattTemperature[0];
}

// TERM_DV_DT - If DV/DT slope falls below minimum value, a transition to the TRICKLE state must occur
// TERM_DT_DT - If DT/DT slope rises above maximum value, a transition to the TRICKLE state must occur
bool dVdT_or_dTdT_Reached() {

  float battVoltage = getBatteryVoltage();
  float battTemperature = getBatteryTemperatureInC();

  if (m_time.min != m_lastMinute) {
    m_lastMinute = m_time.min;

    if ((oldestBatteryVoltage() - battVoltage) >= DV_DT) {
      m_termination=TERM_DV_DT;
      m_chargeStatus=STATUS_TRICKLE;
#ifdef SIMULATE
      printf ("DV_DT detected. lastV=%f, currentV=%f\n", oldestBatteryVoltage(), battVoltage);
#endif // SIMULATE
      return true;
    }

    loadNewHistoricalValues (m_lastMinuteBattVoltage, battVoltage);

    if ((battTemperature - oldestBatteryTemperature()) >= DT_DT) {
      m_termination=TERM_DT_DT;
      m_chargeStatus=STATUS_TRICKLE;
#ifdef SIMULATE
      printf ("DT_DT detected. lastT=%f, currentT=%f\n", oldestBatteryTemperature(), battTemperature);
#endif // SIMULATE
      return true;
    }

    loadNewHistoricalValues (m_lastMinuteBattTemperature, battTemperature);
  }
  return false;
}




// DOCK_DETECTED_VOLTAGE_THRESHOLD - if dock voltage rises above threshold, a transition to the FAST state must occur

// UNDOCK_DETECTED - if dock voltage drops below threshold, a transition to the NOT_DOCKED state must occur
bool undockDetected() {
  if (!dockDetected()) {
    m_chargeStatus=STATUS_NOT_DOCKED;
    return true;
  }
  return false;
}

// dockDetected waits until ADC_DOCK_CURRENT climbs above a trigger threshold
// That threshold means that a docking station current is being detected, and hence the robot is now docked.
bool dockDetected() {
  return getDockCurrent() > DOCK_DETECTED_CURRENT_THRESHOLD;
}

