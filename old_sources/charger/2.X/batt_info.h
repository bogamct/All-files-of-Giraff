// capacity in amp hours also known as "C"
#ifndef BATT_SIZE
#error "You must define batt_size, the battery amp hours.  Currently either 5.0 (powerspace) or 4.4 (shwh power)"
#endif

// a full batteyr is defined as zero (cause it's a definite point we can identify) so an empty battery is
// a negative number.  empirically determined.  We measured for a 5.0 Watt-hour battery, so we calculate
// a smaller amount for a smaller batter
#define BATT_EMPTY_WATT_SECONDS         (-372000.0f / (5.0/BATT_SIZE))

//We want the first low level warning (yellow indicator) to occur when there is at least 10min left
//The mothrboard G03-9F2 consumes about 40.8W, to be safe we use 41W in the calculations
//So calculate how many ws the motherboard consumes = 41*10*60 = 24600Ws
//We also need to make some assumption about the power consumption of the rest of the system.
//Since the drive motors consume most power we will assume the worst case scenario that they
//will be running at full power. Then we can naglect the rest of the system power consumption and still be on the safe side
//for almost any use.
//Each drive motor has a current limit of 2.5A and the battery voltage will be about 23V in average the last 10min.
//Drive motor power consumption = 5*23*10*60 = 69000 => Worst case total power consumption = 93600Ws => 28.5%, use 30%.
#define BATT_LOW_WARNING_PERCENT 15.0f //in %. Represents 5min of worst case use
#define BATT_PRE_LOW_WARNING_PERCENT 30.0f //in %. Represents 10min of worst case use
#define BATT_LOW_WARNING_WATT_SECONDS   BATT_EMPTY_WATT_SECONDS*(BATT_LOW_WARNING_PERCENT/100.0f) //Indicator should turn red at 15%
#define BATT_PRE_LOW_WARNING_WATT_SECONDS    BATT_EMPTY_WATT_SECONDS*(BATT_PRE_LOW_WARNING_PERCENT/100.0f) //Indicator should turn yellow at 30%
#define BATT_CELLS 20
#define BATT_TRICKLE_CHARGE_CURRENT 0.1f
#define BATT_MAX_VOLTAGE (BATT_CELLS * 1.6f)
#define BATT_VOLTAGE 24 // nominal voltage of battery

