/*
  Copyright (C) 2009, 2010 Matt Reba, Jeremiah Dillingham

    This file is part of BrewTroller.

    BrewTroller is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BrewTroller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BrewTroller.  If not, see <http://www.gnu.org/licenses/>.


BrewTroller - Open Source Brewing Computer
Software Lead: Matt Reba (matt_AT_brewtroller_DOT_com)
Hardware Lead: Jeremiah Dillingham (jeremiah_AT_brewtroller_DOT_com)

Documentation, Forums and more information available at http://www.brewtroller.com
*/

/*
Compiled on Arduino-0022 (http://arduino.cc/en/Main/Software)
  With Sanguino Software "Sanguino-0018r2_1_4.zip" (http://code.google.com/p/sanguino/downloads/list)

  Using the following libraries:
    PID  v0.6 (Beta 6) (http://www.arduino.cc/playground/Code/PIDLibrary)
    OneWire 2.0 (http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip)
    Encoder by CodeRage ()
    FastPin and modified LiquidCrystal with FastPin by CodeRage (http://www.brewtroller.com/forum/showthread.php?t=626)
*/



//*****************************************************************************************************************************
// BEGIN CODE
//*****************************************************************************************************************************

#include <avr/pgmspace.h>
#include <PID_Beta6.h>
#include <pin.h>
#include <menu.h>
#include <ModbusMaster.h>

#include "HardwareProfile.h"
#include "Config.h"
#include "Enum.h"
#include "PVOut.h"
#include "UI_LCD.h"
#include <avr/eeprom.h>
#include <EEPROM.h>
#include <wiring_private.h>
#include <encoder.h>
#include "Com_RGBIO8.h"
#include "UI.h"
#include "StepLogic.h"
#include "Outputs.h"
#include "Events.h"
#include "EEPROM.h"
#include "Temp.h"
#include "BrewCore.h"
#include "Com.h"
#include "BrewTroller.h"

const void(* softReset) (void) = 0;
const char BT[] PROGMEM = "BrewTroller";
const char BTVER[] PROGMEM = "2.7";

//**********************************************************************************
// Compile Time Logic
//**********************************************************************************

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

#ifdef USE_I2C
#include <Wire.h>
#endif

//**********************************************************************************
// Globals
//**********************************************************************************
//Heat Output Pin Array
pin heatPin[4], alarmPin;

#ifdef DIGITAL_INPUTS
pin digInPin[DIGIN_COUNT];
#endif

pin * TriggerPin[5] = { NULL, NULL, NULL, NULL, NULL };
boolean estop = 0;

#ifdef HEARTBEAT
pin hbPin;
#endif

//Volume Sensor Pin Array
#ifdef HLT_AS_KETTLE
  byte vSensor[3] = { HLTVOL_APIN, MASHVOL_APIN, HLTVOL_APIN};
#elif defined KETTLE_AS_MASH
  byte vSensor[3] = { HLTVOL_APIN, KETTLEVOL_APIN, KETTLEVOL_APIN};
#elif defined SINGLE_VESSEL_SUPPORT
  byte vSensor[3] = { HLTVOL_APIN, HLTVOL_APIN, HLTVOL_APIN};
#else
byte vSensor[3] = { HLTVOL_APIN, MASHVOL_APIN, KETTLEVOL_APIN};
#endif

//8-byte Temperature Sensor Address x9 Sensors
byte tSensor[NUM_TS][TEMP_ADDR_SIZE];
int temp[NUM_TS];

//Volume in (thousandths of gal/l)
unsigned long tgtVol[3], volAvg[3], calibVols[3][10];
unsigned int calibVals[3][10];
#ifdef SPARGE_IN_PUMP_CONTROL
unsigned long prevSpargeVol[2] = {0,0};
#endif

#ifdef HLT_MIN_REFILL
unsigned long SpargeVol = 0;
#endif

#ifdef FLOWRATE_CALCS
//Flowrate in thousandths of gal/l per minute
long flowRate[3] = {0,0,0};
#endif


//Create the appropriate 'LCD' object for the hardware configuration (4-Bit GPIO, I2C)
#if defined UI_LCD_4BIT
  #include <LiquidCrystalFP.h>
  
  #ifndef UI_DISPLAY_SETUP
    LCD4Bit LCD(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DATA4_PIN, LCD_DATA5_PIN, LCD_DATA6_PIN, LCD_DATA7_PIN);
  #else
    LCD4Bit LCD(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DATA4_PIN, LCD_DATA5_PIN, LCD_DATA6_PIN, LCD_DATA7_PIN, LCD_BRIGHT_PIN, LCD_CONTRAST_PIN);
  #endif
  
#elif defined UI_LCD_I2C
LCDI2C LCD(UI_LCD_I2CADDR);
#endif



//Valve Variables
unsigned long vlvConfig[NUM_VLVCFGS], actProfiles;
boolean autoValve[NUM_AV];

//Output Banks
OutputBank* outputBanks[NUM_OUTPUT_BANKS];

//Shared buffers
char buf[20];

//Output Globals
double PIDInput[4], PIDOutput[4], setpoint[4];
#ifdef PID_FEED_FORWARD
double FFBias;
#endif
byte PIDCycle[4], hysteresis[4];
#ifdef PWM_BY_TIMER
unsigned int cycleStart[4] = {0,0,0,0};
#else
unsigned long cycleStart[4] = {0,0,0,0};
#endif
boolean heatStatus[4], PIDEnabled[4];
unsigned int steamPSens, steamZero;

byte pidLimits[4] = { PIDLIMIT_HLT, PIDLIMIT_MASH, PIDLIMIT_KETTLE, PIDLIMIT_STEAM };

//Steam Pressure in thousandths
unsigned long steamPressure;
byte boilPwr;

PID pid[4] = {
    PID(&PIDInput[VS_HLT], &PIDOutput[VS_HLT], &setpoint[VS_HLT], 3, 4, 1),

#ifdef PID_FEED_FORWARD
    PID(&PIDInput[VS_MASH], &PIDOutput[VS_MASH], &setpoint[VS_MASH], &FFBias, 3, 4, 1),
#else
    PID(&PIDInput[VS_MASH], &PIDOutput[VS_MASH], &setpoint[VS_MASH], 3, 4, 1),
#endif

    PID(&PIDInput[VS_KETTLE], &PIDOutput[VS_KETTLE], &setpoint[VS_KETTLE], 3, 4, 1),

#ifdef PID_FLOW_CONTROL
    PID(&PIDInput[VS_PUMP], &PIDOutput[VS_PUMP], &setpoint[VS_PUMP], 3, 4, 1)
#else
    PID(&PIDInput[VS_STEAM], &PIDOutput[VS_STEAM], &setpoint[VS_STEAM], 3, 4, 1)
#endif
};
#if defined PID_FLOW_CONTROL && defined PID_CONTROL_MANUAL
  unsigned long nextcompute;
  byte additioncount[2];
#endif

#ifdef RIMS_MLT_SETPOINT_DELAY
  byte steptoset = 0;
  byte RIMStimeExpired = 0;
  unsigned long starttime = 0;
  unsigned long timetoset = 0;
#endif

//Timer Globals
unsigned long timerValue[2], lastTime[2];
boolean timerStatus[2], alarmStatus;

//Log Globals
boolean logData = LOG_INITSTATUS;

//Brew Step Logic Globals
boolean preheated[4];
ControlState boilControlState = CONTROLSTATE_OFF;

//Bit 1 = Boil; Bit 2-11 (See Below); Bit 12 = End of Boil; Bit 13-15 (Open); Bit 16 = Preboil (If Compile Option Enabled)
unsigned int hoptimes[11] = { 105, 90, 75, 60, 45, 30, 20, 15, 10, 5, 0 };
byte pitchTemp;

//Log Strings
const char LOGCMD[] PROGMEM = "CMD";
const char LOGDEBUG[] PROGMEM = "DEBUG";
const char LOGSYS[] PROGMEM = "SYS";
const char LOGCFG[] PROGMEM = "CFG";
const char LOGDATA[] PROGMEM = "DATA";

//PWM by timer globals
#ifdef PWM_BY_TIMER
unsigned int timer1_overflow_count = 0;
unsigned int PIDOutputCountEquivalent[4][2] = {{0,0},{0,0},{0,0},{0,0}};
#endif

//**********************************************************************************
// Setup
//**********************************************************************************

void setup() {

#ifdef ADC_REF
    analogReference(ADC_REF);
#endif

#ifdef USE_I2C
    Wire.begin(BT_I2C_ADDR);
#endif

    //Log initialization (Log.pde)
    comInit();
    //Pin initialization (Outputs.pde)
    pinInit();

    // Default Output Bank
#if defined(PVOUT_TYPE_GPIO)
    outputBanks[0] = new GPIOOutputBank();
#elif defined(PVOUT_TYPE_MUX)
    outputBanks[0] = new MUXOutputBank(MUX_LATCH_PIN, MUX_DATA_PIN, MUX_CLOCK_PIN, MUX_ENABLE_PIN, MUX_ENABLE_LOGIC);
#endif

    tempInit();

    //Check for cfgVersion variable and update EEPROM if necessary (EEPROM.pde)
    checkConfig();

    //Load global variable values stored in EEPROM (EEPROM.pde)
    loadSetup();

    // Initialize output banks after MODBUS banks are created in loadSetup
    for (int x = 0; x < NUM_OUTPUT_BANKS; x++) {
        if (outputBanks[x]) {
            outputBanks[x]->init();
        }
    }

#ifdef DIGITAL_INPUTS
    //Digital Input Interrupt Setup
    triggerSetup();
#endif

    //PID Initialization (Outputs.pde)
    pidInit();

#ifdef PWM_BY_TIMER
    pwmInit();
  #endif

    //User Interface Initialization (UI.pde)
    //Moving this to last of setup() to allow time for I2CLCD to initialize
#ifndef NOUI
    uiInit();
#endif

    //Init of program threads will call event handler to set active screen and must be called after uiInit()
    programThreadsInit();
}


//**********************************************************************************
// Loop
//**********************************************************************************

void loop() {
    //User Interface Processing (UI.pde)
#ifndef NOUI
    uiCore();
#endif

    //Core BrewTroller process code (BrewCore.pde)
    brewCore();
}

int main(void)
{
    init();

    setup();

    for (;;) {
        loop();
        if (serialEventRun) serialEventRun();
    }

    return 0;
}