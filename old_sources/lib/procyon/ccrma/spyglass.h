/*! \file spyglass.h \brief Spyglass Control Panel UI Board Driver. */
//*****************************************************************************
//
// File Name	: 'spyglass.h'
// Title		: Spyglass Control Panel UI Board Driver
// Author		: Pascal Stang - Copyright (C) 2005
// Created		: 7/20/2005
// Revised		: 7/23/2005
// Version		: 0.9
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
//  SPYGLASS PINOUT for 10-pin I/O header:
//
//			    ----
//		+5V - 1 |++| 2 - +5V
//		SCL - 3 |++| 4 - GND
//		SDA - 5 |++| 6 - GND
//		      7 |++| 8 - BEEPER PWR (+5V)
//		      9 |++| 10
//		        ----
//
//  CONNECTIONS:
//		Connect power to the +5V and ground pins.  And connect the SCL/SDA
//	pins to the corresponding SDA/SCL pins of your processor's I2C bus.
//
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef SPYGLASS_H
#define SPYGLASS_H

// defines
// Base I2C node addresses
#define PCF8574_I2C_BASE_ADDR	0x40
#define MAX517_I2C_BASE_ADDR	0x58

// PCF8574 node addresses
#define PCF_NODE_BUTTONS		0x00
#define PCF_NODE_LCD_DATA		0x01
#define PCF_NODE_CONTROL		0x02

// Control node bit-defines
#define SPYGLASS_LED0		0x01
#define SPYGLASS_LED1		0x02
#define SPYGLASS_BEEPER		0x04
#define SPYGLASS_LCD_RS		0x20
#define SPYGLASS_LCD_RW		0x40
#define SPYGLASS_LCD_E		0x80


// functions
//! initialize the I2C bus for communication with the spyglass UI.
void spyglassInit(void);

// *********** MISC INPUT/OUTPUT commands ***********

//! Read the state of pushbuttons on the spyglass UI
/// Returns an 8-bit number representing the state of buttons S1-S7 in bits 0-6 respectively.
///  - '0' bit value = button not pressed
///  - '1' bit value = button is pressed
u08 spyglassGetPushbuttons(void);

//! Sets the state of LEDs on the spyglass UI
/// Argument is a 2-bit number representing the desired state of LEDs D2 & D3 respectively.
///  - '0' bit value = LED off
///  - '1' bit value = LED on
void spyglassSetLeds(u08 leds);

//! Sets the state of the beeper on the spyglass UI
/// - '0' state => beeper off
/// - '1' state => beeper on
void spyglassSetBeeper(u08 state);

u08 spyglassSetLcdContrast(u08 contrast);

// *********** LCD commands ***********

//! Initialize LCD for operation
void spyglassLcdInit(void);
//! Set write/cursor position to upper left. 
void spyglassLcdHome(void);
//! Clear LCD display.
void spyglassLcdClear(void);
//! Set write/cursor posision on LCD display (x=col, y=line).
void spyglassLcdGotoXY(u08 x, u08 y);
//! Write control or display data to LCD.
void spyglassLcdWrite(u08 rs, u08 data);
//! Write character to LCD.
void spyglassLcdWriteChar(u08 c);

// *********** LOW-LEVEL ACCESS ***********

//! Write I/O Data to PCF8574 I2C<->Digital I/O chip.
u08 pcf8574Write(u08 nodeAddr, u08 data);
//! Read I/O Data from PCF8574 I2C<->Digital I/O chip.
u08 pcf8574Read(u08 nodeAddr);

#endif
