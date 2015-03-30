#ifndef BC_HAL_H
#define BC_HAL_H

#ifndef SIMULATE
#include <avr/interrupt.h>
#endif // SIMULATE

#include "./chemistry/NiMH.h"

// arbitrary integer mapping for the colors
#define RED 20
#define GREEN 30

#define AMBIENT_TEMPERATURE 20
#define TYPICAL_DOCK_VOLTAGE 32

static inline void timer0Enable() {
#ifndef SIMULATE
  //enable the output compare match interrupt
  TIMSK |= (1<<TOIE0);
#endif
}

static inline void timer0Disable() {
#ifndef SIMULATE
  //disable the output compare match interrupt
  TIMSK &= ~(1<<TOIE0);
#endif
}

extern u08 m_termination;

extern volatile u16 m_ledTimer;
extern volatile u16 m_dataSendTimer;
extern volatile u16 m_makeAdjustTimer;
extern volatile u16 m_rtcTimer;
extern volatile u16 m_initTimer;
extern volatile u16 m_mbTimer;

typedef struct {
  float simulatedBattV;
  float simulatedBattC; 
  float simulatedBattT;
  float simulatedDockV;
  float simulatedDockC;
  float simulatedAmpHours;
  u16 expectedTermination; 
  u16 expectedChargeStatus; 
  u16 waitInMinutes;
} TestCaseData;


// foward defs
void testCaseExecutor();
void convertPWMtoCurrent(u16 p_pwm);
void setPWM(u16 p_pwm);
u16 getPWM();
void loadNextTestCase(TestCaseData* currentTestCase);

void rtcInit();
void ledsInit();
void motherboardInit(bool);
void pllInit();
void pwmInit();
void timerInit();
void enableInterrupts();

bool getLED(int);
void setLED(int,bool);
void displayStatus();
void coulombCounterSimulate();

#endif
