/*
 * clock.h
 *
 * Created: 2011-09-25 19:09:40
 *  Author: Patrik
 */ 

#include <stdint.h>
#include <stdbool.h>

#ifndef CLOCK_H_
#define CLOCK_H_
		
	uint16_t get_Milliseconds ( void );
	uint8_t get_Seconds ( void );
	uint8_t get_Minute ( void );
	uint8_t get_Hour ( void );
	void clock_Init( void );
	void set_Alarm_Status( bool on );
	bool get_Alarm_Status( void );

#endif /* CLOCK_H_ */