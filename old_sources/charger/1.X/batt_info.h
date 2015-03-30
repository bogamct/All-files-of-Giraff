// capacity in amp hours also known as "C"
#ifndef BATT_SIZE
#error "You must define batt_size, the battery amp hours.  Currently either 5.0 (powerspace) or 4.4 (shwh power)"
#endif

// a full batteyr is defined as zero (cause it's a definite point we can identify) so an empty battery is
// a negative number.  empirically determined.  We measured for a 5.0 Watt-hour battery, so we calculate
// a smaller amount for a smaller batter
#define BATT_EMPTY_WATT_SECONDS	     (-372000.0f / (5.0/BATT_SIZE))
#define BATT_CELLS 20
#define BATT_TRICKLE_CHARGE_CURRENT 0.1f
#define BATT_MAX_VOLTAGE (BATT_CELLS * 1.6f)
#define BATT_VOLTAGE 24 // nominal voltage of battery
