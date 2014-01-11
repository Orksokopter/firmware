
//#define JIM_MPX_INVERT

//changes outside this rate are deemed sensor/buss errors
#define BARO_SANITY_CHECK_DMPS	100		// dm/S 20,40,60,80 or 100

// ===============================================================================================
// =                                UAVX Quadrocopter Controller                                 =
// =                           Copyright (c) 2008 by Prof. Greg Egan                             =
// =                 Original V3.15 Copyright (c) 2007 Ing. Wolfgang Mahringer                   =
// =                     http://code.google.com/p/uavp-mods/ http://uavp.ch                      =
// ===============================================================================================

//    This is part of UAVX.

//    UAVX is free software: you can redistribute it and/or modify it under the terms of the GNU
//    General Public License as published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.

//    UAVX is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without
//    even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//    See the GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License along with this program.
//    If not, see http://www.gnu.org/licenses/

#ifndef BATCHMODE
	//#define GKE
	//#define RX6CH
	//#define EXPERIMENTAL
	#define TESTING
	//#define RX6CH 					// 6ch Receivers
	//#define SIMULATE
	//#define HEXACOPTER
	#define QUADROCOPTER
	//#define TRICOPTER
	//#define HELICOPTER
	//#define AILERON
	//#define ELEVON
	//#define HAVE_CUTOFF_SW			// Ground PortC Bit 0 (Pin 11) for landing cutoff otherwise 4K7 pullup.
	//#define I2C_HW
#endif // !BATCHMODE

#ifdef CLOCK_40MHZ
//	#define USE_IRQ_ADC_FILTERS					// Use digital LP filters for ADC inputs - 16MHz irq overheads too high
#endif // CLOCK_40MHZ

#ifdef EXPERIMENTAL
	#define UAVXBOARD
#endif // EXPERIMENTAL

#ifdef I2C_HW
	#include "i2c.h"
#else
	#define MASTER 		0
	#define SLEW_ON 	0
#endif // I2C_HW

//________________________________________________________________________________________________

#define USE_PPM_FAILSAFE

// Airframe

#ifdef UAVXBOARD
	#define GYRO_ITG3200
	#define UAVX_HW
#endif

#if ( defined TRICOPTER | defined QUADROCOPTER | defined HEXACOPTER )
	#define MULTICOPTER
#endif

#if ( defined HELICOPTER | defined AILERON | defined ELEVON )
	#if ( defined AILERON | defined ELEVON )
		#define NAV_WING
	#endif
#endif

#ifdef QUADROCOPTER
	#define AF_TYPE QuadAF
#endif
#ifdef TRICOPTER
	#define AF_TYPE TriAF
#endif
#ifdef HEXACOPTER
	#define AF_TYPE HexAF
#endif
#ifdef HELICOPTER
	#define AF_TYPE HeliAF
#endif
#ifdef ELEVON
	#define AF_TYPE ElevAF
#endif
#ifdef AILERON
	#define AF_TYPE AilAF
#endif

// Filters

#define	ADC_ATT_FREQ				100		// Hz Roll and Pitch PID loops 125-200Hz
#define	ADC_BATT_FREQ				5
#define	ADC_ALT_FREQ				20		// x 0.1Hz baro sampled at 20Hz STEVE tune for baro noise

#ifdef TRICOPTER
	#define	ADC_YAW_FREQ			3		// Hz
#else
	#define	ADC_YAW_FREQ			10		// Hz
#endif // TRICOPTER
#define COMPASS_FREQ				10		// Hz must be less than 10Hz

#define GPS_INC_GROUNDSPEED					// GPS groundspeed is not used for flight but may be of interest

// Timeouts and Update Intervals

#define FAILSAFE_TIMEOUT_MS			1000L 	// mS. hold last "good" settings and then restore flight or abort
#define ABORT_TIMEOUT_GPS_MS		5000L	// mS. go to descend on position hold if GPS valid.
#define ABORT_TIMEOUT_NO_GPS_MS		0L		// mS. go to descend on position hold if GPS valid.
#define ABORT_UPDATE_MS				1000L	// mS. retry period for RC Signal and restore Pilot in Control

#define THROTTLE_LOW_DELAY_MS		1000L	// mS. that motor runs at idle after the throttle is closed
#define THROTTLE_UPDATE_MS			3000L	// mS. constant throttle time for altitude hold

#define NAV_ACTIVE_DELAY_MS			10000L	// mS. after throttle exceeds idle that Nav becomes active
#define NAV_RTH_LAND_TIMEOUT_MS		10000L	// mS. Shutdown throttle if descent lasts too long

#define UAVX_TELEMETRY_INTERVAL_MS		125L	// mS. emit an interleaved telemetry packet
#define ARDU_TELEMETRY_INTERVAL_MS		200L	// mS. alternating 1:5
#define UAVX_CONTROL_TELEMETRY_INTERVAL_MS 100L	// mS. flight control only
#define CUSTOM_TELEMETRY_INTERVAL_MS	250L	// mS.

#define GPS_TIMEOUT_MS				2000L	// mS.

#define	ALT_UPDATE_HZ				20L		// Hz based on 50mS update time for Baro

// Altitude Hold

#define ALT_SCRATCHY_BEEPER					// Scratchy beeper noise on altitude hold
#define ALT_HOLD_MAX_ROC_DMPS		5L		// Must be changing altitude at less than this for alt. hold to be detected

// Accelerometers

#define DAMP_HORIZ_LIMIT 			3L		// equivalent stick units - no larger than 5
#define DAMP_VERT_LIMIT_LOW			-5L		// maximum throttle reduction
#define DAMP_VERT_LIMIT_HIGH		20L		// maximum throttle increase

// Gyros

#define ATTITUDE_FF_DIFF			24L		// 0 - 32 max feedforward speeds up roll/pitch recovery on fast stick change

#define	ATTITUDE_ENABLE_DECAY				// enables decay to zero angle when roll/pitch is not in fact zero!
											// unfortunately there seems to be a leak which cause the roll/pitch
											// to increase without the decay.

// Enable "Dynamic mass" compensation Roll and/or Pitch
// Normally disabled for pitch only
//#define DISABLE_DYNAMIC_MASS_COMP_ROLL
//#define DISABLE_DYNAMIC_MASS_COMP_PITCH

// Altitude Hold

// the range within which throttle adjustment is proportional to altitude error
#define ALT_BAND_DM					50L		// Decimetres

#define LAND_DM						30L		// Decimetres deemed to have landed when below this height

#define ALT_MAX_THR_COMP			40L		// Stick units was 32

#define ALT_INT_WINDUP_LIMIT		16L

#define ALT_RF_ENABLE_CM			500L	// altitude below which the rangefiner is selected as the altitude source
#define ALT_RF_DISABLE_CM			600L	// altitude above which the rangefiner is deselected as the altitude source

// Navigation

#define NAV_ACQUIRE_BEEPER

//#define ATTITUDE_NO_LIMITS				// full stick range is available otherwise it is scaled to Nav sensitivity

#define NAV_RTH_LOCKOUT				350L	// ~35 units per degree - at least that is for IDG300

#define NAV_MAX_ROLL_PITCH 			25L		// Rx stick units
#define NAV_CONTROL_HEADROOM		10L		// at least this much stick control headroom above Nav control
#define NAV_DIFF_LIMIT				24L		// Approx double NAV_INT_LIMIT
#define NAV_INT_WINDUP_LIMIT		64L		// ???

#define NAV_ENFORCE_ALTITUDE_CEILING		// limit all autonomous altitudes
#define NAV_CEILING					120L	// 400 feet
#define NAV_MAX_NEUTRAL_RADIUS		3L		// Metres also minimum closing radius
#define NAV_MAX_RADIUS				99L		// Metres

#ifdef NAV_WING
	#define NAV_PROXIMITY_RADIUS	20L		// Metres if there are no WPs
	#define NAV_PROXIMITY_ALTITUDE	5L		// Metres
#else
	#define NAV_PROXIMITY_RADIUS	5L		// Metres if there are no WPs
	#define NAV_PROXIMITY_ALTITUDE	3L		// Metres
#endif // NAV_WING

// reads $GPGGA sentence - all others discarded

#define	GPS_MIN_SATELLITES			6		// preferably > 5 for 3D fix
#define GPS_MIN_FIX					1		// must be 1 or 2
#define GPS_ORIGIN_SENTENCES 		30L		// Number of sentences needed to obtain reasonable Origin
#define GPS_MIN_HDILUTE				130L	// HDilute * 100

#define	NAV_SENS_THRESHOLD 			40L		// Navigation disabled if Ch7 is less than this
#define	NAV_SENS_ALTHOLD_THRESHOLD 	20L		// Altitude hold disabled if Ch7 is less than this
#define NAV_SENS_6CH				80L		// Low GPS gain for 6ch Rx

#define	NAV_YAW_LIMIT				10L		// yaw slew rate for RTH
#define NAV_MAX_TRIM				20L		// max trim offset for altitude hold
#define NAV_CORR_SLEW_LIMIT			1L		// *5L maximum change in roll or pitch correction per GPS update

#define ATTITUDE_HOLD_LIMIT 		8L		// dead zone for roll/pitch stick for position hold
#define ATTITUDE_HOLD_RESET_INTERVAL 25L	// number of impulse cycles before GPS position is re-acquired

//#define NAV_PPM_FAILSAFE_RTH				// PPM signal failure causes RTH with Signal sampled periodically

// Throttle

#define	THROTTLE_MAX_CURRENT		40L		// Amps total current at full throttle for estimated mAH
#define	CURRENT_SENSOR_MAX			50L		// Amps range of current sensor - used for estimated consumption - no actual sensor yet.
#define	THROTTLE_CURRENT_SCALE	((THROTTLE_MAX_CURRENT * 1024L)/(200L * CURRENT_SENSOR_MAX ))

#define THROTTLE_SLEW_LIMIT			0		// limits the rate at which the throttle can change (=0 no slew limit, 5 OK)
#define THROTTLE_MIDDLE				10  	// throttle stick dead zone for baro
#define THROTTLE_MIN_ALT_HOLD		75		// min throttle stick for altitude lock

// RC

#define RC_INIT_FRAMES				32		// number of initial RC frames to allow filters to settle

//________________________________________________________________________________________

//#include "UAVXRevision.h"

// 18Fxxx C18 includes

#include <p18cxxx.h>
#include <math.h>
#include <delays.h>
#include <timers.h>
#include <usart.h>
#include <capture.h>
#include <adc.h>

// Useful Constants
#define NUL 	(uint8)0
#define SOH 	(uint8)1
#define EOT 	(uint8)4
#define ACK		(uint8)6
#define HT 		(uint8)9
#define LF 		(uint8)10
#define CR 		(uint8)13
#define NAK 	(uint8)21
// #define ESC 	(uint8)27
#define true 	(uint8)1
#define false 	(uint8)0

#define MILLIPI 			3142
#define CENTIPI 			314
#define HALFMILLIPI 		1571
#define QUARTERMILLIPI		785
#define SIXTHMILLIPI		524
#define TWOMILLIPI 			6284

#define MILLIRAD 			18
#define CENTIRAD 			2

#define MAXINT32 			0x7fffffff
#define	MAXINT16 			0x7fff

// Additional Types
typedef unsigned char 		uint8 ;
typedef signed char 		int8;
typedef unsigned int 		uint16;
typedef int 				int16;
typedef short long 			int24;
//typedef long 			int24;
typedef unsigned short long uint24;
typedef long 				int32;
typedef unsigned long 		uint32;
typedef uint8 				boolean;
typedef float 				real32;

typedef union {
	int16 i16;
	uint16 u16;
	struct {
		uint8 b0;
		uint8 b1;
	};
	struct {
		int8 pad;
		int8 i1;
	};
} i16u;

typedef union {
	int24 i24;
	uint24 u24;
	struct {
		uint8 b0;
		uint8 b1;
		uint8 b2;
	};
	struct {
		uint8 pad;
		int16 i2_1;
	};
} i24u;

typedef union {
	int32 i32;
	uint32 u32;
	struct {
		uint8 b0;
		uint8 b1;
		uint8 b2;
		uint8 b3;
	};
	struct {
		uint16 w0;
		uint16 w1;
	};
	struct {
		int16 pad;
		int16 iw1;
	};

	struct {
		uint8 pad;
		int24 i3_1;
	};
} i32u;

typedef struct {
	i32u v;
	int16 a;
	int16 f;
	uint8 dt;
	} SensorStruct;

typedef struct { // Tx
	uint8 Head, Tail;
	uint8 B[128];
	} uint8x128Q;

typedef struct { // PPM
	uint8 Head;
	int16 B[4][8];
	} int16x8x4Q;

typedef struct { // Baro
	uint8 Head, Tail;
	int24 B[8];
	} int24x8Q;

typedef struct { // GPS
	uint8 Head, Tail;
	int32 Lat[4], Lon[4];
	} int32x4Q;

// Macros
#define Set(S,b) 			((uint8)(S|=(1<<b)))
#define Clear(S,b) 			((uint8)(S&=(~(1<<b))))
#define IsSet(S,b) 			((uint8)((S>>b)&1))
#define IsClear(S,b) 		((uint8)(!(S>>b)&1))
#define Invert(S,b) 		((uint8)(S^=(1<<b)))

#define Abs(i)				(((i)<0) ? -(i) : (i))
#define Sign(i)				(((i)<0) ? -1 : 1)

#define Max(i,j) 			((i<j) ? j : i)
#define Min(i,j) 			((i<j) ? i : j )
#define Decay1(i) 			(((i) < 0) ? (i+1) : (((i) > 0) ? (i-1) : 0))

// To speed up NMEA sentence processing
// must have a positive argument
#define ConvertDDegToMPi(d) (((int32)d * 3574L)>>11)
#define ConvertMPiToDDeg(d) (((int32)d * 2048L)/3574L)

#define ToPercent(n, m) (((n)*100L)/m)

// Simple filters using weighted averaging
#define VerySoftFilter(O,N) 	(SRS16((O)+(N)*3, 2))
#define SoftFilter(O,N) 		(SRS16((O)+(N), 1))
#define MediumFilter(O,N) 		(SRS16((O)*3+(N), 2))
#define HardFilter(O,N) 		(SRS16((O)*7+(N), 3))

// Unsigned
#define VerySoftFilterU(O,N)	(((O)+(N)*3+2)>>2)
#define SoftFilterU(O,N) 		(((O)+(N)+1)>>1)
#define MediumFilterU(O,N) 		(((O)*3+(N)+2)>>2)
#define HardFilterU(O,N) 		(((O)*7+(N)+4)>>3)

#define NoFilter(O,N)			(N)

// #define DisableInterrupts 	(INTCONbits.GIEH=0)
// #define EnableInterrupts 	(INTCONbits.GIEH=1)
#define InterruptsEnabled 	(INTCONbits.GIEH)

// Clock
#ifdef CLOCK_16MHZ
	#define	TMR0_1MS		0
#else // CLOCK_40MHZ
	#define	TMR0_1MS		(65536-640) // actually 1.0248mS to clear PWM
#endif // CLOCK_16MHZ

#define _PreScale0		16				// 1 6 TMR0 prescaler
#define _PreScale1		8				// 1:8 TMR1 prescaler
#define _PreScale2		16
#define _PostScale2		16

// Parameters for UART port ClockHz/(16*(BaudRate+1))
#ifdef CLOCK_16MHZ
#define _B9600			104
#define _B38400			26
#else // CLOCK_40MHZ
#define _B9600			65
#define _B38400			65
#endif // CLOCK_16MHZ

// This is messy - trial and error to determine worst case interrupt latency!
#ifdef CLOCK_16MHZ
	#define INT_LATENCY		(uint16)(256 - 35) // x 4uS
	#define FastWriteTimer0(t) TMR0L=(uint8)t
	#define GetTimer0		Timer0.u16=(uint16)TMR0L
#else // CLOCK_40MHZ
	#define INT_LATENCY		(uint16)(65536 - 35) // x 1.6uS
	#define FastWriteTimer0(t) Timer0.u16=t;TMR0H=Timer0.b1;TMR0L=Timer0.b0
	#define GetTimer0		{Timer0.b0=TMR0L;Timer0.b1=TMR0H;}
#endif // CLOCK_16MHZ

// Bit definitions
#define Armed			(PORTAbits.RA4)

#ifdef HAVE_CUTOFF_SW
#define InTheAir		(PORTCbits.RC0) // normally open micro switch to ground
#else
#define InTheAir		true
#endif // HAVE_CUTOFF_SW

// EEPROM

#define PARAMS_ADDR_EE		0			// code assumes zero!
#define MAX_PARAMETERS	64		// parameters in EEPROM start at zero

#define STATS_ADDR_EE	 	( PARAMS_ADDR_EE + (MAX_PARAMETERS *2) )
#define MAX_STATS			64

// uses second Page of EEPROM
#define NAV_ADDR_EE			256L
// 0 - 8 not used

#define NAV_NO_WP			(NAV_ADDR_EE + 9)
#define NAV_PROX_ALT		(NAV_ADDR_EE + 10)
#define NAV_PROX_RADIUS		(NAV_ADDR_EE + 11)
#define NAV_ORIGIN_ALT		(NAV_ADDR_EE + 12)
#define NAV_ORIGIN_LAT		(NAV_ADDR_EE + 14)
#define NAV_ORIGIN_LON 		(NAV_ADDR_EE + 18)
#define NAV_RTH_ALT_HOLD 	(NAV_ADDR_EE + 22)	// P[NavRTHAlt]
#define NAV_WP_START		(NAV_ADDR_EE + 24)

#define WAYPOINT_REC_SIZE 	(4 + 4 + 2 + 1)		// Lat + Lon + Alt + Loiter
#define NAV_MAX_WAYPOINTS	((256 - 24 - 1)/WAYPOINT_REC_SIZE)

//______________________________________________________________________________________________

// main.c

#define FLAG_BYTES 8
#define TELEMETRY_FLAG_BYTES 6
typedef union {
	uint8 AllFlags[FLAG_BYTES];
	struct { // Order of these flags subject to change
		uint8
		AltHoldEnabled:1,
		AllowTurnToWP:1,			// stick programmed
		GyroFailure:1,
		LostModel:1,
		NearLevel:1,
		LowBatt:1,
		GPSValid:1,
		NavValid:1,

		BaroFailure:1,
		AccFailure:1,
		CompassFailure:1,
		GPSFailure:1,
		AttitudeHold:1,
		ThrottleMoving:1,
		HoldingAlt:1,
		Navigate:1,

		ReturnHome:1,
		WayPointAchieved:1,
		WayPointCentred:1,
		UsingGPSAlt:1,
		UsingRTHAutoDescend:1,
		BaroAltitudeValid:1,
		RangefinderAltitudeValid:1,
		UsingRangefinderAlt:1,

		// internal flags not really useful externally

		AllowNavAltitudeHold:1,	// stick programmed
		UsingPositionHoldLock:1,
		Ch5Active:1,
		Simulation:1,
		AcquireNewPosition:1,
		MotorsArmed:1,
		NavigationActive:1,
		UsingPolar:1,

		Signal:1,
		RCFrameOK:1,
		ParametersValid:1,
		RCNewValues:1,
		NewCommands:1,
		AccelerationsValid:1,
		CompassValid:1,
		CompassMissRead:1,

		UsingPolarCoordinates:1,
		ReceivingGPS:1,
		PacketReceived:1,
		NavComputed:1,
		AltitudeValid:1,
		UsingSerialPPM:1,
		UsingTxMode2:1,
		UsingAltOrientation:1,

		// outside telemetry flags

		UsingTelemetry:1,
		TxToBuffer:1,
		NewBaroValue:1,
		BeeperInUse:1,
		RFInInches:1,
		FirstArmed:1;
		};
} Flags;

enum FlightStates { Starting, Landing, Landed, Shutdown, InFlight};

extern near int8 State;

// accel.c

extern void SendCommand(int8);
extern uint8 ReadLISL(uint8);
extern uint8 ReadLISLNext(void);
extern void WriteLISL(uint8, uint8);
extern void IsLISLActive(void);

extern void ReadAccelerations(void);
extern void GetNeutralAccelerations(void);
extern void AccelerometerTest(void);
extern void InitAccelerometers(void);

extern i16u Ax, Ay, Az;
extern int8 LRIntCorr, FBIntCorr;
extern int8 NeutralLR, NeutralFB, NeutralDU;
extern int16 DUVel, LRVel, FBVel, DUAcc, LRAcc, FBAcc, DUComp, LRComp, FBComp;

//______________________________________________________________________________________________

// adc.c

#define ADC_TOP_CHANNEL		(uint8)4

#define ADCPORTCONFIG 		0b00001010 // AAAAA
#define ADCBattVoltsChan 	0
#define NonIDGADCRollChan 	1
#define NonIDGADCPitchChan 	2
#define IDGADCRollChan 		2
#define IDGADCPitchChan 	1
#define ADCAltChan 			3 	// Altitude
#define ADCYawChan			4
#define TopADCChannel		4

extern int16 ADC(uint8);
extern void InitADC(void);

extern SensorStruct ADCVal[];
extern uint8 ADCChannel;

//______________________________________________________________________________________________

// autonomous.c

extern void FailsafeHoldPosition(void);
extern void DoPolarOrientation(void);
extern void Navigate(int32, int32);
extern void SetDesiredAltitude(int16);
extern void DoFailsafeLanding(void);
extern void AcquireHoldPosition(void);
extern void NavGainSchedule(int16);
extern void DoNavigation(void);
extern void FakeFlight(void);
extern void DoPPMFailsafe(void);
extern void WriteWayPointEE(uint8, int32, int32, int16, uint8);
extern void UAVXNavCommand(void);
extern void GetWayPointEE(int8);
extern void InitNavigation(void);

typedef struct { int32 E, N; int16 A; uint8 L; } WayPoint;

enum NavStates { HoldingStation, ReturningHome, AtHome, Descending, Touchdown, Navigating, Loitering};
enum FailStates { MonitoringRx, Aborting, Terminating, Terminated };

extern int16 NavRCorr, NavPCorr;

#ifdef SIMULATE
extern int16 FakeDesiredPitch, FakeDesiredRoll, FakeDesiredYaw, FakeHeading;
#endif // SIMULATE

extern near int8 FailState;
extern WayPoint WP;
extern int8 CurrWP;
extern int8 NoOfWayPoints;
extern int16 WPAltitude;
extern int32 WPLatitude, WPLongitude;
extern int16 WayHeading;
extern int16 NavClosingRadius, NavNeutralRadius, NavCloseToNeutralRadius, NavProximityRadius, NavProximityAltitude;
extern int16 CompassOffset;
extern int24 NavRTHTimeoutmS;
extern int8 NavState;
extern int16 NavSensitivity, RollPitchMax;
extern int16 AltSum;

extern int16 NavRCorr, NavRCorrP, NavPCorr, NavPCorrP, NavYCorr, SumNavYCorr;
extern int8 NavYCorrLimit;
extern int16 EffNavSensitivity;
extern int16 EastP, EastDiffSum, EastI, EastCorr, NorthP, NorthDiffSum, NorthI, NorthCorr;
extern int24 EastD, EastDiffP, NorthD, NorthDiffP;

//______________________________________________________________________________________________

// baro.c

#define BARO_INIT_RETRIES	10	// max number of initialisation retries

enum BaroTypes { BaroBMP085, BaroSMD500, BaroMPX4115, BaroUnknown };

extern void SetFreescaleMCP4725(int16);
extern void SetFreescaleOffset(void);
extern void ReadFreescaleBaro(void);
extern int16 FreescaleToDM(int24);
extern void GetFreescaleBaroAltitude(void);
extern boolean IsFreescaleBaroActive(void);
extern void InitFreescaleBarometer(void);

extern void StartBoschBaroADC(boolean);
extern void ReadBoschBaro(void);
extern int24 CompensatedBoschPressure(uint16, uint16);
extern void GetBoschBaroAltitude(void);
extern boolean IsBoschBaroActive(void);
extern void InitBoschBarometer(void);

extern void GetBaroAltitude(void);
extern void InitBarometer(void);

extern void ShowBaroType(void);
extern void BaroTest(void);

extern int32 OriginBaroPressure, CompBaroPressure;
extern uint16 BaroPressure, BaroTemperature;
extern boolean AcquiringPressure;
extern int24 BaroRelAltitude, BaroRelAltitudeP;
extern int16 BaroROC;
extern i16u	BaroVal;
extern int8 BaroType;
extern int16 AltitudeUpdateRate;
extern int8	BaroRetries;
extern i32u BaroValF;
extern int16 BaroFilterA;

#ifdef SIMULATE
extern int24 FakeBaroRelAltitude;
#endif // SIMULATE

//______________________________________________________________________________________________

// compass.c

#define COMPASS_I2C_ID		0x42		// I2C slave address
#define COMPASS_MAXDEV		30			// maximum yaw compensation of compass heading
#define COMPASS_MIDDLE		10			// yaw stick neutral dead zone
#define COMPASS_TIME_MS		50			// 20Hz

extern int16 GetCompass(void);
extern void GetHeading(void);
extern void GetCompassParameters(void);
extern void DoCompassTest(void);
extern void CalibrateCompass(void);
extern void InitHeading(void);
extern void InitCompass(void);

extern i24u Compass;
extern int16 HeadingFilterA;
extern i32u HeadingValF;

//______________________________________________________________________________________________

// control.c

extern void DoAltitudeHold(int24, int16);
extern void UpdateAltitudeSource(void);
extern void AltitudeHold(void);

extern void LimitRollSum(void);
extern void LimitPitchSum(void);
extern void LimitYawSum(void);
extern void InertialDamping(void);
extern void DoOrientationTransform(void);
extern void DoControl(void);

extern void LightsAndSirens(void);
extern void InitControl(void);

extern int16 RE, PE, YE, HE;					// gyro rate error
extern int16 REp, PEp, YEp;				// previous error for derivative
extern int16 Rl, Pl, Yl, Ylp;							// PID output values
extern int16 RollSum, PitchSum, YawSum;			// integral/angle
extern int16 RollTrim, PitchTrim, YawTrim;
extern int16 HoldYaw, YawSlewLimit;
extern int16 YawFilterA;
extern int16 RollIntLimit256, PitchIntLimit256, YawIntLimit256;
extern int16 CruiseThrottle, DesiredThrottle, IdleThrottle, InitialThrottle, StickThrottle;
extern int16 DesiredRoll, DesiredPitch, DesiredYaw, DesiredHeading, DesiredCamPitchTrim, Heading;
extern int16 ControlRoll, ControlPitch, ControlRollP, ControlPitchP;
extern int16 CurrMaxRollPitch;
extern int16 ThrLow, ThrHigh, ThrNeutral;
extern int16 AltComp, AltDiffSum, AltD, AltDSum;
extern int16 AttitudeHoldResetCount;
extern int24 DesiredAltitude, Altitude;
extern int16 ROC;
extern boolean FirstPass;

//______________________________________________________________________________________________

// eeprom.c

extern int8 ReadEE(uint16);
extern int16 Read16EE(uint16);
extern int32 Read32EE(uint16);
extern void WriteEE(uint16, int8);
extern void Write16EE(uint16, int16);
extern void Write32EE(uint16, int32);

//______________________________________________________________________________________________

// gps.c

extern void UpdateField(void);
extern int32 ConvertGPSToM(int32);
extern int32 ConvertMToGPS(int32);
extern int24 ConvertInt(uint8, uint8);
extern int32 ConvertLatLonM(uint8, uint8);
extern int32 ConvertUTime(uint8, uint8);
extern void ParseGPRMCSentence(void);
extern void ParseGPGGASentence(void);
extern void SetGPSOrigin(void);
extern void ParseGPSSentence(void);
extern void GPSTest(void);
extern void UpdateGPS(void);
extern void InitGPS(void);

#define MAXTAGINDEX 		4
#define GPSRXBUFFLENGTH 	80
typedef struct {
		uint8 	s[GPSRXBUFFLENGTH];
		uint8 	length;
	} NMEAStruct;

extern NMEAStruct NMEA;

extern const rom uint8 NMEATag[];

extern int32 GPSMissionTime, GPSStartTime;
extern int32 GPSLatitude, GPSLongitude;
extern int32 OriginLatitude, OriginLongitude;
extern int24 GPSAltitude, GPSRelAltitude, GPSOriginAltitude;
extern int32 DesiredLatitude, DesiredLongitude;
extern int32 LatitudeP, LongitudeP, HoldLatitude, HoldLongitude;
extern int16 GPSLongitudeCorrection;
extern int16 GPSVel, GPSROC;
extern int8 GPSNoOfSats;
extern int8 GPSFix;
extern int16 GPSHDilute;
extern uint8 nll, cc, lo, hi;
extern boolean EmptyField;
extern int16 ValidGPSSentences;
extern int32 SumGPSRelAltitude, SumBaroRelAltitude;

#ifdef SIMULATE
extern int32 FakeGPSLongitude, FakeGPSLatitude;
#endif // SIMULATE

//______________________________________________________________________________________________

// gyro.c

extern void ShowGyroType(uint8);
extern void CompensateRollPitchGyros(void);
extern void GetGyroValues(void);
extern void CalculateGyroRates(void);
extern void CheckGyroFault(uint8, uint8, uint8);
extern void ErectGyros(void);
extern void GyroTest(void);
extern void InitGyros(void);

extern void ITG3200ViewRegisters(void);
extern void BlockReadITG3200(void);
extern uint8 ReadByteITG3200(uint8);
extern void WriteByteITG3200(uint8, uint8);
extern void InitITG3200(void);

extern int16 GyroMidRoll, GyroMidPitch, GyroMidYaw;
extern int16 RollRate, PitchRate, YawRate;
extern i32u YawRateF;
extern int16 RollRateADC, PitchRateADC, YawRateADC;

//______________________________________________________________________________________________

// irq.c

#define CONTROLS 			7
#define MAX_CONTROLS 		12 	// maximum Rx channels

#define RxFilter			MediumFilterU
//#define RxFilter			SoftFilterU
//#define RxFilter			NoFilter

#define	RC_GOOD_BUCKET_MAX	20
#define RC_GOOD_RATIO		4

#define RC_MINIMUM			0

#ifdef CLOCK_16MHZ
	#define RC_MAXIMUM		238
#else
	#define RC_MAXIMUM		240	// adjust for faster arithmetic in RCMap
#endif // CLOCK_40MHZ

#define RC_NEUTRAL			((RC_MAXIMUM-RC_MINIMUM+1)/2)

#define RC_MAX_ROLL_PITCH	(170)

#define RC_THRES_STOP		((6L*RC_MAXIMUM)/100)
#define RC_THRES_START		((10L*RC_MAXIMUM)/100)

#define RC_FRAME_TIMEOUT_MS 	25
#define RC_SIGNAL_TIMEOUT_MS 	(5L*RC_FRAME_TIMEOUT_MS)
#define RC_THR_MAX 			RC_MAXIMUM

#define MAX_ROLL_PITCH		RC_NEUTRAL	// Rx stick units - rely on Tx Rate/Exp

#ifdef RX6CH
	#define RC_CONTROLS 5
#else
	#define RC_CONTROLS CONTROLS
#endif //RX6CH

extern void SyncToTimer0AndDisableInterrupts(void);
extern void ReceivingGPSOnly(uint8);
extern void InitTimersAndInterrupts(void);
extern void ReceivingGPSOnly(uint8);
extern int24 mSClock(void);

enum { Clock, GeneralCountdown, UpdateTimeout, RCSignalTimeout, BeeperTimeout, ThrottleIdleTimeout,
	FailsafeTimeout, AbortTimeout, NavStateTimeout, LastValidRx, LastGPS, StartTime,
	GPSTimeout, GPSROCUpdate, LEDChaserUpdate, LastBattery, TelemetryUpdate, RangefinderROCUpdate, NavActiveTime,
	ThrottleUpdate, VerticalDampingUpdate, BaroUpdate, CompassUpdate};

enum WaitStates { WaitSentinel, WaitTag, WaitBody, WaitCheckSum};

extern int24 mS[];
extern int16 RC[];

extern near i16u PPM[];
extern near int8 PPM_Index;
extern near int24 PrevEdge, CurrEdge;
extern near uint8 Intersection, PrevPattern, CurrPattern;
extern near i16u Width, Timer0;
extern near int24 PauseTime; // for tests
extern near uint8 RxState;
extern near uint8 ll, tt, gps_ch;
extern near uint8 RxCheckSum, GPSCheckSumChar, GPSTxCheckSum;
extern near boolean WaitingForSync;

extern int8	SignalCount;
extern uint16 RCGlitches;

//______________________________________________________________________________________________

// i2c.c

#define	I2C_ACK			((uint8)(0))
#define	I2C_NACK		((uint8)(1))

#define SPI_CS			PORTCbits.RC5
#define SPI_SDA			PORTCbits.RC4
#define SPI_SCL			PORTCbits.RC3
#define SPI_IO			TRISCbits.TRISC4

#define	RD_SPI			1
#define WR_SPI			0
#define DSEL_LISL  		1
#define SEL_LISL  		0

extern void InitI2C(uint8, uint8);
extern boolean I2CWaitClkHi(void);
extern void I2CStart(void);
extern void I2CStop(void);
extern uint8 WriteI2CByte(uint8);
extern uint8 ReadI2CByte(uint8);
extern uint8 ReadI2CString(uint8 *, uint8);
extern uint8 ScanI2CBus(void);

extern boolean ESCWaitClkHi(void);
extern void ESCI2CStart(void);
extern void ESCI2CStop(void);
extern uint8 WriteESCI2CByte(uint8);

extern void ProgramSlaveAddress(uint8);
extern void ConfigureESCs(void);

//______________________________________________________________________________________________

// leds.c

#define AUX2M			0x01
#define BlueM			0x02
#define RedM			0x04
#define GreenM			0x08
#define AUX1M			0x10
#define YellowM			0x20
#define AUX3M			0x40
#define BeeperM			0x80

#define ALL_LEDS_ON		LEDsOn(BlueM|RedM|GreenM|YellowM)
#define AUX_LEDS_ON		LEDsOn(AUX1M|AUX2M|AUX3M)

#define ALL_LEDS_OFF	LEDsOff(BlueM|RedM|GreenM|YellowM)
#define AUX_LEDS_OFF	LEDsOff(AUX1M|AUX2M|AUX3M)

#define ALL_LEDS_ARE_OFF	( (LEDShadow&(BlueM|RedM|GreenM|YellowM))== (uint8)0 )

#define LEDRed_ON		LEDsOn(RedM)
#define LEDBlue_ON		LEDsOn(BlueM)
#define LEDGreen_ON		LEDsOn(GreenM)
#define LEDYellow_ON	LEDsOn(YellowM)
#define LEDAUX1_ON		LEDsOn(AUX1M)
#define LEDAUX2_ON		LEDsOn(AUX2M)
#define LEDAUX3_ON		LEDsOn(AUX3M)
#define LEDRed_OFF		LEDsOff(RedM)
#define LEDBlue_OFF		LEDsOff(BlueM)
#define LEDGreen_OFF	LEDsOff(GreenM)
#define LEDYellow_OFF	LEDsOff(YellowM)
#define LEDYellow_TOG	if( (LEDShadow&YellowM) == (uint8)0 ) LEDsOn(YellowM); else LEDsOff(YellowM)
#define LEDRed_TOG		if( (LEDShadow&RedM) == (uint8)0 ) LEDsOn(RedM); else LEDsOff(RedM)
#define LEDBlue_TOG		if( (LEDShadow&BlueM) == (uint8)0 ) LEDsOn(BlueM); else LEDsOff(BlueM)
#define LEDGreen_TOG	if( (LEDShadow&GreenM) == (uint8)0 ) LEDsOn(GreenM); else LEDsOff(GreenM)
#define Beeper_OFF		LEDsOff(BeeperM)
#define Beeper_ON		LEDsOn(BeeperM)
#define Beeper_TOG		if( (LEDShadow&BeeperM) == (uint8)0 ) LEDsOn(BeeperM); else LEDsOff(BeeperM)

extern void SendLEDs(void);
extern void LEDsOn(uint8);
extern void LEDsOff(uint8);
extern void LEDChaser(void);

extern uint8 LEDShadow, SaveLEDs, LEDPattern;

//______________________________________________________________________________________________

// math.c

extern int16 SRS16(int16, uint8);
extern int32 SRS32(int32, uint8);
extern int16 Make2Pi(int16);
extern int16 MakePi(int16);
extern int16 Table16(int16, const int16 *);
extern int16 int16sin(int16);
extern int16 int16cos(int16);
extern int16 int32atan2(int32, int32);
extern int16 int16sqrt(int16);
extern int32 int32sqrt(int32);

//______________________________________________________________________________________________

// menu.c

extern void ShowPrompt(void);
extern void ShowRxSetup(void);
extern void ShowSetup(boolean);
extern void ProcessCommand(void);

extern const rom uint8 SerHello[];
extern const rom uint8 SerSetup[];
extern const rom uint8 SerPrompt[];

extern const rom uint8 RxChMnem[];

//______________________________________________________________________________________________

// outputs.c

#define OUT_MINIMUM			1			// Required for PPM timing loops
#define OUT_MAXIMUM			200			// to reduce Rx capture and servo pulse output interaction
#define OUT_NEUTRAL			105			// 1.503mS @ 105 16MHz
#define OUT_HOLGER_MAXIMUM	225
#define OUT_YGEI2C_MAXIMUM	240
#define OUT_X3D_MAXIMUM		200

extern uint8 SaturInt(int16);
extern void DoMulticopterMix(int16 CurrThrottle);
extern void CheckDemand(int16 CurrThrottle);
extern void MixAndLimitMotors(void);
extern void MixAndLimitCam(void);
extern void OutSignals(void);
extern void InitI2CESCs(void);
extern void StopMotors(void);
extern void InitMotors(void);

enum PWMTags1 {FrontC=0, BackC, RightC, LeftC, CamRollC, CamPitchC}; // order is important for X3D & Holger ESCs
enum PWMTags2 {ThrottleC=0, AileronC, ElevatorC, RudderC};
enum PWMTags3 {RightElevonC=1, LeftElevonC=2};
enum PWMTags4 {K1=0, K2, K3, K4, K5, K6};
#define NoOfPWMOutputs 			4
#define NoOfI2CESCOutputs 		4

extern int16 PWM[6];
extern int16 PWMSense[6];
extern int16 ESCI2CFail[4];
extern int16 CurrThrottle;
extern int8 ServoInterval;

extern near uint8 SHADOWB, PWM0, PWM1, PWM2, PWM4, PWM5;
extern near int8 ServoToggle;

extern int16 ESCMin, ESCMax;

//______________________________________________________________________________________________

// params.c

extern void ReadParametersEE(void);
extern void WriteParametersEE(uint8);
extern void UseDefaultParameters(void);
extern void UpdateParamSetChoice(void);
extern boolean ParameterSanityCheck(void);
extern void InitParameters(void);

enum TxRxTypes {
	FutabaCh3, FutabaCh2, FutabaDM8, JRPPM, JRDM9, JRDXS12,
	DX7AR7000, DX7AR6200, FutabaCh3_6_7, DX7AR6000, GraupnerMX16s, DX6iAR6200, FutabaCh3_R617FS, DX7aAR7000, CustomTxRx };
enum RCControls {ThrottleC, RollC, PitchC, YawC, RTHC, CamPitchC, NavGainC};
enum ESCTypes { ESCPPM, ESCHolger, ESCX3D, ESCYGEI2C };
enum GyroTypes { Gyro300D5V, Gyro150D5V, IDG300, Gyro300D3V, CustomGyro};
enum AFs { QuadAF, TriAF, HexAF, HeliAF, ElevAF, AilAF };

enum Params { // MAX 64
	RollKp, 			// 01
	RollKi,				// 02
	RollKd,				// 03
	HorizDampKp,		// 04c
	RollIntLimit,		// 05
	PitchKp,			// 06
	PitchKi,			// 07
	PitchKd,			// 08
	AltKp,				// 09c
	PitchIntLimit,		// 10

	YawKp, 				// 11
	YawKi,				// 12
	YawKd,				// 13
	YawLimit,			// 14
	YawIntLimit,		// 15
	ConfigBits,			// 16c
	TimeSlots,			// 17c
	LowVoltThres,		// 18c
	CamRollKp,			// 19
	PercentCruiseThr,	// 20c

	VertDampKp,			// 21c
	MiddleDU,			// 22c
	PercentIdleThr,		// 23c
	MiddleLR,			// 24c
	MiddleFB,			// 25c
	CamPitchKp,			// 26
	CompassKp,			// 27
	AltKi,				// 28c
	NavRadius,			// 29
	NavKi,				// 30

	unused1,			// 31
	unused2,			// 32
	NavRTHAlt,			// 33
	NavMagVar,			// 34c
	GyroRollPitchType,	// 35c
	ESCType,			// 36c
	TxRxType,			// 37c
	NeutralRadius,		// 38
	PercentNavSens6Ch,	// 39
	CamRollTrim,		// 40c
	NavKd,				// 41
	VertDampDecay,		// 42c
	HorizDampDecay,		// 43c
	BaroScale,			// 44c
	TelemetryType,		// 45c
	MaxDescentRateDmpS,	// 46
	DescentDelayS,		// 47c
	NavIntLimit,		// 48
	AltIntLimit,		// 49
	UnusedGravComp,		// 50c
	CompSteps,			// 51c
	ServoSense,			// 52c
	CompassOffsetQtr,	// 53c
	BatteryCapacity,	// 54c
	GyroYawType,		// 55c
	AltKd,				// 56
	Orient,				// 57
	NavYawLimit			// 58

	// 56 - 64 unused currently
	};

#define FlyXMode 			0
#define FlyAltOrientationMask 		0x01

#define UsePositionHoldLock 0
#define UsePositionHoldLockMask 	0x01

#define UseRTHDescend 		1
#define	UseRTHDescendMask	0x02

#define TxMode2 			2
#define TxMode2Mask 		0x04

#define RxSerialPPM 		3
#define RxSerialPPMMask		0x08

#define RFInches 		4
#define RFInchesMask		0x10

// bit 4 is pulse polarity for 3.15

#define UseGPSAlt 			5
#define	UseGPSAltMask		0x20

#define UsePolar 			6
#define	UsePolarMask		0x40

// bit 7 unusable in UAVPSet

extern const rom int8 ComParms[];
extern const rom int8 DefaultParams[];

extern const rom uint8 ESCLimits [];


extern int16 OSin[], OCos[];
extern uint8 Orientation, PolarOrientation;

extern uint8 ParamSet;
extern boolean ParametersChanged, SaveAllowTurnToWP;
extern int8 P[];

extern uint8 UAVXAirframe;

//__________________________________________________________________________________________

// rangefinder.c

extern void GetRangefinderAltitude(void);
extern void InitRangefinder(void);

extern int16 RangefinderAltitude, RangefinderAltitudeP, RangefinderROC;

//__________________________________________________________________________________________

// rc.c

extern void DoRxPolarity(void);
extern void InitRC(void);
extern void MapRC(void);
extern void UpdateControls(void);
extern void CaptureTrims(void);
extern void CheckThrottleMoved(void);

extern const rom boolean PPMPosPolarity[];
extern const rom uint8 Map[CustomTxRx+1][CONTROLS];
extern int8 RMap[];

#define PPMQMASK 3
extern int16 PPMQSum[];
extern int16x8x4Q PPMQ;

//__________________________________________________________________________________________

// serial.c

extern void TxString(const rom uint8*);
extern void TxChar(uint8);
extern void TxValU(uint8);
extern void TxValS(int8);
extern void TxNextLine(void);
extern void TxNibble(uint8);
extern void TxValH(uint8);
extern void TxValH16(uint16);
extern uint8 PollRxChar(void);
extern uint8 RxChar(void);
extern uint8 RxNumU(void);
extern int8 RxNumS(void);
extern void TxVal32(int32, int8, uint8);
extern void SendByte(uint8);
extern void TxESCu8(uint8);
extern void TxESCi8(int8);
extern void TxESCi16(int16);
extern void TxESCi24(int24);
extern void TxESCi32(int32);
extern void SendPacket(uint8, uint8, uint8 *, boolean);

#define TX_BUFF_MASK	127
extern uint8 	TxCheckSum;
extern uint8x128Q 	TxQ;

//______________________________________________________________________________________________

// stats.c

extern void ZeroStats(void);
extern void ReadStatsEE(void);
extern void WriteStatsEE(void);
extern void ShowStats(void);

enum Statistics {
	GPSAltitudeS, BaroRelAltitudeS, ESCI2CFailS, GPSMinSatsS, MinBaroROCS, MaxBaroROCS, GPSVelS,
	AccFailS, CompassFailS, BaroFailS, GPSInvalidS, GPSMaxSatsS, NavValidS,
	MinHDiluteS, MaxHDiluteS, RCGlitchesS, GPSBaroScaleS, GyroFailS, RCFailsafesS, I2CFailS, MinTempS, MaxTempS, BadS, BadNumS}; // NO MORE THAN 32 or 64 bytes

extern int16 Stats[];

//______________________________________________________________________________________________

// telemetry.c

extern void SendTelemetry(void);
extern void SendUAVX(void);
extern void SendUAVXControl(void);
extern void SendFlightPacket(void);
extern void SendNavPacket(void);
extern void SendControlPacket(void);
extern void SendStatsPacket(void);
extern void SendArduStation(void);
extern void SendCustom(void);
extern void SensorTrace(void);
extern void CheckTelemetry(void);

extern uint8 UAVXCurrPacketTag;

enum PacketTags {UnknownPacketTag = 0, LevPacketTag, NavPacketTag, MicropilotPacketTag, WayPacketTag,
	AirframePacketTag, NavUpdatePacketTag, BasicPacketTag, RestartPacketTag, TrimblePacketTag,
	MessagePacketTag, EnvironmentPacketTag, BeaconPacketTag, UAVXFlightPacketTag,
	UAVXNavPacketTag, UAVXStatsPacketTag, UAVXControlPacketTag};

enum TelemetryTypes { NoTelemetry, GPSTelemetry, UAVXTelemetry, UAVXControlTelemetry, ArduStationTelemetry, CustomTelemetry };

//______________________________________________________________________________________________

// temperature.c

extern void GetTemperature(void);
extern void InitTemperature(void);

extern i16u AmbientTemperature;

//______________________________________________________________________________________________

// utils.c

extern void InitPorts(void);
extern void InitPortsAndUSART(void);
extern void InitMisc(void);
extern void Delay1mS(int16);
extern void Delay100mSWithOutput(int16);
extern void DoBeep100mSWithOutput(uint8, uint8);
extern void DoStartingBeepsWithOutput(uint8);
extern int32 SlewLimit(int32, int32, int32);
extern int32 ProcLimit(int32, int32, int32);
extern int16 DecayX(int16, int16);
extern void LPFilter16(int16*, i32u*, int16);
extern void LPFilter24(int24* i, i32u* iF, int16 FilterA);
extern void CheckAlarms(void);

extern int16 BatteryVoltsADC, BatteryCurrentADC, BatteryVoltsLimitADC, BatteryCurrentADCEstimated, BatteryChargeUsedmAH;
extern int32 BatteryChargeADC, BatteryCurrent;

//______________________________________________________________________________________________

// bootl18f.asm

extern void BootStart(void);

//______________________________________________________________________________________________

// tests.c

extern void DoLEDs(void);
extern void ReceiverTest(void);
extern void PowerOutput(int8);
extern void LEDsAndBuzzer(void);
extern void BatteryTest(void);

//______________________________________________________________________________________________

// Sanity checks

// check the Rx and PPM ranges
#if ( OUT_MINIMUM >= OUT_MAXIMUM )
#error OUT_MINIMUM < OUT_MAXIMUM!
#endif
#if (OUT_MAXIMUM < OUT_NEUTRAL)
#error OUT_MAXIMUM < OUT_NEUTRAL !
#endif

#if RC_MINIMUM >= RC_MAXIMUM
#error RC_MINIMUM < RC_MAXIMUM!
#endif
#if (RC_MAXIMUM < RC_NEUTRAL)
#error RC_MAXIMUM < RC_NEUTRAL !
#endif

#if (( defined TRICOPTER + defined QUADROCOPTER + defined HEXACOPTER + defined HELICOPTER + defined AILERON + defined ELEVON ) != 1)
#error None or more than one aircraft configuration defined !
#endif

