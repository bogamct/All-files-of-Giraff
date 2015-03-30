// simulator_hal.c - Provides abstractions of calls to hardware
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "simulator_hal.h"

#include <unistd.h>
#include <time.h>
#include <pthread.h>

static char m_c;
static int line_mode = 0;
static int new_data = 0;
char c;
int str_pointer=0;
char str[100];
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;

float m_simulatedServoVelocity = 0.0; // start stationary
float m_simulatedServoPosition = -50.0; // start at random location

void flushSimulatedServoPosition() {
 m_simulatedServoPosition = 0;
}

void simulateAlteredPowerOfServo(s32 servoError) {
	// calculate new servo position given new power reading
	// positive power moves in positive direction
	// negative power moves in negative direction
	m_simulatedServoVelocity += ((float) servoError) / 100.0; // was 500 is arbitrary scaling constant
	m_simulatedServoPosition += m_simulatedServoVelocity;
}

s32 getSimulatedServoPosition() {
	
	return (s32) m_simulatedServoPosition;
}

int getSimulatedServoHomeSwitchState() {
	
	if (m_simulatedServoPosition < 200.0) {
		return 1;
	} else {
		return 0;
	}
}


void *characterBufferThread( void *ptr ) {
  while (1) {
    c = getchar ();
    if (c == -1) continue;  // ignore EOF
    if (c == 10) { c = 13; } // convert to LF to keep controller.c happy
    str[str_pointer] = c;
    str_pointer++;
    if (c == 13) { str[str_pointer] = 0;  str_pointer = 0; new_data = 1; }		
    while (new_data == 1) {
      pthread_mutex_lock( &condition_mutex );		
      pthread_cond_wait( &condition_cond, &condition_mutex );
      pthread_mutex_unlock( &condition_mutex );				
    }
  }
}

void uartInit() {
  pthread_t thread1;
  char *message1 = "Buffered Input Thread";
  int  iret1;

  if (1) { //(set_tty_raw() == -1) // not a TTY
    line_mode = 1;
    
    // create thread to listen for new input
    iret1 = pthread_create( &thread1, NULL, characterBufferThread, (void*) message1);	
  }
}	

int uartIsCharAvailable() {
  if (line_mode) {
    if (new_data) { return 1;} else {return 0;}
  }
  
  if ((m_c = kb_getc())!= 0) {
    echo(m_c);
    if (m_c == '~') {
      set_tty_cooked(); 
      exit(0); 
    }
    return 1;
  } else {
    return 0;
  }
}

char uartBlockingGetChar() {
  if (line_mode) {
    char c = str[str_pointer];
    str_pointer++;
    if (str[str_pointer] == 0) {
      pthread_mutex_lock( &condition_mutex );
      new_data = 0; 
      str_pointer = 0; 
      pthread_cond_signal( &condition_cond );
      pthread_mutex_unlock( &condition_mutex );
    }
    //printf ("[%c, %d]\n", c, (int) c);
    return c;
  }
  return m_c;
}

char *  ltoa ( long value, char * buffer, int radix ) {
  sprintf (buffer, "%ld", value);
  return buffer;
}

// converts the double value passed in val into an ASCII representationthat will be stored under s
char * 	dtostrf (double __val, char __width, char __prec, char *__s) {
  sprintf (__s, "%f", __val);
  return __s;
}

int strcmp_P (const char * string1, const char * string2) {
  return strcmp(string1, string2);
}



// no more than 1 sec (1000 ms)
void _delay_ms(unsigned int ms) {
  struct timespec request;
  struct timespec leftOver;
  int sleepOK;
  request.tv_sec = 0;
  request.tv_nsec = ms * 1000000;
  leftOver.tv_sec = 0;
  leftOver.tv_nsec = 0;
  float i,j;

  do {
    sleepOK = nanosleep(&request, &leftOver);
    request.tv_nsec = leftOver.tv_nsec;
  } while (leftOver.tv_nsec != 0);

  for(i=0.0;i<500.0;i=i+1.0) {
    j*=i;
  }
}

// encoder-specific code follows

#define NUMBER_OF_ENCODERS 3
static long encoderPos[NUMBER_OF_ENCODERS];
static char encoder_is_on=0;

// called during unit test to simulate correct encoder values
void encoder_simulator(unsigned char encoderNum, long position) {
  if (!encoder_is_on) {
    printf("Encoder not on!!!\n\r"); 
    exit(-1);
  }
  encoderPos[encoderNum] = -position;
}

void encoderInit(void) {
  int i;
  for (i=0;i< NUMBER_OF_ENCODERS; i++) {
    encoderPos[i] = 0;
  }
  encoder_is_on = 1;
}

void encoderOff(void) {
  encoder_is_on = 0;
}

long encoderGetPosition(unsigned char encoderNum) {
  if (!encoder_is_on) {
    printf("Encoder not on!!!\n\r"); 
    exit(-1);
  }
  if (encoderNum < NUMBER_OF_ENCODERS) {
    return encoderPos[encoderNum];
  }
  else {
    return 0;
  }
}

void encoderSetPosition(unsigned char encoderNum, long position) {
  if (!encoder_is_on) {
    printf("Encoder not on!!!\n\r"); 
    exit(-1);
  }
  if (encoderNum < NUMBER_OF_ENCODERS) {
    encoderPos[encoderNum] = position;
  }
}

static int slow_it_down = 0;
// simulates timer interrupt - interrupt normally occurs 36141 times per second
void simulate_timer_interrupt(u16* p_timer) {
  if (*p_timer) {
    slow_it_down++;
    if  (slow_it_down == 67) { // pick a prime number to keep weird resonances from occuring. This runs slower than real-time.
      slow_it_down = 0;
      struct timespec request;
      struct timespec leftOver;
      request.tv_sec = 0;
      request.tv_nsec = 1;
      nanosleep(&request, &leftOver);	
    }		
    *p_timer = *p_timer - 1;
  }
}

// UART
void uartPutChar( u08 p_char ) {}
void uartPutString( const char *s ) {}


// SPI Stuff
void spiDisableMaster() {}
int spiIsCharAvailable() {return 0;}
u08 spiGetErrors() {return 0;}
void spiClearErrors() {}
void spiInitMaster() {}
u08 spiGetChar() {return 0;}
void spiMasterTick() {}
void spiPutChar( char p_char ) {}
void spiPutString( char* p_str ) {}

