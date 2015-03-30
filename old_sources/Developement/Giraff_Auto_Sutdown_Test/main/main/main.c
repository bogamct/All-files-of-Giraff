/*
 * main.c
 * 
 * Created: 2011-11-08 11:17:15
 *  Author: Patrik
 *
 */ 

#include <avr/io.h>
#include <stdbool.h> 
#include "clock.h"

#define MINUTES_BETWEEN_REBOOTS 4

int main(void)
{
	
	//PD0 output for On/Off signal
	DDRD = 0xFF;
	
	//Start in Off
	set_Giraff_Switch_State(false);
	
	//Init the clock
	clock_Init();
	
	//wait 15 seconds until starting first time
	while (get_Seconds() < 15);
	set_Giraff_Switch_State(true);
	
    while(1)
    {
		while ((get_Minute() % MINUTES_BETWEEN_REBOOTS) == 0)
		{
			if (get_Seconds() == 1)
			{
				set_Giraff_Switch_State(false);
				while (get_Seconds() < 30);
				set_Giraff_Switch_State(true);
			}
		}			
		
    }
}

void set_Giraff_Switch_State (bool On)
{
	if(On)
	{
		PORTD = 0xFF;
	} else {
		PORTD = 0x00;
	}
}