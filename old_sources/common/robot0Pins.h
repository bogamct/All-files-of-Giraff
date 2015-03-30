#ifndef ROBOT0_PINS_H
#define ROBOT0_PINS_H

#define MC_SLEEP 1

#define MC_LED0_DDR DDRC
#define MC_LED0_PIN PC7
#define MC_LED0_PORT PORTC

#define NUM_ENCODERS 3

// -------------------- Encoder 0 (driver left) connections --------------------
// Phase A quadrature encoder output should connect to this interrupt line:
// *** NOTE: the choice of interrupt PORT, DDR, and PIN must match the external
// interrupt you are using on your processor.  Consult the External Interrupts
// section of your processor's datasheet for more information.

// Interrupt Configuration
#define ENC0_SIGNAL		  SIG_INTERRUPT1    // Interrupt signal name
#define ENC0_INT		  INT1	            // matching INTx bit in GIMSK/EIMSK
#define ENC0_ICR		  EICRA	        // matching Int. Config Register (MCUCR,EICRA/B)
#define ENC0_ISCX0		  ISC10	        // matching Interrupt Sense Config bit0
#define ENC0_ISCX1		  ISC11	        // matching Interrupt Sense Config bit1
// PhaseA Port/Pin Configuration
// *** PORTx, DDRx, PINx, and Pxn should all have the same letter for "x" ***
#define ENC0_PHASEA_PORT	  PORTD	        // PhaseA port register
#define ENC0_PHASEA_DDR		  DDRD	        // PhaseA port direction register
#define ENC0_PHASEA_PORTIN	  PIND	        // PhaseA port input register
#define ENC0_PHASEA_PIN		  PD3	        // PhaseA port pin

// Phase B quadrature encoder output should connect to this direction line:
// *** PORTx, DDRx, PINx, and Pxn should all have the same letter for "x" ***
#define ENC0_PHASEB_PORT	  PORTC	        // PhaseB port register
#define ENC0_PHASEB_DDR		  DDRC	        // PhaseB port direction register
#define ENC0_PHASEB_PORTIN	  PINC	        // PhaseB port input register
#define ENC0_PHASEB_PIN		  PC5	        // PhaseB port pin
#define ENC0_REVERSER             1

// -------------------- Encoder 1 (driver right)  connections --------------------
// Phase A quadrature encoder output should connect to this interrupt line:
// *** NOTE: the choice of interrupt pin and port must match the external
// interrupt you are using on your processor.  Consult the External Interrupts
// section of your processor's datasheet for more information.

// Interrupt Configuration
#define ENC1_SIGNAL		  SIG_INTERRUPT0    // Interrupt signal name
#define ENC1_INT		  INT0	            // matching INTx bit in GIMSK/EIMSK
#define ENC1_ICR		  EICRA	        // matching Int. Config Register (MCUCR,EICRA/B)
#define ENC1_ISCX0		  ISC00	        // matching Interrupt Sense Config bit0
#define ENC1_ISCX1		  ISC01	        // matching Interrupt Sense Config bit1
// PhaseA Port/Pin Configuration
// *** PORTx, DDRx, PINx, and Pxn should all have the same letter for "x" ***
#define ENC1_PHASEA_PORT	  PORTD	        // PhaseA port register
#define ENC1_PHASEA_DDR		  DDRD	        // PhaseA port direction register
#define ENC1_PHASEA_PORTIN	  PIND	        // PhaseA port input register
#define ENC1_PHASEA_PIN		  PD2		// PhaseA port pin

// Phase B quadrature encoder output should connect to this direction line:
// *** PORTx, DDRx, PINx, and Pxn should all have the same letter for "x" ***
#define ENC1_PHASEB_PORT	  PORTC	        // PhaseB port register
#define ENC1_PHASEB_DDR		  DDRC	        // PhaseB port direction register
#define ENC1_PHASEB_PORTIN	  PINC	        // PhaseB port input register
#define ENC1_PHASEB_PIN		  PC6		// PhaseB port pin
#define ENC1_REVERSER             -1

// -------------------- Encoder 2 connections (disabled) --------------------
// Phase A quadrature encoder output should connect to this interrupt line:
// *** NOTE: the choice of interrupt pin and port must match the external
// interrupt you are using on your processor.  Consult the External Interrupts
// section of your processor's datasheet for more information.

// Interrupt Configuration
//#define ENC2_SIGNAL		  SIG_INTERRUPT2    // Interrupt signal name
#define ENC2_INT		  INT2	        // matching INTx bit in GIMSK/EIMSK
#define ENC2_ICR		  EICRA	        // matching Int. Config Register (MCUCR,EICRA/B)
#define ENC2_ISCX0		  ISC20         // matching Interrupt Sense Config bit0
#define ENC2_ISCX1		  ISC21	        // matching Interrupt Sense Config bit1
// PhaseA Port/Pin Configuration
// *** PORTx, DDRx, PINx, and Pxn should all have the same letter for "x" ***
#define ENC2_PHASEA_PORT	  PORTB	        // PhaseA port register
#define ENC2_PHASEA_PORTIN	  PINB	        // PhaseA port input register
#define ENC2_PHASEA_DDR		  DDRB	        // PhaseA port direction register
#define ENC2_PHASEA_PIN		  PB2	        // PhaseA port pin

// Phase B quadrature encoder output should connect to this direction line:
// *** PORTx, DDRx, PINx, and Pxn should all have the same letter for "x" ***
#define ENC2_PHASEB_PORT	  PORTA	        // PhaseB port register
#define ENC2_PHASEB_DDR		  DDRA	        // PhaseB port direction register
#define ENC2_PHASEB_PORTIN	  PINA	        // PhaseB port input register
#define ENC2_PHASEB_PIN		  PA1	        // PhaseB port pin
#define ENC2_REVERSER             -1

// This motor is labelled "M3" on the board
#define MC_MOTOR0_PHASE_DDR DDRC
#define MC_MOTOR0_PHASE_PIN PC3
#define MC_MOTOR0_PHASE_PORT PORTC
#define MC_MOTOR0_PHASE_REVERSE 0

#define MC_MOTOR0_PWM_OCR OCR1B
#define MC_MOTOR0_PWM_DDR DDRD
#define MC_MOTOR0_PWM_PIN PD4
#define MC_MOTOR0_PWM_PORT PORTD
#define MC_MOTOR0_PWM_MAX 511
#define MC_MOTOR0_PWM_FORMULA(limitedPower) ((u16) abs( limitedPower ))

#define MC_MOTOR0_SLEEP_DDR DDRC
#define MC_MOTOR0_SLEEP_PIN PC4
#define MC_MOTOR0_SLEEP_PORT PORTC

// This motor is labelled "M2" on the board
#define MC_MOTOR1_PHASE_DDR DDRC
#define MC_MOTOR1_PHASE_PIN PC2
#define MC_MOTOR1_PHASE_PORT PORTC
#define MC_MOTOR1_PHASE_REVERSE 0

#define MC_MOTOR1_PWM_OCR OCR1A
#define MC_MOTOR1_PWM_DDR DDRD
#define MC_MOTOR1_PWM_PIN PD5
#define MC_MOTOR1_PWM_PORT PORTD
#define MC_MOTOR1_PWM_MAX 511
#define MC_MOTOR1_PWM_FORMULA(limitedPower) ((u08) abs( limitedPower ))

#define MC_MOTOR1_SLEEP_DDR DDRC
#define MC_MOTOR1_SLEEP_PIN PC1
#define MC_MOTOR1_SLEEP_PORT PORTC

// This motor is labelled "M1" on the board
#define MC_MOTOR2_PHASE_DDR DDRC
#define MC_MOTOR2_PHASE_PIN PC0
#define MC_MOTOR2_PHASE_PORT PORTC
#define MC_MOTOR2_PHASE_REVERSE 1

#define MC_MOTOR2_PWM_OCR OCR2B
#define MC_MOTOR2_PWM_DDR DDRD
#define MC_MOTOR2_PWM_PORT PORTD
#define MC_MOTOR2_PWM_PIN PD6
#define MC_MOTOR2_PWM_MAX 255
#define MC_MOTOR2_PWM_FORMULA(limitedPower) ((u08) abs( limitedPower ))

#define MC_MOTOR2_SLEEP_DDR DDRD
#define MC_MOTOR2_SLEEP_PIN PD7
#define MC_MOTOR2_SLEEP_PORT PORTD

// this motor isn't here on robot 0
#define MC_MOTOR3_PWM_MAX 0

#endif
