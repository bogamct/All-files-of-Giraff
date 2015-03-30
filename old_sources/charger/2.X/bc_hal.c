#ifndef SIMULATE
#include <util/delay.h>
#endif //SIMULATE

#include "../../common/robotSpecificSettings.h"
#include "bc_hal.h"
#include "bc.h"
#include "stdio.h"
#include "stdlib.h"
#include "a2d.h"
#include "charger_status.h"
#include "batt_info.h"

#ifdef SIMULATE
u16 m_simulatedPWM = 0; // simulates PWM register
float m_simulatedBattV=24.0;
float m_simulatedBattC=0.0; 
float m_simulatedBattT=20.0;
float m_simulatedDockV=32.0;
float m_simulatedDockC=0.0;
float m_simulatedAmpHours= BATT_SIZE - 0.5; // default to almost fully charged
u16 m_displayStatusCount = 0;
s16 m_currentTestCase = -1;
bool m_loadNewTestCase = true;
u16 m_needToWait = 0;
u16 m_needToWaitHours = 0;
u16 m_lastMinuteATestcaseFailed = -1;
u16 m_numberOfFailedTestCases = 0;
u16 m_numberOfTimesInUnexpectedErrorState = 0;
#endif

//This delay determines how fast the low battery indicator blinks
#define LOW_BATT_BLINK_DELAY 5
u08 m_currentLowBattBlink = 0;

void rtcInit() {
  m_time.sec = 0x00;
  m_time.min = 0x00;
  m_time.hour = 0x00;
}

void ledsInit() {
#ifndef SIMULATE
  //INternal red LED
  CHRG_INTERNAL_RED_LED_DDR |= _BV( CHRG_INTERNAL_RED_LED_PIN ) ;
  CHRG_INTERNAL_RED_LED_PORT |= _BV( CHRG_INTERNAL_RED_LED_PIN );
  
  //Internal green led
  CHRG_INTERNAL_GREEN_LED_DDR |= _BV( CHRG_INTERNAL_GREEN_LED_PIN ) ;
  CHRG_INTERNAL_GREEN_LED_PORT &= ~_BV( CHRG_INTERNAL_GREEN_LED_PIN );
  
  //External giraff on/off indicator led
  CHRG_EXTERNAL_ONOFF_LED_DDR |= _BV( CHRG_EXTERNAL_ONOFF_LED_PIN ) ;
  CHRG_EXTERNAL_ONOFF_LED_PORT &= ~_BV( CHRG_EXTERNAL_ONOFF_LED_PIN );

  //External giraff charging indicator led
  CHRG_EXTERNAL_CHARGING_LED_DDR |= _BV( CHRG_EXTERNAL_CHARGING_LED_PIN ) ;
  CHRG_EXTERNAL_CHARGING_LED_PORT &= ~_BV( CHRG_EXTERNAL_CHARGING_LED_PIN );

  //External giraff battery level indicator led, red.
  CHRG_EXTERNAL_BAT_LVL_RED_LED_DDR |= _BV( CHRG_EXTERNAL_BAT_LVL_RED_LED_PIN ) ;
  CHRG_EXTERNAL_BAT_LVL_RED_LED_PORT &= ~_BV( CHRG_EXTERNAL_BAT_LVL_RED_LED_PIN );  

  //External giraff battery level indicator led, green.
  CHRG_EXTERNAL_BAT_LVL_GREEN_LED_DDR |= _BV( CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PIN ) ;
  CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PORT &= ~_BV( CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PIN );    
#endif
}

/*void pllInit() {
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
}*/

void inline pwmInitPin() {
#ifndef SIMULATE
  DDRD |= _BV( PD4 ); //Set PD4 as output
  PORTD &= ~_BV( PD4 ); //Set to 0 to not affect anything
#endif
}

void inline setPWMMode( bool p_on ) {
#ifndef SIMULATE
  if ( p_on ) {
    TCCR1A |= _BV( COM1B1 ); //Clear OC1B on Compare Match.
    //TCCR1C |= _BV( COM1B1S );  
  } else {
    TCCR1A &= ~_BV( COM1B1 ); //Normal port operation
    //TCCR1C &= ~_BV( COM1B1S );
  }
#endif
}

void pwmInit() {
#ifdef SIMULATE
#else
  TCCR1A = _BV( WGM13 ) | _BV( WGM12 ) | _BV( WGM11 ) | _BV( WGM10 ); //Fast PWM mode, OCR1A as TOP value
  TCCR1B = _BV( CS10 ); //No clock prescaling

  setPWMMode( true );
  // Start the counter at zero
  TCNT1 = 0; //This 16-bit register contains the value of Timer/Counter1

  // Start PWM at 50% duty cycle  
  OCR1B = 255; //Compare match register value
  OCR1A = 511; //Counter TOP value
  
  pwmInitPin();
#endif 
}

// Function that adjusts PWM width
void setPWM(u16 p_pwm) {
#ifdef SIMULATE
  m_simulatedPWM = p_pwm;
  convertPWMtoCurrent(p_pwm);
#else
  OCR1B = p_pwm;
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
  return OCR1B;
#endif // SIMULATE	
}

void timerInit() {
#ifdef SIMULATE
#else
  TCCR0A = 0; //Normal port operations, internal clock
  TCCR0B = _BV( CS00 ); //No clock prescaling
  TCNT0 =0; //8-bit counter set to 0
  OCR0A = 0;
  OCR0B = 0;

  timer0Enable();
#endif
}

bool getLED(int p_led) {
#ifndef SIMULATE
//Note that the internal LEDs are reversed from the external LEDs
  if ( p_led==INTERNAL_RED ) {
    return (CHRG_INTERNAL_RED_LED_PORT & _BV( CHRG_INTERNAL_RED_LED_PIN ) ) == _BV ( CHRG_INTERNAL_RED_LED_PIN );
  } else if ( p_led==INTERNAL_GREEN ){
    return (CHRG_INTERNAL_GREEN_LED_PORT & _BV( CHRG_INTERNAL_GREEN_LED_PIN ) ) == _BV( CHRG_INTERNAL_GREEN_LED_PIN );
  } else if ( p_led==ONOFF_LED ){
    return !(CHRG_EXTERNAL_ONOFF_LED_PORT & _BV( CHRG_EXTERNAL_ONOFF_LED_PIN ) ) == _BV( CHRG_EXTERNAL_ONOFF_LED_PIN );
  } else if ( p_led==CHARGING_LED ){
    return !(CHRG_EXTERNAL_CHARGING_LED_PORT & _BV( CHRG_EXTERNAL_CHARGING_LED_PIN ) ) == _BV( CHRG_EXTERNAL_CHARGING_LED_PIN );
  } else if ( p_led==BAT_LVL_RED ){
    return !(CHRG_EXTERNAL_BAT_LVL_RED_LED_PORT & _BV( CHRG_EXTERNAL_BAT_LVL_RED_LED_PIN ) ) == _BV( CHRG_EXTERNAL_BAT_LVL_RED_LED_PIN );
  } else if ( p_led==BAT_LVL_GREEN ){
    return !(CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PORT & _BV( CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PIN ) ) == _BV( CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PIN );
  } else {
    //Should never end up here so set an error
    m_chargeStatus= STATUS_ERROR;
    m_termination=TERM_SOFTWARE_ERROR6;
    return false;
  }
#else
  return false;
#endif
}

void setLED(int p_led, bool p_on) {
#ifndef SIMULATE
//Note that the internal LEDs are reversed from the external LEDs
  if ( p_led==INTERNAL_RED ) {
    if ( p_on ) {
      CHRG_INTERNAL_RED_LED_PORT |= _BV( CHRG_INTERNAL_RED_LED_PIN );
    } else {
      CHRG_INTERNAL_RED_LED_PORT &= ~_BV( CHRG_INTERNAL_RED_LED_PIN );
    }
  } else if ( p_led==INTERNAL_GREEN ) {
    if ( p_on ) {
      CHRG_INTERNAL_GREEN_LED_PORT |= _BV( CHRG_INTERNAL_GREEN_LED_PIN );
    } else {
      CHRG_INTERNAL_GREEN_LED_PORT &= ~_BV( CHRG_INTERNAL_GREEN_LED_PIN );
    }
  } else if ( p_led==ONOFF_LED ) {
    if ( p_on ) {
      CHRG_EXTERNAL_ONOFF_LED_PORT &= ~_BV( CHRG_EXTERNAL_ONOFF_LED_PIN );
    } else {
      CHRG_EXTERNAL_ONOFF_LED_PORT |= _BV( CHRG_EXTERNAL_ONOFF_LED_PIN );
    }
  } else if ( p_led==CHARGING_LED ) {
    if ( p_on ) {
      CHRG_EXTERNAL_CHARGING_LED_PORT &= ~_BV( CHRG_EXTERNAL_CHARGING_LED_PIN );
    } else {
      CHRG_EXTERNAL_CHARGING_LED_PORT |= _BV( CHRG_EXTERNAL_CHARGING_LED_PIN );
    }
  } else if ( p_led==BAT_LVL_RED ) {
    if ( p_on ) {
      CHRG_EXTERNAL_BAT_LVL_RED_LED_PORT &= ~_BV( CHRG_EXTERNAL_BAT_LVL_RED_LED_PIN );
    } else {
      CHRG_EXTERNAL_BAT_LVL_RED_LED_PORT |= _BV( CHRG_EXTERNAL_BAT_LVL_RED_LED_PIN );
    }
  } else if ( p_led==BAT_LVL_GREEN ) {
    if ( p_on ) {
      CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PORT &= ~_BV( CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PIN );
    } else {
      CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PORT |= _BV( CHRG_EXTERNAL_BAT_LVL_GREEN_LED_PIN );
    }
  }
#endif
}

void displayStatus(float p_wattSeconds) {
	
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
    setLED( INTERNAL_GREEN, false);
    setLED( INTERNAL_RED, true);
    setLED( CHARGING_LED, true); //Turn on charging indicator
    break;
  case STATUS_TRICKLE:
    setLED( INTERNAL_GREEN, true);
    setLED( INTERNAL_RED, false);
    setLED( CHARGING_LED, true); //Turn on charging indicator
    break;
  case STATUS_INIT:
    // init function will alternate their status
    break;
  case STATUS_NOT_DOCKED:
    setLED( INTERNAL_GREEN, true );
    setLED( INTERNAL_RED, true );
    setLED( CHARGING_LED, false); //Turn off charging indicator
    break;
  case STATUS_ERROR:
    // do nothing with internal LEDs - LEDS are blinked within the error() state function
    setLED( CHARGING_LED, false); //Turn off charging indicator since we are not charging
    break;
  default:
    // we should never get here, so set an error
    m_chargeStatus= STATUS_ERROR;
    m_termination=TERM_SOFTWARE_ERROR3;
    setLED( CHARGING_LED, false); //Turn off charging indicator since we are not charging
  }
  
  //The Giraff is defined as on if the motherboard is on
  if ( isMotherboardOn() ) {
    setLED( ONOFF_LED, true);
  } else {
    setLED( ONOFF_LED, false);
  }
  
  //Battery indicator has 3 levels, represents green, yellow, red.
  if ( p_wattSeconds > BATT_PRE_LOW_WARNING_WATT_SECONDS ) {
    //Green
    setLED( BAT_LVL_GREEN, true);
    setLED( BAT_LVL_RED, false);
  } else if ( p_wattSeconds > BATT_LOW_WARNING_WATT_SECONDS ) {
    //Yellow
    setLED( BAT_LVL_GREEN, true);
    setLED( BAT_LVL_RED, true);
  } else {
    //Red blinking
    if ( m_currentLowBattBlink >= LOW_BATT_BLINK_DELAY ) {
      setLED( BAT_LVL_GREEN, false);
      setLED( BAT_LVL_RED, !getLED( BAT_LVL_RED ));
      m_currentLowBattBlink = 0;
    } else {
      m_currentLowBattBlink++;
    }
  }
}

//void motherboardInit(bool p_buttonDepressed) {
void motherboardInit() {
#ifndef SIMULATE
  /*if ( p_buttonDepressed ) {
    CHRG_MB_ONOFF_SWITCH_PORT |= _BV( CHRG_MB_ONOFF_SWITCH_PIN );
  } else {
    CHRG_MB_ONOFF_SWITCH_PORT &= ~_BV( CHRG_MB_ONOFF_SWITCH_PIN );
  }*/
  
  //MB ON/OFF pin as output. High means button is pressed.
  CHRG_MB_ONOFF_SWITCH_DDR |= _BV( CHRG_MB_ONOFF_SWITCH_PIN );
  
  //Initial value as not pressed.
  CHRG_MB_ONOFF_SWITCH_PORT &= !_BV( CHRG_MB_ONOFF_SWITCH_PIN );
  
  //MB ON/OFF indicator pin as input. Low means that MB is on.
  CHRG_MB_ONOFF_INDICATOR_DDR |= _BV( CHRG_MB_ONOFF_INDICATOR_PIN );
  
  //Activate pull-up on MB ON/OFF indicator pin
  CHRG_MB_ONOFF_INDICATOR_PORT |= _BV( CHRG_MB_ONOFF_INDICATOR_PIN );
  
  //Giraff ON/OFF button pin as input. High means that button is pressed.
  CHRG_GIRAFF_ONOFF_BUTTON_DDR |= _BV( CHRG_GIRAFF_ONOFF_BUTTON_PIN );
  
  //Enable INT0 (Giraff ON/OFF button), trigger at any edge and activate the interrupt
  EICRA = _BV( ISC00 );
  setGiraffOnOffButtonInterrupt(true);
  
  //INT1 (MB ON/OFF indicator), trigger at rising edge when motherboard turns off
  //Start with interrupt disabled
  EICRA |= _BV( ISC11 ) | _BV( ISC10 );
  setMotherboardOnOffIndicatorInterrupt(false);
   
  //Giraff main power controll pin. High mean that power is on.
  CHRG_MAIN_POWER_ONOFF_DDR |= _BV( CHRG_MAIN_POWER_ONOFF_PIN );

#endif
}

bool isMotherboardOn() {
#ifndef SIMULATE
  return (!CHRG_MB_ONOFF_INDICATOR_PORT & _BV( CHRG_MB_ONOFF_INDICATOR_PIN ) ) == _BV ( CHRG_MB_ONOFF_INDICATOR_PIN ); 
#else
  return false;
#endif
}

bool isGiraffOnOffButtonPressed () {
#ifndef SIMULATE
  return (CHRG_GIRAFF_ONOFF_BUTTON_PORT & _BV( CHRG_GIRAFF_ONOFF_BUTTON_PIN ) ) == _BV ( CHRG_GIRAFF_ONOFF_BUTTON_PIN );
#else
  return false;
#endif
}

void setGiraffOnOffButtonInterrupt(bool activate) {
#ifndef SIMULATE  
  if ( activate ) {
    EIMSK |= _BV( INT0 );
  } else {
    EIMSK &= !_BV( INT0 );
  }
#endif
}

void setMotherboardOnOffIndicatorInterrupt(bool activate) {
#ifndef SIMULATE
  if ( activate ) {
    EIMSK |= _BV( INT1 );
  } else {
    EIMSK &= !_BV( INT1 );
  }
#endif
}

void setMainPower (bool activate) {
#ifndef SIMULATE
  if ( activate ) {
    CHRG_MAIN_POWER_ONOFF_PORT |= _BV( CHRG_MAIN_POWER_ONOFF_PIN );
  } else {
    CHRG_MAIN_POWER_ONOFF_PORT &= !_BV( CHRG_MAIN_POWER_ONOFF_PIN );
  }
#endif
}

void setMotherboardOnOfSwitchPin (bool setToHigh) {
#ifndef SIMULATE
  //SetToHigh = true means the same as pressing the button on a normal computer
  if ( setToHigh ) {
    CHRG_MB_ONOFF_SWITCH_PORT |= _BV( CHRG_MB_ONOFF_SWITCH_PIN );
  } else {
    CHRG_MB_ONOFF_SWITCH_PORT &= !_BV( CHRG_MB_ONOFF_SWITCH_PIN );
  }
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
    if ( m_giraffOnOffButtonDebounceTimer ) m_giraffOnOffButtonDebounceTimer--;
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
void testCaseExecutor( u16 p_counter ) {
	
  // Each test case consists of a new load of starting parameters. 
#define NUM_TEST_CASES 14

  TestCaseData testCases[NUM_TEST_CASES] = {
    // 0: Check that the first state is the init state
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, 0, 0, BATT_SIZE/2, 0, STATUS_INIT,0},
	
    // 1: Start with typical setup, with robot undocked, do this after waiting 2s to get out of the init state
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE, 0, 0, BATT_SIZE/2, 0, STATUS_NOT_DOCKED,2},
	
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
    {BATT_VOLTAGE, 0, AMBIENT_TEMPERATURE+3, TYPICAL_DOCK_VOLTAGE, 1.5, BATT_SIZE/3, TERM_DT_DT, STATUS_TRICKLE,1}
		
  };
	
	if ( m_loadNewTestCase ) {
	  //Load a new test case
	  m_currentTestCase++;
      if (m_currentTestCase == NUM_TEST_CASES) {
	    printf("\n\nALL TESTS COMPLETE WITH THE FOLLOWING RESULT:\n");
		printf("Number of failed test cases: %d.\n",m_numberOfFailedTestCases);
		printf("Number of times in error state: %d.\n",m_numberOfTimesInUnexpectedErrorState);
		while(1);
	    exit(0);
      }
      printf ("[%d:%d:%d:%d] Loading test case #%d.\n", 
	  m_time.hour, m_time.min, m_time.sec, p_counter,
	  m_currentTestCase);
	  if (m_chargeStatus == STATUS_ERROR) {
	    if (m_currentTestCase > 0) {
			if (testCases[m_currentTestCase - 1].expectedChargeStatus != STATUS_ERROR) {
				//Unexpected error state
				printf ("IN UNEXPECTED ERROR STATE.\n");
				m_numberOfTimesInUnexpectedErrorState++;
			}
		}
		// reset if we're in the ERROR state, just so we can run other tests
		m_chargeStatus = STATUS_INIT;
		m_termination = 0;
	  }
      loadNextTestCase(&(testCases[m_currentTestCase]));
	  m_loadNewTestCase = false;
	} else {
	  // If both the termination status and the charge status match what is expected, then the test passes
	  // and we move onto the next test case.
	  if ((testCases[m_currentTestCase].expectedTermination == m_termination) && 
		  (testCases[m_currentTestCase].expectedChargeStatus == m_chargeStatus)) {
		if (testCases[m_currentTestCase].waitInMinutes == 0  || (m_needToWait == m_time.min && m_needToWaitHours == m_time.hour)) {
		  m_needToWait = 0;
		  m_needToWaitHours = 0;
		  printf("[%d:%d:%d:%d] TEST CASE #%d HAS PASSED - Term: %i Status: %i\n", 
			 m_time.hour, m_time.min, m_time.sec, p_counter,
			 m_currentTestCase,
			 m_termination, 
			 m_chargeStatus
			 );
			m_loadNewTestCase = true;
		}
	  } else if (m_lastMinuteATestcaseFailed != m_time.min) {
		//Prints out an testcase failed maximum once every simulated minute unless in the init state 
		//(which might occur after a state that ends with an error) or if the wait timer is set.
		/*if (m_chargeStatus == STATUS_INIT ) {
			printf("[%d:%d:%d:%d] Still in the init state. Possible after the error in the last test.\n", 
				 m_time.hour, m_time.min, m_time.sec, p_counter);
		} else if (testCases[m_currentTestCase].waitInMinutes == 0  || (m_needToWait == m_time.min && m_needToWaitHours == m_time.hour)) {*/
	  if ( testCases[m_currentTestCase].waitInMinutes == 0  || (m_needToWait == m_time.min && m_needToWaitHours == m_time.hour) )
	    if ( m_chargeStatus == STATUS_INIT ) {
		  printf("[%d:%d:%d:%d] Still in the init state. Possible after the error in the last test.\n", 
				 m_time.hour, m_time.min, m_time.sec, p_counter);
		} else {
			printf("[%d:%d:%d:%d] TEST CASE #%d HAS FAILED - Term: Exp %i, Act %i Status Exp %i, Act %i\n", 
				 m_time.hour, m_time.min, m_time.sec, p_counter, 
				 m_currentTestCase,
				 testCases[m_currentTestCase].expectedTermination,
				 m_termination,
				 testCases[m_currentTestCase].expectedChargeStatus,
				 m_chargeStatus
				 );
			m_numberOfFailedTestCases++;
		}
		m_lastMinuteATestcaseFailed = m_time.min;
	  }
	}
}

void loadNextTestCase(TestCaseData* currentTestCase) {
	
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
