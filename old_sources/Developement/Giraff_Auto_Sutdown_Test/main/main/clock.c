/*
 * clock.c
 *
 * Created: 2011-09-25 19:09:53
 *  Author: Patrik
 */ 

#include <avr\interrupt.h>
#include "clock.h"

void update_Clock( void );
void update_Siren( void );

#define COUNTER_PRESCALER 1024;

/* Define a time struct for the clock */
typedef struct 
{
	uint16_t milliseconds;
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
} time;
	 
/* Define a clock */
time clock = {0,0,0,0};
	
/*Define alarm state*/
bool alarm_On;
		
void clock_Init ( void )
{
	//Set counter divider clkio/1024
	TCCR0 |= ( (1<<CS02) | (1<<CS00) );
	
	//Set compare register to generate interrupt after 0.256seconds
	OCR0 = 250;
	
	//Enable compare interrupt
	TIMSK |= ( (1<<OCIE0) );
	
	//enable interrupt
	sei();	
	
	//Set siren pin to be output pin on PD7
	DDRD |= (1<<PD7); 
}

uint16_t get_Milliseconds ( void )
{
	return clock.milliseconds;
}

uint8_t get_Seconds ( void ) 
{
	return clock.second;
}	

uint8_t get_Minute ( void ) 
{
	return clock.minute;	
}	

uint8_t get_Hour ( void ) 
{
	return clock.hour;	
}

void set_Alarm_Status( bool on )
{
	alarm_On = on;
}

bool get_Alarm_Status( void )
{	
	return alarm_On;
}

void update_Clock( void )
{
	clock.milliseconds += 256; //(OCR0 / (F_CPU / COUNTER_PRESCALER)) => 250 / ( 1M / 1024) = 0.256 seconds
    if (clock.milliseconds >= 1000)
	{
		clock.milliseconds = clock.milliseconds % 1000;
		clock.second += 1;
		if (clock.second >= 60)
		{
			clock.second = 0;
			clock.minute += 1;
			if (clock.minute >= 60)
			{
				clock.minute = 0;
				clock.hour += 1;
			}
		}
	}	
}

void update_Siren( void )
{
	if (get_Alarm_Status())
	{
		if (get_Seconds() % 2)
		{
			PORTD |= (1<<PD7);
		} else
		{
			PORTD &= (0<<PD7);
		}	
	} else
	{
		PORTD |= (1<<PD7);
	}
}

ISR(TIMER0_COMP_vect)
{
	update_Clock();
	update_Siren();		
}

