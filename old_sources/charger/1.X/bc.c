#include <stdlib.h>
#include <string.h>

#ifndef SIMULATE
#include "../../common/crc.h"
#include "../../common/commonSPI.h"
#include "../../common/numEncoder.h"
#endif

#include "chargerInfo.h"
#include "a2d.h"
#include "a2d_hal.h"
#include "bc_hal.h"
#include "bc.h"
#include "timer.h"
#include "version.h"
#include "batt_status.h"
#include "batt_info.h"
#include "charger_status.h"

#define LED_TIMER_INTERVAL_MICROS    200000
#define RTC_TIMER_INTERVAL_MICROS    1000000
#define INIT_TIMER_INTERVAL_MICROS   1500000
#define BEFORE_MB_ON_INTERVAL_MICROS 500000
#define MB_ON_INTERVAL_MICROS        100000
#define CHARGING_EFFICIENCY          (0.87f)
// save charger status every five minutes
#define CHARGER_SAVE_SECONDS         (5*60)
#define LED_TIMER_TICKS (( MICROS_TO_TIMER_TICKS( LED_TIMER_INTERVAL_MICROS ) ))
#define RTC_TIMER_TICKS (( MICROS_TO_TIMER_TICKS( RTC_TIMER_INTERVAL_MICROS ) ))
#define INIT_TIMER_TICKS (( MICROS_TO_TIMER_TICKS( INIT_TIMER_INTERVAL_MICROS ) ))
#define BEFORE_MB_ON_TICKS (( MICROS_TO_TIMER_TICKS( BEFORE_MB_ON_INTERVAL_MICROS ) ))
#define MB_ON_TICKS (( MICROS_TO_TIMER_TICKS( MB_ON_INTERVAL_MICROS ) ))

#define MB_STATE_INIT 0
#define MB_STATE_ON 1
#define MB_STATE_OFF 2

#define NUM_MESSAGES 9

volatile u16 m_ledTimer = 0;
volatile u16 m_dataSendTimer = 0;
volatile u16 m_makeAdjustTimer = 0;
volatile u16 m_rtcTimer = 0;
volatile u16 m_initTimer = INIT_TIMER_TICKS;
volatile u16 m_mbTimer = BEFORE_MB_ON_TICKS;
u16 m_targetPWM = 1;
u16 m_chargerSaveTimer = 0;

#ifndef SIMULATE
	char m_rcvBuff[SPI_RX_BUFFER_SIZE];
	char m_tmpBuff[SPI_TX_BUFFER_SIZE];
#endif
u08 m_rcvBuffIndex=0;

u08 m_mbState = MB_STATE_INIT;
u08 m_chargeStatus = STATUS_INIT;
u08 m_termination = 0;
u08 m_warnings = 0;
u08 m_numberOfBlinks = 0;
 
bool m_ledOn = false;
u08 m_currentErrorBlink = 0;

//bool m_coulombCounterEnabled = true;
float m_wattSeconds;
float m_oldWattSeconds;
u08 m_messageCounter=0;

extern u32 m_battCounter;

u08 indexToCmd( u08 p_index ) {
  switch ( p_index ) {
  case 0:return CMD_DOCK_CURRENT;
  case 1:return CMD_DOCK_VOLTAGE;
  case 2:return CMD_BATT_CURRENT;
  case 3:return CMD_BATT_VOLTAGE;
  case 4:return CMD_BATT_TEMPERATURE;
  case 5:return CMD_OFFSET_ADJUSTMENT;
  }
  m_termination = TERM_SOFTWARE_ERROR1;
  return 0;
}


/*
  void handleLine();

  void handleChar( u08 p_char ) {
  if ( p_char == 13 ) {
  m_rcvBuff[m_rcvBuffIndex]=0;
  m_rcvBuffIndex=0;
  handleLine();
  } else {
  m_rcvBuff[m_rcvBuffIndex++] = p_char;
  if ( m_rcvBuffIndex >= RCV_BUFFER_SIZE ) {
  m_rcvBuffIndex = RCV_BUFFER_SIZE - 1;
  }
  }
  }


  void handleLine() { 
  if ( m_rcvBuff[0] == CMD_BUCK && m_rcvBuff[1] == ' ' ) {
  m_targetPWM = atoi(m_rcvBuff+2);
  } 
  }
*/

void addString( char *p_buf, char *p_key, char *p_str ) {
  char* ptr = p_buf + strlen( p_buf );
  strcpy( ptr, p_key );
  strcpy( ptr+3, p_str );
}

void outputChargerData() {
#ifndef SIMULATE
  // if there's nothing in the transmit buffer
  if ( spiGetTXCount() == 0 ) {
    m_messageCounter++;

    // to avoid overflowing the SPI buffer [32 bytes] we send all the data as separate messages
    // but always include the status since that is needed to quickly detect dock & undock
    u08 messageNum = m_messageCounter % NUM_MESSAGES;
    m_tmpBuff[0]=0;

    // This uses 8 chars
    encodeS16( m_tmpBuff, "S:", (m_warnings << 12) | (m_chargeStatus << 8) | m_termination );

    switch ( messageNum ) {
    case 0:
      // this uses 9 chars
      encodeS16( m_tmpBuff, ",P:", getPWM() );
      break;
    case 1:
      // this uses 9 chars
      encodeS16( m_tmpBuff, ",T:", (((m_time.hour*60)+m_time.min)*60)+m_time.sec );
      break;
    case 2:
      // this uses 13 chars.  The open curly bracket is required because otherwise C doesn't let you
      // declare a variable in a switch statement
      encodeFloat( m_tmpBuff, ",B:", getBatteryVoltage() );
      break;
    case 3:
      // this uses 13 chars
      encodeFloat( m_tmpBuff, ",b:", getBatteryCurrent() );
      break;
    case 4:
      // this uses 13 chars
      encodeFloat( m_tmpBuff, ",t:", getBatteryTemperatureInC() );
      break;
    case 5:
      // this uses 13 chars
      encodeFloat( m_tmpBuff, ",D:", getDockVoltage() );
      break;
    case 6:
      encodeFloat( m_tmpBuff, ",d:", getDockCurrent() );
      break;
    case 7:
      // this uses 13 chars
      encodeFloat( m_tmpBuff, ",W:", m_wattSeconds );
      break;
    case 8:
      // this uses <= 12 chars
      addString( m_tmpBuff, ",v:", CHARGER_VERSION );
      // add user abbreviation to version string
      char* ptr = m_tmpBuff + strlen( m_tmpBuff );
      strcpy( ptr, CHARGER_USER_ABBREV );
      break;
      /*
	case 9: 
	// this is used for debugging arbitrary values
	encodeS16( m_tmpBuff, ",Z:", (s16) m_battCounter );
	break;
	}
      */
    }
    addCRCAndCR( m_tmpBuff ); // can add 7 characters
    spiPutString( m_tmpBuff );
  }
#endif 
}

u08 m_updatingA2D=false;

void doImportantStuff() {
  // since doImportantStuff is called before and after time-sensitive operations in updateA2D(), we want to make sure 
  // we don't start executing updateA2D() when we're already in updateA2D()
  if ( !m_updatingA2D ) {
    m_updatingA2D = true;
    updateA2D();
    m_updatingA2D = false;
  }
#ifndef SIMULATE
  /*
    if ( spiIsCharAvailable() ) {
    u08 received = spiGetChar();
    handleChar( received );      
    }
  */
#endif
}

void startedCharging() {
}

void finishedCharging( bool p_chargingCompleted ) {
  // if we're finished charging, then the battery is full and we should stop counting coulombs, even though
  // we are still trickle charging
  if (p_chargingCompleted) {
    //    m_coulombCounterEnabled = false;
    m_wattSeconds = 0;
  }
}

void ensureWattSecondsIsInRange() {
  if (m_wattSeconds < BATT_EMPTY_WATT_SECONDS){
    m_wattSeconds = BATT_EMPTY_WATT_SECONDS;
  } else if (m_wattSeconds > 0){
    m_wattSeconds = 0;
  }
}

void powerSourceSwitch( bool p_powerFromAC ) {
  // if we're on batteries, then we should start counting coulombs again
  //  if (!p_powerFromAC) {
  //    m_coulombCounterEnabled = true;
  //  }
}

int main() {
  rtcInit();
  ledsInit();
  motherboardInit( false );
//  pllInit();
  pwmInit();
  timerInit();
  enableInterrupts();

  a2dInit();

#ifdef SIMULATE
  testResistanceToC();
#else
  spiInitSlave(0);
#endif
  setPWM( 0 );  

  m_chargeStatus = STATUS_INIT;
  m_termination = 0;
  
#ifndef SIMULATE
  // we log the battery status every 5 minutes, so when we init we assume the battery has lost
  // 5 minutes worth of charge.  At roughly 100 minutes of run-time, 5 minutes is 5%
  // Also, the battery self-discharges at a rate of 1% per day, so this accounts for that too
  m_wattSeconds = batt_status_init();
  m_wattSeconds += BATT_EMPTY_WATT_SECONDS*0.05f;
  ensureWattSecondsIsInRange();
  m_oldWattSeconds = m_wattSeconds;
#endif

  u08 oldChargeStatus;

  setLED( RED,   true );
  setLED( GREEN, false );

  while (true) {
    // if we're in an error state, displayStatus does nothing.  The error gets display in the
    // error() method.  Otherwise, we set the two LEDs to show whether the system is docked, fast charging, ...
    displayStatus(); 
    doImportantStuff();

#ifdef SIMULATE
    testCaseExecutor();
#endif
    outputChargerData();

    if ( getSetTimer( &m_rtcTimer, RTC_TIMER_TICKS ) ) {
      updateRTCEverySec();
    }

    oldChargeStatus = m_chargeStatus;
    // Transistion between states depending on m_chargeStatus
    if (m_chargeStatus==STATUS_ERROR) {
      error();
    } else if (m_chargeStatus==STATUS_TRICKLE) {
      trickleCharge();
    } else if (m_chargeStatus==STATUS_INIT) {
      init();
    } else if (m_chargeStatus==STATUS_FAST) {
      fastCharge();
    } else if (m_chargeStatus==STATUS_NOT_DOCKED) {
      notDocked();
    }

    if (oldChargeStatus!=STATUS_NOT_DOCKED && m_chargeStatus==STATUS_NOT_DOCKED) {
      // power from battery
      powerSourceSwitch( false );
    }

    if (oldChargeStatus==STATUS_NOT_DOCKED && m_chargeStatus!=STATUS_NOT_DOCKED) {
      // power from AC
      powerSourceSwitch( true );
    }

    if (oldChargeStatus!=STATUS_FAST && m_chargeStatus==STATUS_FAST) {
      startedCharging();
    }

    if (oldChargeStatus==STATUS_FAST && m_chargeStatus!=STATUS_FAST) {
      // if we used to be fast charging, but now we're not, then charging is complete.  Charging was completed
      // if we're now in trickle mode
      finishedCharging( m_chargeStatus==STATUS_TRICKLE );
    }
  }
  return 0;
}

bool getSetTimer(volatile u16* p_timer, u16 p_resetValue ) {
  bool timerExpired = false;

  timer0Disable();
  if ( !(*p_timer) ) {
    *p_timer = p_resetValue;
    timerExpired = true;
  }
  timer0Enable();
  
#ifdef SIMULATE
  *p_timer = *p_timer - 1; // simulate decrement of hardware timer
#endif
  
  return timerExpired;
}

void coulombCounter() {
  float battVolt = getBatteryVoltage();
  float battCurrent = getBatteryCurrent();
  float wattSecond = battVolt * battCurrent;
  
  // if it's charging, adjust the watt-seconds to account for heat loss in battery
  // the constant was empirally determined by comparing watt-seconds from dead-battery to full-charge and
  // full-charge to dead-battery
  
  if ( battCurrent > 0 ) {
    wattSecond *= CHARGING_EFFICIENCY;
  }
  m_wattSeconds += wattSecond;

  ensureWattSecondsIsInRange();
}

void updateRTCEverySec() {
  if ( 60 == ++m_time.sec ) {
    if ( 60 == ++m_time.min ) {
      if ( 24 == ++m_time.hour ) {
	m_time.hour = 0x00;
      }
      m_time.min = 0x00;
    }
    m_time.sec = 0x00;       
  }
#ifdef SIMULATE
  coulombCounterSimulate();
#else
  coulombCounter();

  m_chargerSaveTimer++;
  if ( m_chargerSaveTimer > CHARGER_SAVE_SECONDS ) {
    m_chargerSaveTimer=0;
    if ( m_oldWattSeconds != m_wattSeconds ) {
      m_oldWattSeconds = m_wattSeconds;
      batt_status_set( m_wattSeconds );
    }
  }
#endif
}

// Defines the ERROR state. The ERROR state should only be called in the event of a fatal error.
// There is no way to exit ERROR mode without restarting the system.
// It is assumed that ultimately an error message will be sent to the system user
void error() {
  // NOTE: WE MUST STOP PWM IN ERROR STATE, SO THAT IT DOESN'T CONTINUE TO CHARGE
  setPWM(0);

  m_numberOfBlinks = m_termination;

  // output error code by blinking LEDS
  if ( getSetTimer( &m_ledTimer, LED_TIMER_TICKS ) ) {

    // Blink the LED	 
    if (m_currentErrorBlink < m_numberOfBlinks) {
      setLED( RED, m_ledOn);
      m_ledOn = !m_ledOn;

      if (!m_ledOn) {	
	m_currentErrorBlink++;
      }
      // Pause at end of blink cycle
    } else if (m_currentErrorBlink < m_numberOfBlinks+1) {
      setLED( RED, false);
      m_ledOn = !m_ledOn; // just for keeping time; not used in this if clause

      if (!m_ledOn) {	
	m_currentErrorBlink++;
      }
      // Begin new blink cycle
    } else {
      m_currentErrorBlink = 0;
    } 
  }
}

// Defines the INIT state. The INIT state merely waits until the m_a2ds array is filled with A2D values, such that each 
// m_a2ds is populated. A quarter second is more than enough time for this to happen.
//
void init() {
  if ( m_mbState == MB_STATE_INIT ) {
    if ( getSetTimer( &m_mbTimer, MB_ON_TICKS ) ) {
      m_mbState = MB_STATE_ON;

      // turn on the MB if the power being consumed by the system is more than .5A
      float battCurrent = getBatteryCurrent();
      float dockCurrent = getDockCurrent();
      float systemPower = dockCurrent - battCurrent;

      if ( systemPower < 0.5f ) {
	motherboardInit( true );
      }
    }
  }

  if ( m_mbState == MB_STATE_ON ) {
    if ( getSetTimer( &m_mbTimer, 0 ) ) {
      m_mbState = MB_STATE_OFF;
      motherboardInit( false );
    }
  }

  if ( getSetTimer( &m_ledTimer, LED_TIMER_TICKS ) ) {
    setLED( RED, !getLED( RED ) );
    setLED( GREEN, !getLED( GREEN ) );
  }

  if ( getSetTimer( &m_initTimer, 0 ) ) {
    if (dockDetected()) {
      // Switch to fast charge mode now that the A2Ds have meaningful values
      m_chargeStatus=STATUS_FAST;
      initFastCharge();
    } else {
      m_chargeStatus=STATUS_NOT_DOCKED;
    }
  }
}

// Defines the STATUS_NOT_DOCKED state. The STATUS_NOT_DOCKED state waits until the docking station is detected.
// Once the robot is docked, the state machine transistions to fast charge mode.
void notDocked() {
  if (dockDetected()) {
    m_chargeStatus=STATUS_FAST;
    initFastCharge();
  }
  setPWM( 0 );
  m_termination = 0;
}


