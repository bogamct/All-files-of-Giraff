#include "commonSPI.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// buffer sizes MUST be a power of 2
// HACK -- these constants don't belong here, they should be determined by the calling module
//#define SPI_RX_BUFFER_SIZE 16
//#define SPI_TX_BUFFER_SIZE 32

#define SPI_RX_BUFFER_MASK ( SPI_RX_BUFFER_SIZE - 1 )
#define SPI_TX_BUFFER_MASK ( SPI_TX_BUFFER_SIZE - 1 )

#if !defined(SPI_USES_USI_MODULE) && !defined(SPI_USES_SPI_MODULE)
  #if !defined(SPI_SLAVE_USES_SPI_MODULE) && !defined(SPI_USES_SPI_MODULE)
    #error "Must define either SPI_USES_USI_MODULE or SPI_USES_SPI_MODULE to use SPI"
  #endif
#endif

static volatile u08 SPI_TxBuf[SPI_TX_BUFFER_SIZE];
static volatile u08 SPI_RxBuf[SPI_RX_BUFFER_SIZE];
static volatile u08 SPI_TxHead;
static volatile u08 SPI_TxTail;
static volatile u08 SPI_RxHead;
static volatile u08 SPI_RxTail;
static volatile u08 SPI_Errors;

void spiReceiveCharInternal(u08 p_char);
void spiSendCharInternal();

void spiInternalCommonInit() {
  SPI_TxHead = 0;
  SPI_TxTail = 0;
  SPI_RxHead = 0;
  SPI_RxTail = 0;
  SPI_Errors = 0;
}

#ifdef SPI_USES_USI_MODULE
void spiInitSlave( char p_spiMode ) {
  spiInternalCommonInit();
  // Configure port directions.
  USI_DIR_REG |= (1<<USI_DATAOUT_PIN);                      // Outputs.
  USI_DIR_REG &= ~(1<<USI_DATAIN_PIN) | (1<<USI_CLOCK_PIN); // Inputs.
  USI_OUT_REG |= (1<<USI_DATAIN_PIN) | (1<<USI_CLOCK_PIN);  // Pull-ups.
  
  // Configure USI to 3-wire slave mode with overflow interrupt.
  USICR = (1<<USIOIE) //Enable counter overflow interrupt
        | (1<<USIWM0) //Use three wire mode (DO, DI and USCK pins)
		| (1<<USICS1) | (p_spiMode<<USICS0); //External clock

  // Get transfer started so transfer flag will be set
  USIDR = 0;
}

bool inline spiIsTransferCompleteFlag() {
  return ((USISR & (1<<USIOIF)) != 0);
}

void inline spiSetDataRegister(u08 p_char) {
  USIDR = p_char;
}

u08 inline spiGetDataRegister() {
  return USIDR;
}

void inline spiClearTransferCompleteFlag() {
  //clear bit.. Yes, somehow the flag is cleared by setting it to 1.
  USISR = (1<<USIOIF);
}

SIGNAL(SIG_USI_OVERFLOW) {
  spiClearTransferCompleteFlag();
  
  u08 received = spiGetDataRegister();
  if ( received != 0 ) {
    spiReceiveCharInternal( received );
  }
  spiSendCharInternal();
}
#endif

#ifdef SPI_SLAVE_USES_SPI_MODULE
void spiInitSlave() {
  u08 tmp;
  
  spiInternalCommonInit();
 
  /* Switch SCK and MOSI pins to input mode. */
  SPI_DIR_REG &= ~_BV(SPI_SCK_PIN) & ~_BV(SPI_MOSI_PIN);

  // MISO is an output
  SPI_DIR_REG |= _BV(SPI_MISO_PIN);
 
  /* Enable SCK and MOSI pull-up. */
  SPI_OUT_REG &= _BV(SPI_SCK_PIN) & _BV(SPI_MOSI_PIN);
 
  /* Activate the SPI hardware. */
  SPCR = _BV(SPE); //SPI enable
 
  SPSR = 0;

  // turn off power reduction stuff
  PRR = 0;

  /* Clear status flags. */
  tmp = SPSR;
  tmp = SPDR;

  // Get transfer started so transfer flag will be set
  SPDR = 0;  
}

bool inline spiIsTransferCompleteFlag() {
  return ((SPSR & (1<<SPIF)) != 0);
}

void inline spiSetDataRegister(u08 p_char) {
  SPDR = p_char;
}

u08 inline spiGetDataRegister() {
  return SPDR;
}

// cleared automatically when SPDR is read
void inline spiClearTransferCompleteFlag() {
}

SIGNAL(SIG_SPI) {
  spiClearTransferCompleteFlag();
  
  u08 received = spiGetDataRegister();
  if ( received != 0 ) {
    spiReceiveCharInternal( received );
  }
  spiSendCharInternal();
}
#endif

#ifdef SPI_USES_SPI_MODULE
void spiInitMaster() {
  u08 tmp;
  
  spiInternalCommonInit();
  /* Check whether SPI_SS is an input. */
  if (bit_is_clear(SPI_DIR_REG, SPI_SS_PIN)) {
    /* If yes, activate the pull-up. */
    SPI_OUT_REG |= _BV(SPI_SS_PIN);
  }
  
  /* Switch SCK and MOSI pins to output mode. */
  SPI_DIR_REG |= _BV(SPI_SCK_PIN) | _BV(SPI_MOSI_PIN);

  // MISO is an input
  SPI_DIR_REG &= ~_BV(SPI_MISO_PIN);
  
  /* Enable MISO pull-up. */
  SPI_OUT_REG |= _BV(SPI_MISO_PIN);
 
  /* Activate the SPI hardware. */
  SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);

  SPSR = 0;

  // turn off power reduction stuff
  PRR = 0;

  /* Clear status flags. */
  tmp = SPSR;
  tmp = SPDR;

  // Get transfer started so transfer flag will be set
  SPDR = 0;
}

void spiDisableMaster() {
  //make these pins inputs
  SPI_DIR_REG &= ~_BV(SPI_SCK_PIN);
  SPI_DIR_REG &= ~_BV(SPI_MOSI_PIN);  
  SPI_DIR_REG &= ~_BV(SPI_MISO_PIN);  
  SPI_DIR_REG &= ~_BV(SPI_SS_PIN);  

  //disable the pull-up resistors
  SPI_OUT_REG &= ~_BV(SPI_SCK_PIN);
  SPI_OUT_REG &= ~_BV(SPI_MOSI_PIN);  
  SPI_OUT_REG &= ~_BV(SPI_MISO_PIN);  
  SPI_OUT_REG &= ~_BV(SPI_SS_PIN);  

  SPCR &= ~_BV(SPE);
}

bool inline spiIsTransferCompleteFlag() {
  return ((SPSR & (1<<SPIF)) != 0);
}

void inline spiSetDataRegister(u08 p_char) {
  SPDR = p_char;
}

u08 inline spiGetDataRegister() {
  return SPDR;
}

// cleared automatically when SPDR is read
void inline spiClearTransferCompleteFlag() {
}

void spiMasterTick() {
  if (spiIsTransferCompleteFlag()) {
    u08 received = spiGetDataRegister();

    spiClearTransferCompleteFlag();
    if ( received != 0 ) {
      spiReceiveCharInternal( received );
    }
    spiSendCharInternal();
  }
}
#endif

void inline disableOverflowInterrupt() {
#ifdef SPI_USES_USI_MODULE
  USICR &= ~(1<<USIOIE);
#endif
}

void inline enableOverflowInterrupt() {
#ifdef SPI_USES_USI_MODULE
  USICR |= (1<<USIOIE);
#endif
}

void spiSendCharInternal() {
  if ( SPI_TxHead != SPI_TxTail ) {
    u08 tmptail = (SPI_TxTail + 1) & SPI_TX_BUFFER_MASK;
    SPI_TxTail = tmptail;
    spiSetDataRegister( SPI_TxBuf[tmptail] );
  } else {
    spiSetDataRegister( 0 );
  }
}

void spiReceiveCharInternal(u08 p_char) {
  u08 tmphead;

  tmphead = ( SPI_RxHead + 1 ) & SPI_RX_BUFFER_MASK;
  
  if ( tmphead == SPI_RxTail ) {
    SPI_Errors |= 1;
    //buffer overflow
  } else {
    SPI_RxHead = tmphead;
    SPI_RxBuf[tmphead] = p_char;
  }
}

u08 inline spiGetRXCount() {
  u08 rx;

  disableOverflowInterrupt();
  if ( SPI_RxHead >= SPI_RxTail ) {
    rx = SPI_RxHead - SPI_RxTail;
  } else {
    rx = SPI_RX_BUFFER_SIZE - ( SPI_RxTail - SPI_RxHead );
  }
  enableOverflowInterrupt();
  return rx;
}

u08 spiGetTXCount() {
  u08 tx;

  disableOverflowInterrupt();
  if ( SPI_TxHead >= SPI_TxTail ) {
    tx = SPI_TxHead - SPI_TxTail;
  } else {
    tx = SPI_TX_BUFFER_SIZE - ( SPI_TxTail - SPI_TxHead );
  }
  enableOverflowInterrupt();
  return tx;
}

u08 spiGetErrors() {
  return SPI_Errors;
}

void spiClearErrors() {
  SPI_Errors=0;
}

bool spiIsCharAvailable() {
  return spiGetRXCount() > 0;
}

u08 spiGetChar() {
  u08 tmptail;
  u08 data;

  disableOverflowInterrupt();
  if ( SPI_RxHead == SPI_RxTail ) {
    SPI_Errors |= 2;
    data = SPI_NO_DATA;
  } else {
    tmptail = (SPI_RxTail + 1) & SPI_RX_BUFFER_MASK;
    SPI_RxTail = tmptail;

    data = SPI_RxBuf[tmptail];
  }
  enableOverflowInterrupt();
  return data;
}

void spiPutString( char* p_str ) {
  char* num=p_str;

  while (*num) {
    spiPutChar(*num);
    num++;
  }
}

void spiPutChar( char p_char ) {
  u08 tmphead;
  
  disableOverflowInterrupt();
  tmphead = (SPI_TxHead + 1) & SPI_TX_BUFFER_MASK;
  
  // this used to be a while statement without the return -- see uart.c code
  if ( tmphead == SPI_TxTail ){
    SPI_Errors |= 4;
  } else {
    SPI_TxBuf[tmphead] = p_char;
    SPI_TxHead = tmphead;
  }
  enableOverflowInterrupt();
}

/*
void spiSetTenBits(char* p_ptr, u16 p_num) {
  u16 low  = (p_num & (1+2+4+8+16));
  u16 high = (p_num & (32+64+128+256+512));
  high = high >> 5;

  p_ptr[0] = ((u08) high) | 128;
  p_ptr[1] = ((u08) low) | 128;
  p_ptr[2] = 0;
}

u16 spiGetTenBits(char* p_ptr) {
  u08 high = p_ptr[0] &~128;
  u08 low = p_ptr[1] &~128;
  u16 num = (high << 5) | low;
  return num;
}
*/
