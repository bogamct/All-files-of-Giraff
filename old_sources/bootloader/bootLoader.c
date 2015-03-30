/******************************************************************************
 * Filename: bootLoader.c
 *
 * Author: Dan Sandberg
 *
 *
 * Based on intel hex boot loader by Preston K. Manwaring
 *
 * His boot loader was based on Martin Thomas's code
 *
 ******************************************************************************/

// Currently servo doesn't initialize properly because the same timer is used
// for the servo and for the 1 second delay!

/*--- INCLUDE FILES ----------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <common/vartypes.h>
#include <util/delay.h>
#include "../common/serial.h"
#include "../common/commonBootLoader.h"
#include "../common/robotSpecificSettings.h"

// Size of boot loader section in BYTES
#define BLS_SIZE_BYTES 2048

// buffer is used for itoa and for inputting commands
#define BUFFER_SIZE 50

#define UPLOAD_PAGE_COMMAND "UP_PG"
#define CHECK_PAGE_COMMAND  "CK_PG"
#define QUIT_COMMAND        "q"
#define PROMPT              "Ca>"

#define RESPONSE_ERROR   "ER"
#define RESPONSE_OK      "OK"
#define CMD_UNKNOWN "H?"

// Used to keep track of whether the EOF byte from the binary upload
// has been reached
char static s_buffer[BUFFER_SIZE];
char static s_tmpBuffer[BUFFER_SIZE];
u16 static s_flashBuffer[ SPM_PAGESIZE / 2];
u08 static s_stayInBootLoader = 0;
u16 static s_cmdIndex = 0;

u08 static volatile s_timeoutExpired = 0;
u16 static volatile s_timeoutCounter = 0;
u16 static volatile s_ledCounter     = 0;

// Needed to jump to application section
void (*jumpToApplication_internal)(void) = 0x0000;

// forward declarations
inline void processCommand();

static inline void jumpToApplication(void) {
  // move interruptvectors to the application sector
  MCUCR = _BV(IVCE);
  MCUCR = 0;               

  // disable timer interrupt
  TIMSK0 = 0;
  jumpToApplication_internal();
}

static inline void fillBuffer() {
  u16 i;
  u16 num;

  for ( i=0; i < SPM_PAGESIZE;i+=2 ) {
    num = getWord();
    s_flashBuffer[i/2] = num;
  }
}

static inline u16 getCRC( u16 p_start, u16 p_length ) {
  int i;
  u16 data;
  u16 crc = 0xffff;

  for (i=0;i < p_length;i+=2) {
    data = pgm_read_word( p_start+i );
    crc = crcUpdate( crc, data >> 8);
    crc = crcUpdate( crc, data & 0xff );
  }
  return crc;
}

static inline void flashPage( u16 p_page ) {
  u16 i;
  u16 data;

  boot_page_erase( p_page );
  boot_spm_busy_wait();
  
  for( i = 0; i < SPM_PAGESIZE; i += 2 ) {
    data = s_flashBuffer[i/2];
    boot_page_fill( p_page+i, data );
  }
  
  boot_page_write( p_page );
  boot_spm_busy_wait();	
}

static inline void uartPutStringCRLF( char* p_str ) {
  uartPutString( p_str );
  uartPutString( "\r\n" );
}

// reads a line into s_buffer
static inline u08 getLine() {
  if ( uartIsCharAvailable() ) {
    char c = uartBlockingGetChar();
    if ( c == 13 ) {
      s_buffer[s_cmdIndex]='\0';
      s_cmdIndex = 0;
      return 1;
    } else {
      s_buffer[s_cmdIndex] = c;
      if ( s_cmdIndex < BUFFER_SIZE - 1 ) {
	s_cmdIndex++;
      }
    }
  }    
  return 0;
}

// gives a response of the form CK_UP 5:OK/18
static inline void respond( char* p_command, u16 p_page, char* p_response ) {
  uartPutString( p_command );
  uartPutChar( ' ' );

  itoa( p_page, s_tmpBuffer, 10 );
  uartPutString( s_tmpBuffer );
  uartPutChar( ':' );

  uartPutString( p_response );
  uartPutChar( '/' );

  u16 bufferRemaining = uartReceiveBufferRemaining();
  itoa( bufferRemaining, s_tmpBuffer, 10 );
  uartPutStringCRLF( s_tmpBuffer );
}

static inline void checkPage() {
  u16 ourCRC = 0xffff;
  u16 page = getWordWithCRC( &ourCRC );
  u16 realCRC = getWord();
  char* response = RESPONSE_ERROR;

  if ( realCRC == ourCRC ) {
    u16 theCRC = getCRC( page * SPM_PAGESIZE, SPM_PAGESIZE );
    itoa( theCRC, s_buffer, 10 );
    response = s_buffer;
  } 
  respond( CHECK_PAGE_COMMAND, page, response );
}

static inline void uploadPage() {
  u16 ourCRC = 0xffff;
  u16 page = getWordWithCRC( &ourCRC );
  u16 realCRC = getWord();
  char *response = RESPONSE_ERROR;

  fillBuffer( page );
  if ( realCRC == ourCRC ) {
    flashPage( page * SPM_PAGESIZE );
    boot_rww_enable();				//Re-enable the RWW section
    response = RESPONSE_OK;
  } 
  respond( UPLOAD_PAGE_COMMAND, page, response );
}

static inline void ledsInit(void) {
  // Set the LED1 pin to an output pin
  MC_LED0_DDR |= _BV( MC_LED0_PIN );

  // Turn LED1 on
  MC_LED0_PORT |= _BV( MC_LED0_PIN );
}

static inline void motorsInit(void) { 
  MC_MOTOR0_PWM_DDR |= MC_MOTOR0_PWM_PIN;
  MC_MOTOR1_PWM_DDR |= MC_MOTOR1_PWM_PIN;
  MC_MOTOR2_PWM_DDR |= MC_MOTOR2_PWM_PIN;

  MC_MOTOR0_PWM_PORT &=~ MC_MOTOR0_PWM_PIN;
  MC_MOTOR1_PWM_PORT &=~ MC_MOTOR1_PWM_PIN;
  MC_MOTOR2_PWM_PORT &=~ MC_MOTOR2_PWM_PIN;

  MC_MOTOR0_PWM_OCR = MC_MOTOR1_PWM_OCR = MC_MOTOR2_PWM_OCR = 0;
} 

static inline void timerInit(void) {
  TCCR0A = 0;

  // prescaler is set to 1024, so overflow happens at FCPU/(1024*256) Hz
  TCCR0B = (1<<CS02) | (1<<CS00);
  TCNT0  = 0;

  // enable timer overflow interrupt
  TIMSK0 = (1<<TOIE0);
}

int main(void) {
  ledsInit();
  timerInit();

  //move interruptvectors to the Boot sector
  // this strange sequence ( two separate lines ) is necessary
  MCUCR = _BV(IVCE); 
  MCUCR = _BV(IVSEL);

  sei();
  motorsInit();

  uartInit( UART_BAUD_SELECT( 115200, F_CPU ) );
  uartFlowControlOn( 0 );

  uartPutStringCRLF( PROMPT );
  while(1) {   

    // if more than a second has elapsed
    if ( s_timeoutExpired  && !s_stayInBootLoader ) {
      jumpToApplication();
    }

    if ( getLine() ) {
      if ( !strcmp( s_buffer, UPLOAD_PAGE_COMMAND ) ) {
	s_stayInBootLoader = 1;
	uploadPage();
      } else if ( !strcmp( s_buffer, CHECK_PAGE_COMMAND ) ) {
	s_stayInBootLoader = 1;
	checkPage();
      } else if ( !strcmp( s_buffer, QUIT_COMMAND ) ) {
	jumpToApplication();
      } else if ( !strcmp( s_buffer, "" ) ) {
	s_stayInBootLoader = 1;
	// don't do anything for blank lines
      } else {
	uartPutStringCRLF( CMD_UNKNOWN );
      }
      uartPutStringCRLF( PROMPT );      
    }
  }
  
  return 0;
}

// called 35.15 times per second
SIGNAL(SIG_OVERFLOW0) {
  s_timeoutCounter++;
  s_ledCounter++;

  // 3 * 35 = 3 seconds
  if ( s_timeoutCounter > 105 ) {
    s_timeoutExpired = 1;
  }

  if ( s_ledCounter > 12 ) {
    s_ledCounter = 0;

    // blink LED1
    MC_LED0_PORT ^= _BV( MC_LED0_PIN );
  }
}

