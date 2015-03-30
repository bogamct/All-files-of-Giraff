#ifndef NIMH_H
#define NIMH_H

#include <math.h>
#include "../bc.h"
#include "../bc_hal.h"
#include "../vartypes.h"

extern TimeStruct m_time;

extern u08 m_chargeStatus;
extern u08 m_termination ;

void fastCharge();
void initFastCharge();
void trickleCharge();

bool temperatureTooHotForFastCharge();
bool temperatureChargingErrorCondition();
bool temperatureCoolOffErrorCondition();
bool maxVoltageErrorCondition();
bool maxCurrentAdjustmentErrorCondition();
bool maxTimeErrorCondition();
bool dVdT_or_dTdT_Reached();
bool undockDetected();
bool dockDetected();


//#endif

#define  DOCK_DETECTED_CURRENT_THRESHOLD 0.75f  // Theshhold dock current at which point we consider the robot to be docked

#define  FAST_CHARGE_CURRENT 1.5f

// Maximum FAST Charge Time in minutes, for example
// 5Ah/1.5A*1.5(heat losses)*60 = 300 minutes
#define  MAX_TIME_FAST_MINUTES_TOTAL  (BATT_SIZE / FAST_CHARGE_CURRENT * 1.5f * 60)
#define  MAX_TIME_FAST_MINUTES  ((int) MAX_TIME_FAST_MINUTES_TOTAL % 60)
#define  MAX_TIME_FAST_HOURS  ((int) MAX_TIME_FAST_MINUTES_TOTAL / 60)
#define  MAX_CURRENT_ADJUSTMENTS 1000           // How many times does the target current have to be off in the same direction
                                                // before its an error

#define  MIN_TEMP_C_ERROR 10.0f        	        // error if temp below MIN_TEMP_C_ERROR
#define  MAX_TEMP_C_TERM  38.5f                 // trickle charge if temp above MAX_TEMP_C_TERM
#define  MAX_TEMP_C_CHARGING_ERROR 45.0f        // error if temp above MAX_TEMP_C_CHARGING_ERROR when charging
#define  MAX_TEMP_C_COOLOFF_ERROR 50.0f         // error if temp above MAX_TEMP_C_COOLOFF_ERROR when cooling off
#define  TRICKLE_PAUSE_TEMP_C_HIGH 38.3f        // pause trickle charging when above this temperature
#define  TRICKLE_PAUSE_TEMP_C_LOW  37.4f        // resume trickle charging when below this temperature

#define  DV_DT (0.0166f * BATT_CELLS) // Abort if voltage is going down by more than .0166*CELLS over four minutes
#define  DT_DT 2.0f              // Abort if temperature going up by more than two degrees over four minutes
  
#endif
