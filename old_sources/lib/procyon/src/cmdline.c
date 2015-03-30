/*! \file cmdline.c \brief Command-Line Interface Library. */
//*****************************************************************************
//
// File Name	: 'cmdline.c'
// Title		: Command-Line Interface Library
// Author		: Pascal Stang - Copyright (C) 2003
// Created		: 2003.07.16
// Revised		: 2003.07.23
// Version		: 0.1
// Target MCU	: Atmel AVR Series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

//----- Include Files ---------------------------------------------------------
#include <avr/pgmspace.h>	// include AVR program memory support
#include <string.h>		// include standard C string functions
#include <stdlib.h>

// include project-specific configuration
#include <procyon/cmdline.h>
#include "global.h"		// include our global settings
#include "cmdlineconf.h"

// defines
#define ASCII_BEL				0x07
#define ASCII_BS				0x08
#define ASCII_CR				0x0D
#define ASCII_LF				0x0A
#define ASCII_ESC				0x1B
#define ASCII_DEL				0x7F

#define VT100_ARROWUP           'A'
#define VT100_ARROWDOWN         'B'
#define VT100_ARROWRIGHT        'C'
#define VT100_ARROWLEFT         'D'

#define VT100_UP    2
#define VT100_DOWN  3
#define VT100_LEFT  4
#define VT100_RIGHT 5


// Global variables

// strings
char PROGMEM CmdlineOK[] =  "OK ";
char PROGMEM CmdlineERR[] = "ER ";
// command list
// -commands are null-terminated strings
static char CmdlineCommandList[CMDLINE_MAX_COMMANDS][CMDLINE_MAX_CMD_LENGTH];
// command function pointer list
static CmdlineFuncPtrType CmdlineFunctionList[CMDLINE_MAX_COMMANDS];
// number of commands currently registered
u08 CmdlineNumCommands;

char CmdlineBuffer[CMDLINE_BUFFERSIZE];
u08 CmdlineBufferEditPos;
CmdlineFuncPtrType CmdlineExecFunction;
u16 m_cmdSequence = 100;

// Functions

// function pointer to single character output routine
static void (*cmdlineOutputFunc)(unsigned char c);

void cmdlineInit(void) {
  // initialize input buffer
  CmdlineBufferEditPos = 0;
  // initialize executing function
  CmdlineExecFunction = 0;
  // initialize command list
  CmdlineNumCommands = 0;
}

void cmdlineAddCommand(char* newCmdString, CmdlineFuncPtrType newCmdFuncPtr) {
  // add command string to end of command list
  strcpy(CmdlineCommandList[CmdlineNumCommands], newCmdString);
  // add command function ptr to end of function list
  CmdlineFunctionList[CmdlineNumCommands] = newCmdFuncPtr;
  // increment number of registered commands
  CmdlineNumCommands++;
}

void cmdlineSetOutputFunc(void (*output_func)(unsigned char c)) {
  // set new output function
  cmdlineOutputFunc = output_func;
}

void cmdlineInputFunc(unsigned char c) {
  u08 i;
  // process the received character
  
  switch(c) {
  case VT100_UP:
    break;
  case VT100_DOWN:
    break;
  case VT100_RIGHT:
    break;
  case VT100_LEFT:
    break;
  case ASCII_CR:
    // user pressed [ENTER]
    // echo CR and LF to terminal
    //cmdlineOutputFunc(ASCII_CR);
    //cmdlineOutputFunc(ASCII_LF);
    // add null termination to command
    CmdlineBuffer[CmdlineBufferEditPos++] = 0;
    // command is complete, process it
    cmdlineProcessInputString();
    // reset buffer
    CmdlineBufferEditPos = 0;
    break;
  case ASCII_BS:
    if(CmdlineBufferEditPos) {
	// destructive backspace
	// echo backspace-space-backspace
	//cmdlineOutputFunc(ASCII_BS);
	//cmdlineOutputFunc(' ');
	//cmdlineOutputFunc(ASCII_BS);
	// decrement our buffer length and edit position
	CmdlineBufferEditPos--;
    }
    break;
  default:
    // we do CMDLINE_BUFFERSIZE-1 to leave space for the \0 that is added
    if ( CmdlineBufferEditPos < CMDLINE_BUFFERSIZE - 1 ) {
      // echo character to the output
      //cmdlineOutputFunc(c);
      // add it to the command line buffer
      CmdlineBuffer[CmdlineBufferEditPos++] = c;
    }
  }
}

void println(const char *p_str) {
  char buf[5];
  int i;

  //print OK or ER
  const char* ptr = p_str;
  while(pgm_read_byte(ptr)) cmdlineOutputFunc( pgm_read_byte(ptr++) );

  //print num
  ltoa( m_cmdSequence++, buf, 10 );
  if ( m_cmdSequence > 999 ) {
    m_cmdSequence = 100;
  }
  for ( i=0; i < 3; i++ ) {
    cmdlineOutputFunc( buf[i] );
  }

  cmdlineOutputFunc( '>' );
  cmdlineOutputFunc( '\r' );
  cmdlineOutputFunc( '\n' );
}

void cmdlineProcessInputString(void) {
  u08 cmdIndex;
  u08 i=0;

  // find the end of the command (excluding arguments)
  // find first whitespace character in CmdlineBuffer
  while( !((CmdlineBuffer[i] == ' ') || (CmdlineBuffer[i] == 0)) ) i++;

  if(!i) {
    // command was null or empty
    // output a new prompt
    println( CmdlineOK );
    // we're done
    return;
  }

  // search command list for match with entered command
  for(cmdIndex=0; cmdIndex<CmdlineNumCommands; cmdIndex++) {
    if( !strncmp(CmdlineCommandList[cmdIndex], CmdlineBuffer, i) ) {
      // user-entered command matched a command in the list (database)
      // run the corresponding function
      CmdlineExecFunction = CmdlineFunctionList[cmdIndex];
      // new prompt will be output after user function runs
      // and we're done
      return;
    }
  }
  
  // if we did not get a match
  // output an error message
  println( CmdlineERR );
}

void cmdlineMainLoop(void) {
  // do we have a command/function to be executed
  if(CmdlineExecFunction) {
    // run it
    CmdlineExecFunction();
    // reset
    CmdlineExecFunction = 0;
    // output new prompt
    println( CmdlineOK );
  }
}

char* cmdlineGetArgStr2(u08 p_argNum, char* p_buffer) {
  char* start = cmdlineGetArgStr( p_argNum );
  char* end = start;
  int len;

  while( *end != 0 && *end != ' ') end++;
  len = end-start;
  memcpy( p_buffer, start, len );
  p_buffer[len] = '\0';
  return p_buffer;
}

// return string pointer to argument [argnum]
char* cmdlineGetArgStr(u08 argnum) {
  // find the offset of argument number [argnum]
  u08 idx=0;
  u08 arg;
	
  // find the first non-whitespace character
  while( (CmdlineBuffer[idx] != 0) && (CmdlineBuffer[idx] == ' ')) idx++;
	
  // we are at the first argument
  for(arg=0; arg<argnum; arg++) {
    // find the next whitespace character
    while( (CmdlineBuffer[idx] != 0) && (CmdlineBuffer[idx] != ' ')) idx++;
    // find the first non-whitespace character
    while( (CmdlineBuffer[idx] != 0) && (CmdlineBuffer[idx] == ' ')) idx++;
  }
  // we are at the requested argument or the end of the buffer
  return &CmdlineBuffer[idx];
}
