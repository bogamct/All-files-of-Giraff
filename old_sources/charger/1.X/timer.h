#ifndef TIMER_H
#define TIMER_H

//we've slowed down timer by 1/8
#define MICROS_TO_TIMER_TICKS(x) ( (u16) (.031250 * 0.125 * (x)) )

#endif
