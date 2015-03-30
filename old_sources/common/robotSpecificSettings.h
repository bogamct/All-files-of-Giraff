#ifdef MC_ROBOT0
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#include "robot0Pins.h"
#include "robot0Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT1
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#include "robot1Pins.h"
#include "robot1Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT2
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#include "robot2Pins.h"
#include "robot2Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT3
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVD
#include "pcbRevDPins.h"
#include "robot2Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT4
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVE
#include "pcbRevEPins.h"
#include "robot2Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT5
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#include "pcbRevGPins.h"
#include "robot5Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT6
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#include "pcb3Pins.h"
#include "robot6Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT7
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#include "pcb1Pins.h"
#include "robot7Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT8
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#include "pcb4Pins.h"
#include "robot8Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT9
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#include "pcb5Pins.h"
#include "robot9Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT10
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#define ROBOT_NUM 10
#include "pcb10Pins.h"
#include "robot10Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT11
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#define ROBOT_NUM 11
#include "pcb10Pins.h"
#include "robot10Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT12
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#define ROBOT_NUM 12
#include "pcb10Pins.h"
#include "robot10Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT13
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#define ROBOT_NUM 13
#include "pcb10Pins.h"
#include "robot10Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT14
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
#define ROBOT_NUM 14
#include "pcb10Pins.h"
#include "robot10Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT1_1
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
// 1_1 build covers robots 15 to 19
#define ROBOT_NUM 15
#include "pcb10Pins.h"
#include "robot10Constants.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT3_1_17
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
// 3_1_17 and 3_1_24 build covers robots 20 to 35, gear ratio dependent
#define ROBOT_NUM 15
#include "pcb10Pins.h"
#include "robot3.1_17.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT3_1_24
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
// 3_1_17 and 3_1_24 build covers robots 20 to 34, gear ratio dependent
#define ROBOT_NUM 15
#include "pcb10Pins.h"
#include "robot3.1_24.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT3_2
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
// 3_2 build covers Giraffs 35 to 50
#define ROBOT_NUM 15
#include "pcb10Pins.h"
#include "robot3.2.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT3_3
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
// 3_3 build covers Giraffs 51 to 90
#define ROBOT_NUM 15
#include "pcb10Pins.h"
#include "robot3.3.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT3_4
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVG
// 3_4 in the prototype stadium
#define ROBOT_NUM 15
#include "pcb10Pins.h"
#include "robot3.4.h"
#define MC_ROBOT
#endif

#ifdef MC_ROBOT4_0
#ifdef MC_ROBOT
#error "Robot constant already defined."
#endif
#define MC_PCB_REVL
// 4_0 in the prototype stadium
#define ROBOT_NUM 15
#include "pcbRevLPins.h"
#include "robot4.0.h"
#define MC_ROBOT
#endif

#ifndef MC_ROBOT0
#ifndef MC_ROBOT1
#ifndef MC_ROBOT2
#ifndef MC_ROBOT3
#ifndef MC_ROBOT4
#ifndef MC_ROBOT5
#ifndef MC_ROBOT6
#ifndef MC_ROBOT7
#ifndef MC_ROBOT8
#ifndef MC_ROBOT9
#ifndef MC_ROBOT10
#ifndef MC_ROBOT11
#ifndef MC_ROBOT12
#ifndef MC_ROBOT13
#ifndef MC_ROBOT14
#ifndef MC_ROBOT1_1
#ifndef MC_ROBOT3_1_17
#ifndef MC_ROBOT3_1_24
#ifndef MC_ROBOT3_2
#ifndef MC_ROBOT3_3
#ifndef MC_ROBOT3_4
#ifndef MC_ROBOT4_0
#error "Must define either MC_ROBOT# in Makefile"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
