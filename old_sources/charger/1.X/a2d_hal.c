#ifndef SIMULATE
#include <avr/pgmspace.h>
#endif // SIMULATE

#include "bc.h"
#include "a2d_hal.h"
#include "stdio.h"
#include "chargerInfo.h"
#include "charger_status.h"

#define KELVIN_C_DIFF 273.15
#define SIZE_OF_TEMP_LOOKUP_TABLE 61

#ifdef SIMULATE
#define PROGMEM
#endif

#ifdef SIMULATE
void testAccuracy (float accurateValue, float inputValue, float accuracy);
#endif

extern u08 m_termination;

//Function that converts resistance into temperature in Celcius for thermistor MTG1-410F103G
float resistanceToC( float p_resistance ) {
//  y = -4.29677508*10-5*x5  + 4.019937396*10-3*x4
//      -1.404371158*10-1*x3 + 2.319640073*x2
//      -19.90774606*x       + 96.88182507
  float x2 = p_resistance*p_resistance;
  float x3 = x2 * p_resistance;

  doImportantStuff();
  
  float x4 = x3 * p_resistance;
  float x5 = x4 * p_resistance;
  float tempInC =  -4.29677508e-5*x5  + 4.019937396e-3*x4 -1.404371158e-1*x3 + 2.319640073*x2 -19.90774606*p_resistance + 96.88182507;
  return tempInC;
}

#ifdef SIMULATE
void testResistanceToC() {
  #define funcAccuracy 2.0
	
  printf( "Testing resistance to degrees Celsius function(testResistanceToC() in a2d_hal.c)");
 
  testAccuracy( 0, resistanceToC( 34.336 ), funcAccuracy);
  testAccuracy( 5, resistanceToC( 26.387 ), funcAccuracy);
  testAccuracy( 10, resistanceToC( 20.474 ), funcAccuracy);
  testAccuracy( 15, resistanceToC( 16.011 ), funcAccuracy);
  testAccuracy( 20, resistanceToC( 12.608 ), funcAccuracy);
  testAccuracy( 25, resistanceToC( 10.000 ), funcAccuracy);
  testAccuracy( 30, resistanceToC( 7.985 ), funcAccuracy);
  testAccuracy( 35, resistanceToC( 6.417 ), funcAccuracy);
  testAccuracy( 40, resistanceToC( 5.187 ), funcAccuracy);
  testAccuracy( 45, resistanceToC( 4.220 ), funcAccuracy);
  testAccuracy( 50, resistanceToC( 3.451 ), funcAccuracy);
  testAccuracy( 55, resistanceToC( 2.838 ), funcAccuracy);
  testAccuracy( 60, resistanceToC( 2.345 ), funcAccuracy);
  testAccuracy( 65, resistanceToC( 1.947 ), funcAccuracy);
  testAccuracy( 70, resistanceToC( 1.626 ), funcAccuracy);
  testAccuracy( 75, resistanceToC( 1.346 ), funcAccuracy);
  
  printf( " - Unit test completed\n\n" );

}

void testAccuracy (float accurateValue, float inputValue, float accuracy) {
  if ( (inputValue > (accurateValue + accuracy)) || (inputValue < (accurateValue - accuracy)) ) {
	printf("Error - Accuracy test not passed with following parameters:\n");
    printf( "Accurate value = %f\n", accurateValue );
    printf( "Function value = %f\n", inputValue );
    printf( "Accuracy = %f\n", accuracy );
  }
}
#endif

float a2dToVoltage( u16 p_num ) {
  float voltage = p_num * ((VREF/1024) * ((RESISTOR1+RESISTOR2)/RESISTOR1));
  return voltage;
}

float a2dToCurrent( u16 p_num ) {
  s16 num = (s16) p_num;
  // if num is negative, using two's complement, convert it to s16 format
  if ((num & (1<<9))!= 0) {    
    num -= (1<<10);
  }
  
  float current = num * ((VREF*(RESISTOR3+RESISTOR4))/(512*DIFF_AMPLIFIER_GAIN*RESISTANCE_CURRENT_SENSE*RESISTOR4));
  return current;
}

float a2dToTemperatureC( u16 p_num ) {
  doImportantStuff();

  // resistance of non-NTC resistor in voltage divider
  float r0=121.0;
  // the voltage that is actually on the pin
  float voltage = p_num * (VREF/1024);
  float vcc=5.0f;

  doImportantStuff();

  float resistance  = voltage*r0;
  resistance /= (vcc-voltage);

  doImportantStuff();

  float temperature = resistanceToC( resistance );
  return temperature;
}

void a2dStartConversion() {
#ifndef SIMULATE
  ADCSRA |= (1<<ADSC);
#endif
}

u16 a2dGetValue() {
#ifndef SIMULATE
  return ADC;
#else
  return 0;
#endif
}

bool a2dConversionFinished() {
#ifdef SIMULATE
  return true;
#else
  return (ADCSRA & (1<<ADSC)) == 0 ;
#endif
}

void a2dInit() {
#ifdef SIMULATE
#else
  // make sure A2D inputs are set up as inputs, with the pull-up resistor turned off
  DDRA  &= ~ (_BV(PA0) | _BV(PA1) | _BV(PA2) | _BV(PA4) | _BV(PA5)); 
  PORTA &= ~ (_BV(PA0) | _BV(PA1) | _BV(PA2) | _BV(PA4) | _BV(PA5));
  DDRB  &= ~ (_BV(PB4) | _BV(PB5));
  PORTB &= ~ (_BV(PB4) | _BV(PB5));

  ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
  ADCSRB = (1<<BIN);
  ADMUX = (1<<REFS1);  
  DIDR0 = 1+2+4+8+16+32;
  DIDR1 = 16+32;
#endif
}

void a2dChooseChannel( u08 p_adcCommand ) {
#ifdef SIMULATE
#else
  switch ( p_adcCommand ) {
  case CMD_DOCK_CURRENT:
    ADMUX=(ADMUX & (128+64+32)) | (16+2);
    break;
  case CMD_DOCK_VOLTAGE:
    ADMUX=(ADMUX & (128+64+32)) | 8;
    break;
  case CMD_BATT_CURRENT:
    ADMUX=(ADMUX & (128+64+32)) | (8+2+1);
    break;
  case CMD_BATT_VOLTAGE:
    ADMUX=(ADMUX & (128+64+32)) | 7;
    break;
  case CMD_BATT_TEMPERATURE:
    ADMUX=(ADMUX & (128+64+32)) | 2;
    break;
  case CMD_OFFSET_ADJUSTMENT:
    ADMUX=(ADMUX & (128+64+32)) | 13;
    break;
  default:
    m_termination = TERM_SOFTWARE_ERROR5;
  } 
#endif
}
