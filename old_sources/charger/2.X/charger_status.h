// Different charger states
// Note that BatteryLogger.java also has these constants, so they both must be changed together
#define STATUS_FAST    0
#define STATUS_TRICKLE 1
#define STATUS_ERROR   2
#define STATUS_INIT    3
#define STATUS_NOT_DOCKED 4

// Different warnings, each should be a power of two to support
// multiple warnings.  Note that only FOUR (4) bits are available
// here due to bit-packing into an s16
#define WARNING_NONE 0
#define WARNING_EEPROM_NOT_INITIALIZED 1

// These values are also used to determine how many times to blink the LED when an error occurs
// Note that BatteryLogger.java also has these constants, so they both must be changed together
// start at 1
#define  TERM_NEVER_TERMINATED 0
#define  TERM_MIN_TEMP 1
#define  TERM_MAX_TEMP 2
#define  TERM_MAX_VOLT 3
#define  TERM_MAX_CURRENT_ADJUSTMENT 4
#define  TERM_MAX_TIME 5
#define  TERM_SOFTWARE_ERROR1 6
#define  TERM_SOFTWARE_ERROR2 7
#define  TERM_SOFTWARE_ERROR3 8
#define  TERM_SOFTWARE_ERROR4 9
#define  TERM_SOFTWARE_ERROR5 10
#define  TERM_SOFTWARE_ERROR6 11
#define  TERM_MAX_FAST_TEMP 12
#define  TERM_DV_DT 13
#define  TERM_DT_DT 14
