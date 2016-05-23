/*
 Copyright (C) 2015 BrewTroller
 
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
 
 Documentation, Forums and more information available at http://brewtroller.com
 */
#ifndef BrewTroller_Config_h
#define BrewTroller_Config_h

#include <stdint.h>
#include "@ENUM_HEADER@"

//*****************************************************************************************************************************
// USER COMPILE OPTIONS
//*****************************************************************************************************************************

//**********************************************************************************
// UNIT (Metric/US)
//**********************************************************************************
//
#cmakedefine USEMETRIC
//**********************************************************************************


//**********************************************************************************
// Brewing Calculation Factors
//**********************************************************************************
// GRAIN2VOL: The amount of volume in l/kg or gal/lb that grain occupies in the mash
// Conservatively 1 lb = 0.15 gal
// Aggressively 1 lb = 0.093 gal
#ifdef USEMETRIC
#define GRAIN2VOL 1.25
#else
#define GRAIN2VOL .15
#endif

// GRAIN_VOL_LOSS: The amount of liquid volume lost with spent grain. This value can
// vary by grain types, crush, etc.
// Default values are pretty conservative (err on more absorbtion)
// Ray Daniels suggests .20, Denny Conn suggests .10
#ifdef USEMETRIC
#define GRAIN_VOL_LOSS 1.7884
#else
#define GRAIN_VOL_LOSS .2143
#endif

// VOL_SHRINKAGE: The amount of liquid volume reduced as a result of decrease in temperature.
// This value used to be .96 in BrewTroller 2.4 and earlier versions but this value should
// not be used in volume calculations for water at ground temperature when targeting pitch temps.
// A value of '1' (default) will eliminate this from brewing calculations.
#define VOL_SHRINKAGE 1
//#define VOL_SHRINKAGE .96

//**********************************************************************************
// Vessel Options
//**********************************************************************************
// BrewTroller was designed to support three vessles (HLT, Mash and Kettle). This
// sections provides support for specific systems that differ from this model.

// HLT_AS_KETTLE: This option remaps the Kettle temp sensor, volume sensor and heat
// output to the HLT's devices to  allow the HLT to be reused as a kettle.
#cmakedefine HLT_AS_KETTLE

// KETTLE_AS_MASH: This option remaps the Mash temp sensor, volume sensor and heat
// output to the Kettle's devices to allow the Kettle to also serve as Mash Tun.
// Use with HERMS HWProfile for BX1/DX1/EX1 which defines only HLT and Kettle Heat
// Outputs leaving an additional output to use for pumps/valves.
#cmakedefine KETTLE_AS_MASH

// MASH_PREHEAT_SENSOR: This option allows for an alternate temperature sensor to
// control the mash heat output during the Preheat step. This is used to control the
// water temperature on dedicated HEX vessel during preheat. After preheat the
// actual mash temperature sensor would be used to control the mash heat output.
// aka 'Yorg Option 1'
//#define MASH_PREHEAT_SENSOR TS_AUX1

// MASH_PREHEAT_STRIKE/MASH_PREHEAT_STEP1: Use one of the following two options to
// override the zero setpoint for the mash tun when the 'Heat Strike In' program
// option is set to HLT. STRIKE will use the calculated strike temp. STEP1 will use
// the first mash step temp. aka 'Yorg Option 2'
//#define MASH_PREHEAT_STRIKE
//#define MASH_PREHEAT_STEP1

//MASH_PREHEAT_NOVALVES: Disables MASH HEAT/MASH IDLE Valve Profiles during preheat
//#define MASH_PREHEAT_NOVALVES

//SINGLE_VESSEL_SUPPORT: This is a crude hack that uses the HLT sensor and output
//for the HLT, Mash and Kettle functions.
#cmakedefine SINGLE_VESSEL_SUPPORT
//**********************************************************************************

//**********************************************************************************
// Vessel to temperature sensor mapping
//**********************************************************************************
// The purpose of this array is to provide a safe way to map a vessel to the
// temperaure sensor to read for that vessels setpoint.
// The secondary purpose is to provide a safe way to enumerate the heat outputs,
// safely decoupling the #defines values from loop-control.
#if defined PID_FLOW_CONTROL
static const int HEAT_OUTPUTS_COUNT = 4;
static const uint8_t HEAT_OUTPUTS[HEAT_OUTPUTS_COUNT][2] = {{VS_HLT, TS_HLT}, {VS_MASH, TS_MASH}, {VS_KETTLE, TS_KETTLE}, {VS_PUMP, TS_MASH}};
#else
static const int HEAT_OUTPUTS_COUNT = 3;
static const uint8_t HEAT_OUTPUTS[HEAT_OUTPUTS_COUNT][2] = {{VS_HLT, TS_HLT}, {VS_MASH, TS_MASH}, {VS_KETTLE, TS_KETTLE}};
#endif
// These two should be used as the array index when operating on a HEAT_OUTPUT array.
// They need to be variables instead of #defines because of use as index subscripts.
static const uint8_t VS = 0;
static const uint8_t TS = 1;

//**********************************************************************************
// PID Output Power Limit
//**********************************************************************************
// These settings can be used to limit the PID output of the the specified heat
// output. Enter a percentage (0-100)
//
#define PIDLIMIT_HLT 100
#define PIDLIMIT_MASH 100
#define PIDLIMIT_KETTLE 100
#define PIDLIMIT_STEAM 100 // note this is also the PID limit for the pump PWM output if PID_FLOW_CONTROL is enabled

//**********************************************************************************
// PID Feed Forward control
//**********************************************************************************
// This #define enables feed forward on the mash PID loop. The feed forward can be set to any
// number of different temp sensors by using the #define for it below, to see sensor #defines see Enum.h
// under TSensor and output (0-2) Array Element Constants
// NOTE: not a good idea to use any sensor you average into the MASH sensor as your feed forward
//
//#define PID_FEED_FORWARD
//#define FEED_FORWARD_SENSOR TS_AUX1

//**********************************************************************************
// PWM ouputs controled by timer rather than brew core loop
//**********************************************************************************
// This #define enables the PWM outputs to be controled by timer rather than the brew core loop.
// This means that we can have a higher frequency output, and that the timings of the PWM signal are more
// accurate. This is required if you are goign to attempt to control a pump with a PWM output.
// NOTE: The counter/timer is set to work with a 16mhz input frequency and is set to run at 8khz PWM output
// frequency as the fastest possible frequency, (also note that the period cannot exceed 8.19 seconds). Also
// only two PWM outputs can run at 8khz, all the rest must run at a lower frequency as defiend above. The
// two PWM outputs which will be 8khz can be defined below, comment them both out if none are that high.
// Also, the reported period for the 8khz outputs is going to look like 1 seconds in both the UI and the log.
// You will not however be able to set the the PWM frequency from the UI because it is set at 8khz, the value
// given in the UI will be ignored. The % output however will be reported properly through the UI and log.
//#define PWM_BY_TIMER
//**********************************************************************************

//**********************************************************************************
// Flow rate calcs fed into PID controller for auto fly sparge
//**********************************************************************************
// This #define enables the feeding of the flow rate calcs based on the pressure sensors to be fed into the
// PID code to control a pump for fly sparge to get a desired flow rate. Note that the PWM output used to
// control the pump takes over the steam output, and thus the steam output cannot be used for steam.
// Note: This code is designed to work with PWM_BY_TIMER
// Note2: Given our current 10 bit adc and the average pressure sensor resolution for volume you only get about
// 7 ADC clicks per quart, thus if you have your flow rate calcs set to happen to fast you'll always show a 0 flow
// rate. You'll need at least 20 seconds between flow rate calcs to be able to measure this slow of a flow rate.
// Note3: the Pump output must be set to PID for this to work as well.
// Note4: In the UI when you enter the Pump flow rate it's entered in 10ths of a quart per minute, so 1 quart per
// minute would be 10.
//#define PID_FLOW_CONTROL
//#define PID_CONTROL_MANUAL  // modified manual control (still has to be set to PID in settings menu) in case you
//just cant get PID to work
#define PID_FLOW_MIN 30     // this is the minimum PID output duty cycle % to be used when the setpoint is non zero
// this is used because under a certain duty cycle a pump wont even spin or just
// make foam, etc so we need the pump to at least move liquid before we try to
// control it or your process + intregral variables can just run away while you're trying
// to spin up.
//**********************************************************************************

//**********************************************************************************
// Fly sparge pump control to turn the sparge in pump on/off based on a hysteresis from volume of sparge out
//**********************************************************************************
// This #define will turn the fly sparge in valve config on when the hysteresis amount of fluid has been pumped
// into the kettle from the MLT. It will then shut off the pump when that equal amount of sparge water has been
// pumped out of the HLT.
// Note: SPARGE_IN_HYSTERSIS is in 1000ths of a gallon or liter.
//#define SPARGE_IN_PUMP_CONTROL
#define SPARGE_IN_HYSTERESIS 250
//**********************************************************************************

//**********************************************************************************
// Kettle Lid Control
//**********************************************************************************
// The kettle lid Valve Profile can be used to automate covering of the boil kettle.
// The kettle lid profile is activated in the Chill step of a program when the
// kettle temperature is less than the threshhold specified below.
//
#ifdef USEMETRIC
//Celcius
#define KETTLELID_THRESH 80
#else
//Fahrenheit
#define KETTLELID_THRESH 176
#endif
//**********************************************************************************

//**********************************************************************************
// Hop Addition Valve Profile
//**********************************************************************************
// A valve profile is activated based on the boil additions schedule during the boil
// stage of AutoBrew. The parameter below is used to define how long (in milliseconds)
// the profile stays active during each addition.
#define HOPADD_DELAY 0
//**********************************************************************************

//**********************************************************************************
// Smart HERMS HLT
//**********************************************************************************
// SMART_HERMS_HLT: Varies HLT setpoint based on mash target + variance
// HLT_MAX_TEMP: Ceiling value for HLT (Actual max temp in C or F, Decimal values allowed)
// MASH_HEAT_LOSS: Acts as a floor value to ensure HLT temp is at least target +
// specified value
// SMART_HERMS_PREHEAT: Enabling this sub-option will cause SMART_HERMS_HLT
// logic to be enabled during preheat. By default, a recipe's HLT Temp setting is used
// during preheat.

//#define SMART_HERMS_HLT
#define MASH_HEAT_LOSS 0
#define HLT_MAX_TEMP 180
//#define SMART_HERMS_PREHEAT
//**********************************************************************************

//**********************************************************************************
// Strike Temperature Correction
//**********************************************************************************
// STRIKE_TEMP_OFFSET: Adjusts strike temperature to compensate for thermal mass of
// mash tun.
// Value may be positive or negative. Decimal values are allowed.

#define STRIKE_TEMP_OFFSET 0

//**********************************************************************************


//**********************************************************************************
// Mash Temperature Adjustment
//**********************************************************************************
// MASH_AVG_AUXx: Uncomment one or more of the following lines to include averaging
// of AUX1, AUX2 and/or AUX3 temp sensors with mash temp sensor.
//#define MASH_AVG_AUX1
//#define MASH_AVG_AUX2
//#define MASH_AVG_AUX3

//Enable Mash Avergaing Logic if any Mash_AVG_AUXx options were enabled
#if defined MASH_AVG_AUX1 || defined MASH_AVG_AUX2 || defined MASH_AVG_AUX3
#define MASH_AVG
#endif
//**********************************************************************************


//**********************************************************************************
// Stop fly sparge removing water from HLT at this volume
//**********************************************************************************
// This feature was added because the dead space for the HLT may be less than the space lost in the tubing.
// In particular if you're flushing your RIMS tube wort with fly sparge in water the water in the RIMS tube
// will need to be added to the HLT dead space even though the dead space of water actually left in the
// HLT will be less than that and we want to continue fly sparging until it's that low.
// NOTE: volume is in thousands of a gallon
//#define HLT_FLY_SPARGE_STOP
//#define HLT_FLY_SPARGE_STOP_VOLUME 250
//**********************************************************************************


//**********************************************************************************
// Boil Off Unit Change
//**********************************************************************************
// This option will change the units of the boil off from % per hour to 0.1 gallons
// or 1 liter per hour
//#define BOIL_OFF_GALLONS
//**********************************************************************************


//**********************************************************************************
// Pre-Boil Alarm
//**********************************************************************************
// PREBOIL_ALARM: Triggers the alarm during the boil stage when the defined
// temperature is reached

#define PREBOIL_ALARM 205
//**********************************************************************************

//**********************************************************************************
// Buzzer modulation parameters
//**********************************************************************************
// These parameters allow the alarm sound to be modulated.
// The modulation occurs when the BUZZER_CYCLE_TIME value is larger than the BUZZER_ON_TIME
// When the BUZZER_CYCLE_TIME is zero there is no modulation so the buzzer will buzz
// a steady sound
//
//#define BUZZER_CYCLE_TIME 1200 //the value is in milliseconds for the ON and OFF buzzer cycle
//#define BUZZER_ON_TIME 500     //the duration in milliseconds where the alarm will stay on
//**********************************************************************************


//**********************************************************************************
// Serial0 ASCII Protocol Options:
//**********************************************************************************
// These settings apply only if COM_SERIAL0 is set to 'ASCII' in your HWProfile.h
//
// COMSCHEMA: Specifies the schema for a particular type
//  ASCII Messages
//      0 - Original BT 2.0 Messages
//      1 - BT 2.1 Enhanced ASCII
//       Steam, Calc. Vol & Temp, BoilPower, Grain Temp, Delay Start, MLT Heat Source
#define COMSCHEMA 0
//
// LOG_INTERVAL: Specifies how often data is logged via serial in milliseconds. If
// real time display of data is being used a smaller interval is best (1000 ms). A
// larger interval can be used for logging applications to reduce log file size
// (5000 ms).
#define LOG_INTERVAL 2000
//
// LOG_INITSTATUS: Sets whether logging is enabled on bootup. Log status can be
// toggled using the SET_LOGSTATUS command.
#define LOG_INITSTATUS 1


//**********************************************************************************
// BrewTroller PID Display (BTPD)
//**********************************************************************************
// BTPD is an external LED display developed by BrewTroller forum member vonnieda.
// It is a 2 line, 4 digit (8 digits total) LED display with one line red and one
// line green. The digits are about a half inch high and can easily be seen across
// the room. The display connects to the BrewTroller via the I2C header and can be
// daisy chained to use as many as you like, theoretically up to 127 but in practice
// probably 10 or so.
// You need to set the addresses of each display in the Com_BTPD.h file.

// BTPD_INTERVAL: Specifies how often BTPD devices are updated in milliseconds
#define BTPD_INTERVAL 500

// Show temperature and volume per kettle on the same display.  Every other update
// interval the display will switch from temperature to volume.  Make sure that the
// values in Com_BTPD.h use the same address per kettle for both volume and temperature.
//#define BTPD_ALTERNATE_TEMP_VOLUME

//**********************************************************************************

//**********************************************************************************


//**********************************************************************************
// UI: Home Screen Options
//**********************************************************************************
// LOGO_TROLL: Old Home screen with Troll icon
// LOGO_BREWTROLLER: New Home Screen based on new BrewTroller logo
//#define LOGO_TROLL
#define LOGO_BREWTROLLER


//**********************************************************************************
// Brew Step Automation
//**********************************************************************************
// Uncomment the following line(s) to enable various steps to start/stop
// automatically
//

// AUTO_FILL_START: This option will enable the Fill AutoValve logic at the start of
// the Fill step.
//#define AUTO_FILL_START

// AUTO_REFILL_START: This option will enable the Fill AutoValve logic at the start of the
// ReFill steip
//#define AUTO_REFILL_START

// AUTO_FILL_EXIT: This option will automatically exit the Fill step once target
// volumes have been reached.
//#define AUTO_FILL_EXIT

// AUTO_PREHEAT_EXIT: By default the user must manually exit the Preheat step.
// This prevents the strike water from cooling if the brewer is not present at the
// end of preheat. Use this option to automatically exit preheat if desired.
//#define AUTO_PREHEAT_EXIT

// AUTO_ML_XFER: This option will enable the Sparge In AutoValve logic at the start
// of the Grain In step if the Mash Liquor Heat Source is set to HLT. This is used
// to transfer preheated mash liquor from HLT to Mash Tun.
//#define AUTO_ML_XFER

// AUTO_GRAININ_EXIT: This option will automatically exit the Grain In step after
// the specified number of seconds. Use this setting if your grain is automatically
// added to the mash tun using the Add Grain valve profile. You can also specify a
// value of 0 to exit the Grain In step automatically with no additional delay.
// The Grain In step will not process exit logic until the mash liquor transfer is
// completed when the mash Liquor Heat Source is set to HLT.
//#define AUTO_GRAININ_EXIT 0

// AUTO_MASH_HOLD_EXIT: By default the user must manually exit the Mash Hold step.
// This prevents the mash from cooling if the brewer is not present at the end of
// the last mash step. Use this option to automatically exit the mash hold step if
// the boil zone is inactive.
//#define AUTO_MASH_HOLD_EXIT

// AUTO_MASH_HOLD_EXIT_AT_SPARGE_TEMP This option, assuming the AUTO_MASH_HOLD_EXIT
// option is turned on (else does nothing) wont allow the auto mash hold exit to occur until
// the HLT has reached the sparge temp of the currently active program.
//#define AUTO_MASH_HOLD_EXIT_AT_SPARGE_TEMP

// AUTO_SPARGE_START: This option will automatically enable batch or fly sparge
// logic at the start of the sparge step.
//#define AUTO_SPARGE_START

// AUTO_SPARGE_EXIT: This option will automatically advance the sparge step when
// target preboil volume is reached.
//#define AUTO_SPARGE_EXIT

// AUTO_BOIL_RECIRC: Activates the BOIL RECIRC valve profile during the last minutes
// of the AutoBrew Boil stage as defined below (ie AUTO_BOIL_RECIRC 20 will enable
// BOIL RECIRC for the last twenty minutes of boil. Warning: if you do not have a
// valve config that will reroute wort back to the kettle there is a great risk of
// losing wort or causing personal injury when this profile is enabled
//#define AUTO_BOIL_RECIRC 20
//**********************************************************************************

//**********************************************************************************
// Delay setting the first setpoint for the MLT for a RIMS tube so the RIMS tube can be full before power on
//**********************************************************************************
// This code will add a delay of RIMS_DELAY (in miliseconds) to expire before the first MLT setpoint is set. This is to allow the RIMS
// tube to be filled up with the recirc pump and come to a steady state before turning on the power.
//NOTE: Do not use this code if you do not have a dough in time set that is longer than the RIMS_DELAY for any program
// start.
//#define RIMS_MLT_SETPOINT_DELAY
#define RIMS_DELAY 60000
//**********************************

//**********************************************************************************
// Volume Sensor Settings
//**********************************************************************************
// VOLUME_MANUAL: Modifies the user interface to show target volumes instead of
// current volumes for people who are not using volume sensors. The target
// volume information will be shown during Add Grain and during Sparge.
//
#cmakedefine VOLUME_MANUAL
//**********************************************************************************

//**********************************************************************************
// Volume Averaging Settings
//**********************************************************************************
// VOLUME_READ_INTERVAL: Time in ms between volume readings
// VOLUME_READ_COUNT: Number of individual volume readings to average when
// calculating a vessel's volume
//
#define VOLUME_READ_INTERVAL 200
#define VOLUME_READ_COUNT 5
//**********************************************************************************

//**********************************************************************************
// Flow Rate Calculation
//**********************************************************************************
// FLOWRATE_CALCS: Enables calculation of flow rates for each vessel based on
// volume changes over a specified interval
// FLOWRATE_READ_INTERVAL: Time in ms between flowrate calculation updates
//
//#define FLOWRATE_CALCS
#define FLOWRATE_READ_INTERVAL 1000
//**********************************************************************************


//**********************************************************************************
// RS485/Modbus Configuration
//**********************************************************************************

#define RS485_BAUDRATE             76800
#define RS485_PARITY               SERIAL_8E1

#define PVOUT_MODBUS_DEFCOILREG    1000
#define PVOUT_MODBUS_DEFCOILCOUNT  8
#define PVOUT_MODBUS_BASEADDR      10
#define PVOUT_MODBUS_ADDRNONE      255
#define PVOUT_MODBUS_ADDRINIT      247
#define PVOUT_MODBUS_REGIDMODE     9000
#define PVOUT_MODBUS_REGSLAVEADDR  9001
#define PVOUT_MODBUS_REGRESTART    9002

/***********************************************************************************
 * EXPERIMENTAL OPTIONS
 ***********************************************************************************
 The following options are experimental with little to no testing.
 **********************************************************************************/

//**********************************************************************************
// Save HLT and KET heating elements Support
//**********************************************************************************
// EXPERIMENTAL: Uncomment one or more of the following lines to enable forcing the
// HLT, Mash or Kettle outputs to 0 if the volume in said vessel is less than the
// defined value.
// NOTE: Volume is in thousandths of a Gallons/Liters
// USE CAUTION! TESTING REQUIRED.
//
//#define HLT_MIN_HEAT_VOL 4000
//#define MASH_MIN_HEAT_VOL 4000
//#define KETTLE_MIN_HEAT_VOL 4000
//**********************************************************************************

//**********************************************************************************
// Min HLT refill volume
//**********************************************************************************
// EXPERIMENTAL: Uncomment the following line to enable forcing a minimum refill amount in the HLT
// during the refill step. This is so that you can make any amount of sparge water needed by making
// sure your heating elements are covered by water so they can heat your sparge water even if you're
// only going to use 0.25 gallons of it or some other small amount of sparge water.
// NOTE: Volume is in thousandths of a Gallons/Liters
// USE CAUTION! TESTING REQUIRED.
//
//#define HLT_MIN_REFILL
//#define HLT_MIN_REFILL_VOL 4000
//**********************************************************************************


//**********************************************************************************
// HLT Heat During Sparge
//**********************************************************************************
// HLT_HEAT_SPARGE: Enables the HLT setpoint during the sparge until a minimum
// volume level is reached.
// HLT_MIN_SPARGE: Minimum HLT volume trigger to disable the HLT Setpoint during
// sparge. Value represents thousandths of Gallons/Litres.
//
//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  !! WARNING: ENABLING THIS OPTION WITHOUT VOLUME LEVEL SENSING OR USING A MINIMUM !!
//  !! HLT VOLUME FLOAT SWITCH WILL RESULT IN DRY FIRING THE HLT AND CAUSING DAMAGE  !!
//  !! AND/OR PERSONAL HARM!                                                         !!
//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//#define HLT_HEAT_SPARGE
//#define HLT_MIN_SPARGE 2000
//**********************************************************************************

//**********************************************************************************
// Direct-fired RIMS support
//**********************************************************************************
// DIRECT_FIRED_RIMS/RIMS_TEMP_OFFSET/RIMS_DURING_SPARGE: specifies that the mash
// kettle is direct-fired, either with gas, or a heating element, and that the RIMS
// has it's own element. With this option, the VS_MASH is used for the mash, and the
// VS_STEAM is used for the RIMS. Only the VS_MASH is used to change the temp; when
// the temp is within RIMS_TEMP_OFFSET of the set temp, the VS_MASH is turned off,
// and VS_STEAM is turned on, to reach and maintain the set temp. When the strike temp
// is beaing reached, or any other step change grater than RIMS_TEMP_OFFSET, this
// allows VS_MASH to be used for the quicker temeperature change, then for VS_STEAM to
// take over for finer temperaature control.
//#define DIRECT_FIRED_RIMS
#ifdef DIRECT_FIRED_RIMS
// If you are not recirculating your mash, the offset should probably be greater.
#define RIMS_TEMP_OFFSET 5
// Specify the temperature sensor used in the RIMS tube. TS_AUX1, TS_AUX2 or TS_AUX3 is recommended.
#define RIMS_TEMP_SENSOR TS_AUX1
// You really should have a sensor in your RIMS tube: this #defines allow you to set
// the maximum temp that the RIMS tuube is allowed to reach.  It is important to note
// that both the sensor and the heating element should be submersed in liqued, with
// the input and output ports facing up, so that the tube can not run dry.
#define RIMS_MAX_TEMP 180
// As the SSD can get stuck in the ON state, if the RIMS_ALARM_TEMP temperature is
// reached, turn on the alarm.
#define RIMS_ALARM_TEMP 190
// If your HLT output passes through your RIMS tube to your mash kettle, you may want
// to define RIMS_DURING_SPARGE so that it can also control the temp of your sparge
// water.  The logic here is somehwat different than for mashing, in that it will only
// control the VS_STEAM output.  You can use this in conjuction with HLT_HEAT_SPARGE
// to fire the HLT too.
#define RIMS_DURING_SPARGE
#endif
//**********************************************************************************



//**********************************************************************************
// Sparge Options
//**********************************************************************************
// BATCH_SPARGE: Uses batch sparge logic instead of fly sparge logic for programs.
//#define BATCH_SPARGE

// BATCH_VOLUME_OFFSET: Adjusts batch volume calculations to increase or reduce the
// volume of batch sparges.
//#define BATCH_VOLUME_OFFSET 0

// BATCH_SPARGE_RECIRC: Specifies the number of seconds to run the Mash Heat valve
// profile between batch sparges.
//#define BATCH_SPARGE_RECIRC 60
//**********************************************************************************

//**********************************************************************************
// RGB Board options
//**********************************************************************************
// The RGB Board allows you to have RGB LEDs show the status of 8 heat or PV outputs
// and allows you to have up to 8 switches connected to control them. You can connect
// multiple RGB boards to the BrewTroller to expand the number of inputs and outputs.
//
// Each numbered output provides the 4 neccesary connections for a common anode RGB
// LED. Each numbered input provides 2 commons, an auto and a manual input connection
// for connecting a 3 position toggle switch, or similar switch.
//
// By default, BrewTroller is configured to use the first sets of inputs and outputs
// on the RGB boards for heat outputs and then any remaining sets that are available
// for pump/valve outputs.
//
// For instance, if you have 3 heat outputs and 1 RGB board, the RGB board will have
// it's inputs and outputs set up like this:
//
// RGB Board 1, Input/Output 0 = Heat Output 0 (HLT)
// RGB Board 1, Input/Output 1 = Heat Output 1 (Mash)
// RGB Board 1, Input/Output 2 = Heat Output 2 (Boil)
// RGB Board 1, Input/Output 3 = PV Output 0
// RGB Board 1, Input/Output 4 = PV Output 1
// RGB Board 1, Input/Output 5 = PV Output 2
// RGB Board 1, Input/Output 6 = PV Output 3
// RGB Board 1, Input/Output 7 = PV Output 4
//
// Adding a second RGB board would add the following mappings:
//
// RGB Board 2, Input/Output 0 = PV Output 5
// RGB Board 2, Input/Output 1 = PV Output 6
// RGB Board 2, Input/Output 2 = PV Output 7
// RGB Board 2, Input/Output 3 = PV Output 8
// RGB Board 2, Input/Output 4 = PV Output 9
// RGB Board 2, Input/Output 5 = PV Output A
// RGB Board 2, Input/Output 6 = PV Output B
// RGB Board 2, Input/Output 7 = PV Output C
//
// And finally, adding a third RGB board would add:
//
// RGB Board 3, Input/Output 0 = PV Output D
// RGB Board 3, Input/Output 0 = PV Output E
// RGB Board 3, Input/Output 0 = PV Output F
// RGB Board 3, Input/Output 0 = PV Output G
// RGB Board 3, Input/Output 0 = PV Output H
// RGB Board 3, Input/Output 0 = PV Output I
// RGB Board 3, Input/Output 0 = PV Output J
// RGB Board 3, Input/Output 0 = PV Output K
//
// If this default configuration does not suit you, check out the Com_RGBIO8 file
// in the RGBIO8_Init() function to see how to customize it to your specific
// configuration.
//
// Enables the RGBIO8 system.
//
//#define RGBIO8_ENABLE
//
// Enables the setup UI for the RGBIO8 board. This takes up quite a bit of code
// space so it can be disabled once you have set up all of your boards. It is
// not needed in day to day use.
//
#define RGBIO8_SETUP
//
// The first address of your RGB Boards. Other boards should follow using the next
// address. So, for instance, if this value is 0x30, board 2 should be 0x31, board
// 3 should be 0x32, etc.
//
#define RGBIO8_START_ADDR 0x30
//
// The number of RGB boards you have connnected.
//
#define RGBIO8_NUM_BOARDS 1
//
//**********************************************************************************

#endif
