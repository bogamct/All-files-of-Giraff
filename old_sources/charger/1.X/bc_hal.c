#ifndef SIMULATE
#include <util/delay.h>
#else
#include "batt_info.h"
#endif //SIMULATE

#include "../../common/robotSpecificSettings.h"
#include "bc_hal.h"
#include "bc.h"
#include "stdio.h"
#include "stdlib.h"
#include "a2d.h"
#include "charger_status.h"

#ifdef SIMULATE
u16 m_simulatedPWM = 0; // simulates PWM register
float m_simulatedBattV=24.0;
float m_simulatedBattC=0.0; 
float m_simulatedBattT=20.0;
float m_simulatedDockV=32.0;
float m_simulatedDockC=0.0;
float m_simulatedAmpHours= BATT_SIZE - 0.5; // default to almost fully charged
u16 m_displayStatusCount = 0;
u16 m_currentTestCase = 0;
u16 m_needToWait = 0;
u16 m_needToWaitHours = 0;
u16 m_lastMinuteATestcaseFailed = -1;
#endif

void rtcInit() {
  m_time.sec = 0x00;
  m_time.min = 0x00;
  m_time.hour = 0x00;
}

void ledsInit() {
#ifndef SIMULATE
  CHRG_LED_RED_DDR |= _BV( CHRG_LED_RED_PIN ) ;
  CHRG_LED_RED_PORT |= _BV( CHRG_LED_RED_PIN );

  CHRG_LED_GREEN_DDR |= _BV( CHRG_LED_GREEN_PIN ) ;
  CHRG_LED_GREEN_PORT &= ~_BV( CHRG_LED_GREEN_PIN );
#endif
}

void pllInit() {
#ifdef SIMULATE
#else
  // Enable PLL
  PLLCSR = _BV( PLLE );

  // We're supposed to wait 100 uS, so this is overkill
  _delay_ms(1);

  // Wait until PLOCK is set
  while ( (PLLCSR & _BV( PLOCK )) == 0 ) {
    // do nothing
  }

  // Timer/Counter1 is asynchronous and connected to PLL -- 64 Mhz
  PLLCSR |= _BV( PCKE );
#endif
}

void inline pwmInitPin() {
#ifndef SIMULATE
  DDRB |= _BV( PB3 );
  PORTB &= ~_BV( PB3 );
#endif
}

void inline setPWMMode( bool p_on ) {
#ifndef SIMULATE
  if ( p_on ) {
    TCCR1A |= _BV( COM1B1 );
    TCCR1C |= _BV( COM1B1S );  
  } else {
    TCCR1A &= ~_BV( COM1B1 );
    TCCR1C &= ~_BV( COM1B1S );
  }
#endif
}

void pwmInit() {
#ifdef SIMULATE
#else
  TCCR1A = _BV( PWM1B );
  TCCR1B = _BV( CS10 );
  TCCR1D = 0;
  TCCR1E = 0;

  setPWMMode( true );
  // Start the counter at zero
  TC1H = 0;
  TCNT1 = 0;

  //Turn off dead-time
  DT1 = 0;

  // Start PWM at 50% duty cycle  
  TC1H = 0;
  OCR1B = 255;

  // TOP is 511
  TC1H = 1;
  OCR1C = 255; 
  pwmInitPin();
#endif 
}

// Function that adjusts PWM width
void setPWM(u16 p_pwm) {
#ifdef SIMULATE
  m_simulatedPWM = p_pwm;
  convertPWMtoCurrent(p_pwm);
#else
  TC1H = p_pwm >> 8;
  OCR1B = p_pwm & 0xFF;
  if ( p_pwm == 0 ) {
    // turn off PWM to make sure there aren't problems with extreme values when use FastPWM
    setPWMMode(false);
    pwmInitPin();
  } else {
    setPWMMode(true);
  }
#endif // SIMULATE
}

u16 getPWM() {
#ifdef SIMULATE
  return m_simulatedPWM;
#else
  u16 pwm = OCR1B;
  pwm |= (TC1H << 8);
  return pwm;
#endif // SIMULATE	
}

void timerInit() {
#ifdef SIMULATE
#else
  TCCR0A = 0;
  TCCR0B = _BV( CS00 );
  TCNT0H =0;
  TCNT0L =0;
  OCR0A = 0;
  OCR0B = 0;

  timer0Enable();
#endif
}

bool getLED(int p_led) {
#ifndef SIMULATE
  if ( p_led==RED ) {
    return (CHRG_LED_RED_PORT & _BV( CHRG_LED_RED_PIN ) ) == _BV ( CHRG_LED_RED_PIN );
  } else {
    return (CHRG_LED_GREEN_PORT & _BV( CHRG_LED_GREEN_PIN ) ) == _BV( CHRG_LED_GREEN_PIN );
  }
#endif
}

void setLED(int p_led, bool p_on) {
#ifndef SIMULATE
  if ( p_led==RED ) {
    if ( p_on ) {
      CHRG_LED_RED_PORT |= _BV( CHRG_LED_RED_PIN );
    } else {
      CHRG_LED_RED_PORT &= ~_BV( CHRG_LED_RED_PIN );
    }
  } else {
    if ( p_on ) {
      CHRG_LED_GREEN_PORT |= _BV( CHRG_LED_GREEN_PIN );		
    } else {
      CHRG_LED_GREEN_PORT &= ~_BV( CHRG_LED_GREEN_PIN );		
    }
  }
#endif
}

void displayStatus() {
	
#ifdef SIMULATE
  /*	if (m_displayStatusCount<1000) {
    printf ("[%d:%d:%d] S=%x, TERM=%x, dock_c:%f, dock_v:%f, batt_c:%f, batt_v:%f, batt_t:%f\n",
    m_time.hour, m_time.min, m_time.sec, 
    m_chargeStatus,
    m_termination,
    getDockCurrentInC(),
    getDockVoltageInV(),
    getBatteryCurrent(),
    getBatteryVoltage(),
    getBattTemperatureInT()
    );
    m_displayStatusCount++;
    }*/
#endif // SIMULATE
	
  switch (m_chargeStatus) {
  case STATUS_FAST:
    setLED( GREEN, false);
    setLED( RED, true);
    break;
  case STATUS_TRICKLE:
    setLED( GREEN, true);
    setLED( RED, false);
    break;
  case STATUS_INIT:
    // init function will alternate their status
    break;
  case STATUS_NOT_DOCKED:
    setLED( GREEN, true );
    setLED( RED, true );
    break;
  case STATUS_ERROR:
    // do nothing - LEDS are blinked within the error() state function
    break;
  default:
    // we should never get here, so set an error
    m_chargeStatus= STATUS_ERROR;
    m_termination=TERM_SOFTWARE_ERROR3;
  }
}

void motherboardInit(bool p_buttonDepressed) {
#ifndef SIMULATE
  if ( p_buttonDepressed ) {
    CHRG_MB_TURN_ON_PORT |= _BV( CHRG_MB_TURN_ON_PIN );
  } else {
    CHRG_MB_TURN_ON_PORT &= ~_BV( CHRG_MB_TURN_ON_PIN );
  }
  CHRG_MB_TURN_ON_DDR |= _BV( CHRG_MB_TURN_ON_PIN );
#endif
}

void enableInterrupts() {
#ifndef SIMULATE
  sei();
#endif
}

u08 toggle=0;

// this executes F_CPU / 256 times per second (31250)
void inline timerTick() {
  //we slow down timer by factor of eight to allow for longer max timer
  toggle++;

  if ( (toggle & 7) == 0 ) {
    if ( m_rtcTimer ) m_rtcTimer--;
    if ( m_initTimer ) m_initTimer--;
    if ( m_ledTimer ) m_ledTimer--;
    if ( m_dataSendTimer) m_dataSendTimer--;
    if ( m_makeAdjustTimer ) m_makeAdjustTimer--;
    if ( m_mbTimer ) m_mbTimer--;
  }
}

#ifndef SIMULATE
// this executes F_CPU / 256 times per second (31250)
SIGNAL(SIG_OVERFLOW0) {
  timerTick();
}
#endif


/********************************** SIMULATION RELATED CODE FOLLOWS *******************************************************/

#ifdef SIMULATE
void convertPWMtoCurrent(u16 pwm) {
  // assumes 80% efficiency
  m_simulatedBattC = ((float) pwm) * (5.0/255.0) * 0.8;
}

// coulombCounter is called every second to simulate discharge/charge of battery
// This is part of the testing simulation. See also testCaseExecutor(), below.
void coulombCounterSimulate() {
			
  // calculate current energy in battery
  m_simulatedAmpHours += (m_simulatedBattC / (60*60)); // convert amps/sec to amps/hour

  // handle case of battery overcharging
  if (m_simulatedAmpHours > BATT_SIZE) {
    //printf("overcharging.\n");
    m_simulatedAmpHours = BATT_SIZE;
    m_simulatedBattT += 0.01 * m_simulatedBattC; // temp rises at 0.6 degrees/amp/min when overcharging
		
    // simulate voltage drop after charge saturation
    m_simulatedBattV -= m_simulatedBattC * 0.01666; // drop one volt per minute per amp
  }
	
  // handle case of battery undercharging - just make sure it doesn't go negative for now
  if (m_simulatedAmpHours < 0) {
    m_simulatedAmpHours = 0;
  }
	
  // handle ambient cooling
  m_simulatedBattT -= (m_simulatedBattT - AMBIENT_TEMPERATURE) * 0.0001; // this constant needs to be selected more carefully

  // handle battery voltage with respect to remaining charge
  // crude model - voltage drops linearly with respect to capacity once 1/4 of charge is left
  if (m_simulatedAmpHours < BATT_SIZE/4) {
    m_simulatedBattV = BATT_VOLTAGE - ((((BATT_SIZE/4) - m_simulatedAmpHours)/ (BATT_SIZE/4)) * BATT_VOLTAGE);
  }

  // handle battery voltage with respect to load
  // (not implement for now)
	
  /*if (m_chargeStatus * 256 + m_termination != m_displayStatusCount) {
    printf ("[%d:%d:%d] S=%x, TERM=%x, dock_c:%f, dock_v:%f, batt_c:%f, batt_v:%f, batt_t:%f, ampHours:%f\n",
	    m_time.hour, m_time.min, m_time.sec, 
	    m_chargeStatus,
	    m_termination,
	    getDockCurrent(),
	    getDockVoltage(),
	    getBatteryCurrent(),
	    getBatteryVoltage(),
	    getBatteryTemperatureInC(),
	    m_simulatedAmpHours
	    );
    m_displayStatusCount = m_chargeStatus * 256 + m_termination; // track state changes
  }*/
}

// Executes test cases
// Called by the main loop of main()
void testCaseExecutor() {
	
  // Each test case consists of a new load of starting parameters. 
#define NUM_TEST_CASES 14

  TestCaseData testCases[NUM_TEST_CASES] = {
    // 0: Check that the first state is the init state
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, 0, 0, BATT_SIZE/2, 0, STATUS_INIT,0},
	
    // 1: Start with typical setup, with robot undocked
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, 0, 0, BATT_SIZE/2, 0, STATUS_NOT_DOCKED,0},
	
    // 2: Dock robot    
	{BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/2, 0, STATUS_FAST,0},
	
	// 3: Set voltage less than before to be under the DV/DT threshold
	{BATT_VOLTAGE-0.5, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/2, TERM_DV_DT, STATUS_TRICKLE,0},
	
    // 4: Undock robot
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, 0, 0, BATT_SIZE/2, 0, STATUS_NOT_DOCKED,0},
	
    // 5: Re-dock robot. 
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/2, 0, STATUS_FAST,0},
	
    // 6: Over-temp condition
    {BATT_VOLTAGE, 0, MAX_TEMP_C_CHARGING_ERROR+1, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/2, TERM_MAX_TEMP, STATUS_ERROR,0},

    // 7: Under-temp condition
    {BATT_VOLTAGE, 0, MIN_TEMP_C_ERROR-1, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/2, TERM_MIN_TEMP, STATUS_ERROR,0},
	
    // 8: Over-voltage condition
    {BATT_MAX_VOLTAGE+1, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/2, TERM_MAX_VOLT, STATUS_ERROR,0},

    // 9: Charge time-out Part 1 -  Start with an empty battery. Should exit test in fast mode
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/3, 0, STATUS_FAST, 99},

    // 10: Charge time-out Part 2 -  re-empty battery. Should exit test in fast mode
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/3, 0, STATUS_FAST, 99},
	
    // 11: Charge time-out Part 3 -  re-empty battery. Should exit test due to time-out: TERM_MAX_TIME
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/3, TERM_MAX_TIME, STATUS_ERROR, 99},
	
    // 12: DT_DT Part 1 - Simulate by putting it in normal fast charging mode.
    // Wait 1 minute  so that the ambient_temperature is picked up as the last minute's ambient temp
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/3, 0, STATUS_FAST, 0},

    // 13: DT_DT Part 2 - Now raise temperature to simulat an full battery
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE+3, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/3, TERM_DT_DT, STATUS_TRICKLE,0}
		
  };
	
  // If both the termination status and the charge status match what is expected, then the test passes
  // and we move onto the next test case.
  if ((testCases[m_currentTestCase].expectedTermination == m_termination) && 
      (testCases[m_currentTestCase].expectedChargeStatus == m_chargeStatus)) {
    if (testCases[m_currentTestCase].waitInMinutes == 0  || (m_needToWait == m_time.min && m_needToWaitHours == m_time.hour)) {
      m_needToWait = 0;
      m_needToWaitHours = 0;
      printf("[%d:%d:%d] TEST CASE #%d HAS PASSED - Term: %i Status: %i\n", 
	     m_time.hour, m_time.min, m_time.sec, 
	     m_currentTestCase,
	     m_termination,
	     m_chargeStatus
	     );
      m_currentTestCase++;
      if (m_currentTestCase == NUM_TEST_CASES) {
	    printf("\n\nALL TEST CASES HAVE PASSED.\n");
		while(1);
	    exit(0);
      }
      printf ("Loading test case #%d.\n", m_currentTestCase);
      loadNextTestCase(&(testCases[m_currentTestCase]));
    }
  } else if (m_lastMinuteATestcaseFailed != m_time.min) {
    //Prints out an testcase failed maximum once every simulated minute unless in the init state 
	//(which might occur after a state that ends with an error) or if the wait timer is set.
	if (m_chargeStatus == STATUS_INIT) {
		printf("Still in the init state. Possible after the error in the last test.\n");
	} else if (testCases[m_currentTestCase].waitInMinutes == 0  || (m_needToWait == m_time.min && m_needToWaitHours == m_time.hour)) {
		printf("[%d:%d:%d] TEST CASE #%d HAS FAILED - Term: Exp %i, Act %i Status Exp %i, Act %i\n", 
			 m_time.hour, m_time.min, m_time.sec, 
			 m_currentTestCase,
			 testCases[m_currentTestCase].expectedTermination,
			 m_termination,
			 testCases[m_currentTestCase].expectedChargeStatus,
			 m_chargeStatus
			 );
	}
	m_lastMinuteATestcaseFailed = m_time.min;
  }
}

void loadNextTestCase(TestCaseData* currentTestCase) {
	
  if (m_chargeStatus == STATUS_ERROR) {
    printf ("IN ERROR STATE.\n");
    // reset if we're in the ERROR state, just so we can run other tests
    m_chargeStatus = STATUS_INIT;
    m_termination = 0;
  }
	
  if (currentTestCase->waitInMinutes != 0)
    {
      printf ("Non-zero wait=%d\n", currentTestCase->waitInMinutes);
      if (m_needToWait == 0 && m_needToWaitHours == 0) {
	m_needToWait = m_time.min + (60 * m_time.hour) + currentTestCase->waitInMinutes;
	while (m_needToWait >= 60) {
	  m_needToWait -= 60;
	  m_needToWaitHours++;
	}
	printf ("Delay Set: hour=%d, min=%d\n", m_needToWaitHours, m_needToWait);
      } 
    }
		
  m_simulatedBattV=currentTestCase->simulatedBattV;
  m_simulatedBattC=currentTestCase->simulatedBattC; 
  m_simulatedBattT=currentTestCase->simulatedBattT;
  m_simulatedDockV=currentTestCase->simulatedDockV;
  m_simulatedDockC=currentTestCase->simulatedDockC;
  m_simulatedAmpHours=currentTestCase->simulatedAmpHours;

}
#endif // SIMULATE
