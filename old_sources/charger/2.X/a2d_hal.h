#ifndef A2D_HAL_H
#define A2D_HAL_H

#include <common/vartypes.h>
#include <stdbool.h>

#define VREF 1.1f
#define RESISTANCE_CURRENT_SENSE .009f
#define RESISTOR1 1.43f
#define RESISTOR2 39.2f
#define DIFF_AMPLIFIER_GAIN 20
#define RESISTOR3 392.0f
#define RESISTOR4 39200.0f

void a2dInit();
bool a2dConversionFinished();
u16 a2dGetValue();
void a2dStartConversion();
void a2dChooseChannel(u08 p_cmd);
float a2dToTemperatureC(u16);
float a2dToVoltage(u16);
float a2dToCurrent(u16);

#ifdef SIMULATE
void testResistanceToC();
#endif

#endif
