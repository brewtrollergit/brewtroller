#ifndef BREWTROLLER_H
#define BREWTROLLER_H

#define BUILDNUM 3

#include <Arduino.h>
#include <pin.h>
#include <PID_Beta6.h>

#include "HardwareProfile.h"
#include "Temp.h"
#include "PVOut.h"
#include "UI_LCD.h"

const extern void(* softReset) (void);

extern const char BT[];
extern const char BTVER[];

//**********************************************************************************
// Compile Time Logic
//**********************************************************************************

//Enable Mash Avergaing Logic if any Mash_AVG_AUXx options were enabled
#if defined MASH_AVG_AUX1 || defined MASH_AVG_AUX2 || defined MASH_AVG_AUX3
  #define MASH_AVG
#endif

#ifdef USEMETRIC
  #define SETPOINT_MULT 50
  #define SETPOINT_DIV 2
#else
#define SETPOINT_MULT 100
#define SETPOINT_DIV 1
#endif

#ifndef STRIKE_TEMP_OFFSET
  #define STRIKE_TEMP_OFFSET 0
#endif

#if COM_SERIAL0 == BTNIC || defined BTNIC_EMBEDDED
#define BTNIC_PROTOCOL
#endif

#if defined BTPD_SUPPORT || defined UI_LCD_I2C || defined TS_ONEWIRE_I2C || defined BTNIC_EMBEDDED || defined RGBIO8_ENABLE
#define USE_I2C
#endif

#ifdef BOIL_OFF_GALLONS
  #ifdef USEMETRIC
    #define EvapRateConversion 1000
  #else
    #define EvapRateConversion 100
  #endif
#endif

struct ProgramThread {
    byte activeStep;
    byte recipe;
};

//**********************************************************************************
// Globals
//**********************************************************************************
//Heat Output Pin Array
extern pin heatPin[4], alarmPin;

#ifdef DIGITAL_INPUTS
extern pin digInPin[DIGIN_COUNT];
#endif

extern pin * TriggerPin[5];
extern boolean estop;

#ifdef HEARTBEAT
extern pin hbPin;
#endif

//Volume Sensor Pin Array
extern byte vSensor[3];

//8-byte Temperature Sensor Address x9 Sensors
extern byte tSensor[NUM_TS][TEMP_ADDR_SIZE];
extern int temp[NUM_TS];

//Volume in (thousandths of gal/l)
extern unsigned long tgtVol[3], volAvg[3], calibVols[3][10];
extern unsigned int calibVals[3][10];
#ifdef SPARGE_IN_PUMP_CONTROL
extern unsigned long prevSpargeVol[2];
#endif

#ifdef HLT_MIN_REFILL
extern unsigned long SpargeVol;
#endif

#ifdef FLOWRATE_CALCS
//Flowrate in thousandths of gal/l per minute
extern long flowRate[3];
#endif


//Create the appropriate 'LCD' object for the hardware configuration (4-Bit GPIO, I2C)
#if defined UI_LCD_4BIT
  #include <LiquidCrystalFP.h>

  #ifndef UI_DISPLAY_SETUP
    extern LCD4Bit LCD;
  #else
    extern LCD4Bit LCD;
  #endif

#elif defined UI_LCD_I2C
extern LCDI2C LCD;
#endif

//Valve Variables
extern unsigned long vlvConfig[NUM_VLVCFGS], actProfiles;
extern boolean autoValve[NUM_AV];

#if !defined(NUM_MODBUS_RELAY_BOARDS) 
  #define NUM_MODBUS_RELAY_BOARDS 0
#endif
#if NUM_MODBUS_RELAY_BOARDS == 0 && defined(PVOUT_TYPE_MODBUS)
  #undef PVOUT_TYPE_MODBUS
#endif

#define NUM_OUTPUT_BANKS 1+NUM_MODBUS_RELAY_BOARDS

extern OutputBank* outputBanks[NUM_OUTPUT_BANKS];

//Shared buffers
extern char buf[20];

//Output Globals
extern double PIDInput[4], PIDOutput[4], setpoint[4];
#ifdef PID_FEED_FORWARD
extern double FFBias;
#endif
extern byte PIDCycle[4], hysteresis[4];
#ifdef PWM_BY_TIMER
extern unsigned int cycleStart[4];
#else
extern unsigned long cycleStart[4];
#endif
extern boolean heatStatus[4], PIDEnabled[4];
extern unsigned int steamPSens, steamZero;

extern byte pidLimits[4];

//Steam Pressure in thousandths
extern unsigned long steamPressure;
extern byte boilPwr;

extern PID pid[4];
#if defined PID_FLOW_CONTROL && defined PID_CONTROL_MANUAL
  extern unsigned long nextcompute;
  extern byte additioncount[2];
#endif

#ifdef RIMS_MLT_SETPOINT_DELAY
  extern byte steptoset;
  extern byte RIMStimeExpired;
  extern unsigned long starttime;
  extern unsigned long timetoset;
#endif

//Timer Globals
#define NUM_TIMERS 2
extern unsigned long timerValue[NUM_TIMERS], lastTime[NUM_TIMERS];
extern boolean timerStatus[NUM_TIMERS], alarmStatus;

//Log Globals
extern boolean logData;

//Brew Step Logic Globals
extern boolean preheated[4];
extern ControlState boilControlState;

//Bit 1 = Boil; Bit 2-11 (See Below); Bit 12 = End of Boil; Bit 13-15 (Open); Bit 16 = Preboil (If Compile Option Enabled)
extern unsigned int hoptimes[11];
extern byte pitchTemp;

//Log Strings
extern const char LOGCMD[];
extern const char LOGDEBUG[];
extern const char LOGSYS[];
extern const char LOGCFG[];
extern const char LOGDATA[];

//PWM by timer globals
#ifdef PWM_BY_TIMER
extern unsigned int timer1_overflow_count = 0;
extern unsigned int PIDOutputCountEquivalent[4][2] = {{0,0},{0,0},{0,0},{0,0}};
#endif

#endif
