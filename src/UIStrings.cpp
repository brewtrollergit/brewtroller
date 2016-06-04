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
Documentation, Forums and more information available at http://www.brewtroller.com
*/

#include <avr/pgmspace.h>
#include "UIStrings.h"

#ifndef NOUI

namespace UIStrings {
    
    namespace Generic {
        //Generic Menu strings
        const char OK[] PROGMEM = "Ok";
        const char CANCEL[] PROGMEM = "Cancel";
        const char EXIT[] PROGMEM = "Exit";
        const char ABORT[] PROGMEM = "Abort";
        const char MENU[] PROGMEM = "Menu";
        const char SPACE[] PROGMEM = " ";
        const char CONTINUE[] PROGMEM = "Continue";
        const char TARGET[] PROGMEM = "Target";
        const char ACTUAL[] PROGMEM = "Actual";
        const char GREATER_SYM[] PROGMEM = ">";
        const char LESS_SYM[] PROGMEM = "<";
        const char COLON[] PROGMEM = ":";
        const char ON[] PROGMEM = "On";
        const char OFF[] PROGMEM = "Off";
        const char TEMPBLANK[] PROGMEM = "----";
        const char VOLBLANK[] PROGMEM = "--.---";
        const char AUTO[] PROGMEM = "Auto";
        const char MANUAL[] PROGMEM = "Manual";
        const char SET_TIMER[] PROGMEM = "Set Timer";
        const char PAUSE_TIMER[] PROGMEM = "Pause Timer";
        const char START_TIMER[] PROGMEM = "Start Timer";
        const char COLON_SPACE[] PROGMEM = ": ";
        const char PERC_SYM[] PROGMEM = "%";
        const char EMPTY[] PROGMEM = "";
        const char BTVER[] PROGMEM = "BT v";
        const char DOT[] PROGMEM = ".";
    }
    
    namespace Shared {
        const char ALLOFF[] PROGMEM = "All Off";
        const char HLT_TARGET[] PROGMEM = "HLT Target";
        const char HLT_TARGET_VOL[] PROGMEM = "HLT Target Vol";
        const char VALVE_ON[] PROGMEM = ": On";
        const char VALVE_OFF[] PROGMEM = ": Off";
        const char HLT_SETPOINT[] PROGMEM = "HLT Setpoint";
        const char HLTDESC[] PROGMEM = "Hot Liquor Tank";
        const char MASHDESC[] PROGMEM = "Mash Tun";
        const char SEC[] PROGMEM = "s";
        const char ZERO[] PROGMEM = "0";
        const char DELETE[] PROGMEM = "Delete";
    }
    
    //EEPROM Setup/Missing Screen
    namespace EEPROMInit {
        const char MISSING_CONFIG[] PROGMEM = "Missing Config";
        const char INIT_EEPROM[] PROGMEM = "Initialize EEPROM";
        const char PLEASE_WAIT[] PROGMEM = "Please Wait...";
    }
    
    //Home screen strings
    namespace HomeScreen {
        const char BUILD[] PROGMEM = "Build";
        const char BT_URL[] PROGMEM = "www.brewtroller.com";
    }
    
    //Fill screen strings
    namespace FillScreen {
        const char FILL[] PROGMEM = "Fill";
        const char REFILL[] PROGMEM = "Refill";
    }
    
    //Valve profile name strings
    namespace ValveProfile {
        const char FILLHLT[] PROGMEM = "Fill HLT";
        const char FILLMASH[] PROGMEM = "Fill Mash";
        const char ADDGRAIN[] PROGMEM = "Add Grain";
        const char MASHHEAT[] PROGMEM = "Mash Heat";
        const char MASHIDLE[] PROGMEM = "Mash Idle";
        const char SPARGEIN[] PROGMEM = "Sparge In";
        const char SPARGEOUT[] PROGMEM = "Sparge Out";
        const char BOILADDS[] PROGMEM = "Boil Additions";
        const char KETTLELID[] PROGMEM = "Kettle Lid";
        const char CHILLH2O[] PROGMEM = "Chill H2O";
        const char CHILLBEER[] PROGMEM = "Chill Beer";
        const char BOILRECIRC[] PROGMEM = "Boil Recirc";
        const char DRAIN[] PROGMEM = "Drain";
        const char HLTHEAT[] PROGMEM = "HLT Heat";
        const char HLTIDLE[] PROGMEM = "HLT Idle";
        const char KETTLEHEAT[] PROGMEM = "Kettle Heat";
        const char KETTLEIDLE[] PROGMEM = "Kettle Idle";
        const char USER1[] PROGMEM = "User Valve 1";
        const char USER2[] PROGMEM = "User Valve 2";
        const char USER3[] PROGMEM = "User Valve 3";
    }
    
    //Grain in step strings
    //TODO: this needs to be clarified, what is the difference between Grain in and Dough In?
    namespace GrainInScreen {
        const char GRAININ[] PROGMEM = "Grain In";
    }
    
    //Mash step strings
    namespace MashStep {
        const char DELAY[] PROGMEM = "Delay";
        const char PREHEAT[] PROGMEM = "Preheat";
        const char DOUGHIN[] PROGMEM = "Dough In";
        const char ACID[] PROGMEM = "Acid";
        const char PROTEIN[] PROGMEM = "Protein";
        const char SACCH[] PROGMEM = "Sacch";
        const char SACCH2[] PROGMEM = "Sacch2";
        const char MASHOUT[] PROGMEM = "Mash Out";
        const char ENDMASH[] PROGMEM = "End Mash";
#ifdef DIRECT_FIRED_RIMS
        const char BADTEMP[] PROGMEM = "12345678----";
        const char OCTOTHORPE[] PROGMEM = "#";
        const char ASTERISK[] PROGMEM = "*";
        const char DASH[] PROGMEM = "-";
#endif
    }
    
    //Sparge step strings
    namespace SpargeScreen {
        const char SPARGE[] PROGMEM = "Sparge";
        const char FILLBOTH[] PROGMEM = "Fill Both";
        const char FLYSPARGE[] PROGMEM = "Fly Sparge";
    }
    
    //Boil step strings
    namespace BoilScreen {
        const char BOIL[] PROGMEM = "Boil";
        const char BOIL_AUTO[] PROGMEM = "  Auto";
    }
    
    //Chill step state strings
    namespace ChillScreen {
        const char CHILL[] PROGMEM = "Chill";
        const char BEER[] PROGMEM = "Beer";
        const char H20[] PROGMEM = "H20";
        const char IN[] PROGMEM = "In";
        const char OUT[] PROGMEM = "Out";
        const char CHILL_BOTH[] PROGMEM = "Chill Both";
    }
    
    //AUX Screen Strings
    namespace AUXScreen {
        const char AUX_TEMPS[] PROGMEM = "AUX Temps";
        const char AUX[] PROGMEM = "AUX";
        const char AUX_ONE[] PROGMEM = "AUX1";
        const char AUX_TWO[] PROGMEM = "AUX2";
#ifndef DIRECT_FIRED_RIMS
        const char AUX_THREE[] PROGMEM = "AUX3";
#endif
    }
    
    //Main Menu Strings
    namespace MainMenu {
        const char EDIT_PROGRAM[] PROGMEM = "Edit Program";
        const char START_PROGRAM[] PROGMEM = "Start Program";
        const char RESET_ALL[] PROGMEM = "Reset All";
        const char SYSTEM_SETUP[] PROGMEM = "System Setup";
        const char DRAIN_ERR1[] PROGMEM = "Cannot drain while";
        const char DRAIN_ERR2[] PROGMEM = "mash or boil zone";
        const char DRAIN_ERR3[] PROGMEM = "is active";
    }
    
    //Fill Menu Strings
    namespace FillMenu {
        const char AUTO_FILL[] PROGMEM = "Auto Fill";
        const char MASH_TARGET[]  PROGMEM = "Mash Target";
        const char MASH_TARGET_VOL[] PROGMEM = "Mash Target Vol";
    }
    
    //Mash Menu Strings
    namespace MashMenu {
        const char MASH_SETPOINT[] PROGMEM = "Mash Setpoint";
        const char MASH_TIMER[] PROGMEM = "Mash Timer";
    }
    
    //Sparge Menu Strings
    namespace SpargeMenu {
        const char AUTO_IN[] PROGMEM = "Auto In";
        const char AUTO_OUT[] PROGMEM = "Auto Out";
        const char AUTO_FLY[] PROGMEM = "Auto Fly";
        const char KETTLE_TARGET[] PROGMEM = "Kettle Target";
        const char KETTLE_TARGET_VOL[] PROGMEM = "Kettle Target Vol";
    }
    
    //Boil Menu Strings
    namespace BoilMenu {
        const char BOIL_CTRL[] PROGMEM = "Boil Ctrl: ";
        const char BOIL_TEMP[] PROGMEM = "Boil Temp"; // Also used in system setup (outputs config)
        const char BOIL_POWER[] PROGMEM = "Boil Power"; // Also used in system setup (outputs config)
        const char BOIL_TIMER[] PROGMEM = "Boil Timer";
    }
    
    //Program Strings
    namespace Program {
        //Program Menu Strings
        namespace ProgramMenu {
            const char PROG_NAME[] PROGMEM = "Program Name:";
            const char EDIT_PROG[] PROGMEM = "Edit Program";
            const char GRAIN_TEMP[] PROGMEM = "Grain Temp";
            const char START[] PROGMEM = "Start";
            const char DELAY_START[] PROGMEM = "Delay Start";
            const char START_FAILED[] PROGMEM = "Program start failed";
            const char START_ERR1[] PROGMEM = "Cannot start program";
            const char START_ERR2[] PROGMEM = "while mash zone is";
            const char START_ERR3[] PROGMEM = "active.";
            const char BATCH_VOL[] PROGMEM = "Batch Vol:";
            const char GRAIN_WT[] PROGMEM = "Grain Wt:";
            const char BOIL_LEN[] PROGMEM = "Boil Length";
            const char MINUTES_IND[] PROGMEM = " Min";
            const char MASH_RATIO[] PROGMEM = "Mash Ratio";
            const char RATIO_TO_1[] PROGMEM = ":1";
            const char NO_SPARGE[] PROGMEM = "NoSparge";
            const char HLT_TEMP[] PROGMEM = "HLT Temp:";
            const char SPARGE_TEMP[] PROGMEM = "Sparge Temp";
            const char PITCH_TEMP[] PROGMEM = "Pitch Temp";
            const char MASH_SCHED[] PROGMEM = "Mash Schedule";
            const char HEAT_STRIKE_IN[] PROGMEM = "Heat Strike In:";
            const char MASH[] PROGMEM = "MASH";
            const char UNKWN[] PROGMEM = "UNKWN";
            const char PROG_CALCS[] = "Program Calcs";
            const char BATCH_VOLUME[] PROGMEM = "Batch Volume";
            const char GRAIN_WEIGHT[] PROGMEM = "Grain Weight";
#ifdef USE_METRIC
            const char RATIO_UNITS[] PROGMEM = " l/kg";
#else
            const char RATIO_UNITS[] PROGMEM = "qts/lb";
#endif
            const char STRIKE_TEMP[] PROGMEM = "Strike Temp:";
            const char STRIKE_VOL[] PROGMEM = "Strike Vol:";
            const char SPARGE_VOL[] PROGMEM = "Sparge Vol:";
            const char PREBOIL_VOL[] PROGMEM = "Preboil Vol:";
            const char GRAIN_VOL[] PROGMEM = "Grain Vol:";
            const char GRAIN_LOSS[] PROGMEM = "Grain Loss:";
        }
        
        //Hop Schedule
        namespace HopSchedule {
            const char AT_BOIL[] PROGMEM = "At Boil";
            const char MIN_SPACE[] PROGMEM = " Min";
            const char MIN_2SPACE[] PROGMEM = "  Min";
            const char MIN_3SPACE[] PROGMEM = "   Min";
        }
        
        //HLT Warning
        namespace HLTWarning {
            const char HLT_CAP_WARN[] PROGMEM = "HLT Capacity Issue";
        }
        
        //Mash Warning
        namespace MashWarning {
            const char MASH_CAP_WARN[] PROGMEM = "Mash Capacity Issue";
        }
        
        //Boil Warning
        namespace BoilWarning {
            const char BOIL_CAP_WARN[] PROGMEM = "Boil Capacity Issue";
        }
        
        //Program Error Strings
        namespace StepError {
            const char FAILED_ADV1[] PROGMEM = "Failed to advance";
            const char FAILED_ADV2[] PROGMEM = "program.";
        }
    }
    
    //Vessel Strings
    namespace Vessel {
        const char TITLE_VS_HLT[] PROGMEM = "HLT";
        const char TITLE_VS_MASH[] PROGMEM = "Mash";
        const char TITLE_VS_KETTLE[] PROGMEM = "Kettle";
#ifdef PID_FLOW_CONTROL
        const char TITLE_VS_PUMP[] PROGMEM = "Pump";
#elif defined USESTEAM
        const char TITLE_VS_STEAM[] PROGMEM = "Steam";
#endif
    }
    
    //E-Stop Strings
    namespace EStop {
        const char ESTOP_TRIGGERED[] PROGMEM = "E-Stop Triggered";
        const char CLEAR_ALARM[] PROGMEM = ">Clear Alarm";
        const char CLEAR_ESTOP[] PROGMEM = " Clear E-Stop";
    }
    
    //Direct Fired RIMS Strings
    namespace DFRims {
#ifdef DIRECT_FIRED_RIMS
        const char AT[] PROGMEM = "At";
        const char SET[] PROGMEM = "Set";
        const char RIMS[] PROGMEM = "RIMS";
#endif
    }
    
    //Abort Confirm Dialog
    namespace AbortDialog {
        const char ABORT_OP1[] PROGMEM = "Abort operation and";
        const char ABORT_OP2[] PROGMEM = "reset setpoints,";
        const char ABORT_OP3[] PROGMEM = "timers and outputs?";
        const char RESET[] PROGMEM = "Reset";
    }
    
    //Confirm Delete Dialog
    namespace DeleteDialog {
        const char DELETE_ITEM[] PROGMEM = "Delete Item?";
    }
    
    //Print Timer
    namespace Timer {
        const char TIMER_SEP[] PROGMEM = ":  :";
    }
    
    
    //System Setup Menu
    namespace SystemSetup {
#ifndef UI_NO_SETUP
        const char TEMP_SENSORS[] PROGMEM = "Temperature Sensors";
        const char OUTPUTS[] PROGMEM = "Outputs";
        const char VOLS_CAPS[] PROGMEM = "Volume/Capacity";
#ifdef PVOUT
        const char VALVE_PROFILES[] PROGMEM = "Valve Profiles";
#ifdef PVOUT_TYPE_MODBUS
        const char RS485_OUTPUTS[] PROGMEM = "RS485 Outputs";
#endif
#endif
#ifdef UI_DISPLAY_SETUP
        const char DISPLAY_S[] PROGMEM = "Display";
#endif
#ifdef RGBIO8_ENABLE
#ifdef RGBIO8_SETUP
        const char RGB_SETUP[] PROGMEM = "RGB Setup";
#endif
#endif
#ifdef DIGITAL_INPUTS
        const char TRIGGERS[] PROGMEM = "Triggers";
#endif
        
        const char* const TITLE_VS[] PROGMEM = {
            Vessel::TITLE_VS_HLT,
            Vessel::TITLE_VS_MASH,
            Vessel::TITLE_VS_KETTLE
            
#ifdef PID_FLOW_CONTROL
            , TITLE_VS_PUMP
#elif defined USESTEAM
            , TITLE_VS_STEAM
#endif
        };
        
        //Temp sensor Assign Menu
        namespace TempSensorAssign {
            const char BREW_KETTLE[] PROGMEM = "Brew Kettle";
            const char WATER_IN[] PROGMEM = "H20 In";
            const char WATER_OUT[] PROGMEM = "H20 Out";
            const char BEER_OUT[] PROGMEM = "Beer Out";
            const char AUX1[] PROGMEM = "Aux 1";
            const char AUX2[] PROGMEM = "Aux 2";
            const char AUX3[] PROGMEM = "Aux 3";
#ifdef RIMS_TEMP_SENSOR
            const char RIMS_SENSOR[] PROGMEM = "RIMS";
#endif
            const char ASSIGN_SENSOR[] PROGMEM = "Assign Temp Sensor";
            const char SCAN_BUS[] PROGMEM = "Scan Bus";
            const char DELETE_ADDRESS[] PROGMEM = "Delete Address";
            const char DISCONNECT_WARN1[] PROGMEM = "Disconnect all other";
            const char DISCONNECT_WARN2[] PROGMEM = "temp sensors now";
            const char ASSIGN_BAD_TEMP[] PROGMEM = "---";
        }
        
        //Output Config Menu
        namespace OutputConfig {
            const char HLT_MODE[] PROGMEM = "HLT Mode: ";
            const char MASH_MODE[] PROGMEM = "Mash Mode: ";
            const char KETTLE_MODE[] PROGMEM = "Kettle Mode: ";
#ifdef PID_FLOW_CONTROL
            const char SPARGE_PUMP_MODE[] PROGMEM = "Sparge Pump: ";
#elif defined USESTEAM
            const char STEAM_MODE[] PROGMEM = "Steam Mode: ";
            const char PRES_UNIT[] PROGMEM = "mV/kPa";
            const char CALIB_ZERO[] PROGMEM = "Calibrate Zero?";
#endif
            const char PID_MODE[] PROGMEM = "PID";
            const char ON_OFF_MODE[] PROGMEM = "On/Off";
            const char PIDCYCLE[] PROGMEM = " PID Cycle";
            const char PIDGAIN[] PROGMEM = " PID Gain";
            const char HYSTERESIS[] PROGMEM = " Hysteresis";
#ifdef PID_FLOW_CONTROL
            const char PUMPFLOW[] PROGMEM = "Pump Flow Rate";
#elif defined USESTEAM
            const char STEAMPRESS[] PROGMEM = "Steam Target";
            const char STEAMSENSOR[] PROGMEM = "Steam Sensor Sens";
            const char STEAMZERO[] PROGMEM = "Steam Zero Calib";
#endif
        }
        
        //PID Config Menu
        namespace PIDConfig {
            const char PID_SEP[] PROGMEM = "P:     I:     D:    ";
        }
        
        //Volumes Config Menu
        namespace VolumeConfig {
            const char EVAP_RATE[] PROGMEM = "Evaporation Rate";
            const char CAPACITY[] PROGMEM = " Capacity";
            const char DEADSPACE[] PROGMEM = " Dead Space";
            const char CALIBRATION[] PROGMEM = " Calibration";
#ifdef BOIL_OFF_GALLONS
#ifdef  USEMETRIC
            const char EVAP_RATE_UNIT[] PROGMEM = "l/hr";
#else
            const char EVAP_RATE_UNIT[] PROGMEM = "0.1g/hr";
#endif
#else
            const char EVAP_RATE_UNIT[] PROGMEM = "%/hr";
#endif
        }
        
        //Volume Calibration Menu
        namespace VolumeCalibration {
            const char OPEN_PAREN[] PROGMEM = " (";
            const char CLOSE_PAREN[] PROGMEM = ")";
            const char OPEN[] PROGMEM = "OPEN";
            const char CURR_VOL[] PROGMEM = "Current Volume:";
            const char CALIBRATE[] PROGMEM = "Calibrate";
            const char UPDATE[] PROGMEM = "Update ";
            const char TO[] PROGMEM = " TO ";
            const char MANUAL_ENTRY[] PROGMEM = "Manual Entry";
            const char MANUAL_VOL_ENTRY[] PROGMEM = "Manual Volume Entry";
        }
        
        //Valve Configuration Menu
        namespace ValveProfileConfig {
            const char TEST[] PROGMEM = "Test";
            const char SAVE[] PROGMEM = "Save"; // Also used in Adjust LCD Menu
            const char ONE[] PROGMEM = "1";
            const char CARROT[] PROGMEM = "^";
            const char OPEN_SQR[] PROGMEM = "[";
            const char CLOSE_SQR[] PROGMEM = "]";
        }
        
        //MODBUS Ouptut Config Menu
        namespace MODBUSOutputConfig {
#if defined PVOUT && defined PVOUT_TYPE_MODBUS
            const char BOARD[] PROGMEM = "Board ";
            const char DISABLED[] PROGMEM = ": DISABLED";
            const char CONNECTED[] PROGMEM = ": CONNECTED";
            const char TIMEOUT[] PROGMEM = ": TIMEOUT";
            const char ERROR[] PROGMEM = "ERROR";
            const char ADDRESS[] PROGMEM = "Address: ";
            const char NA[] PROGMEM = "N/A";
            const char REGISTER[] PROGMEM = "Register: ";
            const char COUNT[] PROGMEM = "Count: ";
            const char OFFSET[] PROGMEM = "Offset: ";
            const char AUTO_ASSIGN[] PROGMEM = "Auto Assign";
            const char ID_MODE[] PROGMEM = "ID Mode: ";
            const char RELAY_ADDRESS[] PROGMEM = "RS485 Relay Address";
            const char COIL_REGISTER[] PROGMEM = "Coil Register";
            const char COIL_COUNT[] PROGMEM = "Coil Count";
            const char OUTPUT_OFFSET[] PROGMEM = "Output Offset";
            const char CLICK_RESET[] PROGMEM = "Click/hold to reset";
            const char OUTPUT_BOARD[] PROGMEM = "output board then";
            const char CLICK_TO_ACTIVATE[] PROGMEM = "click to activate.";
            const char ASSIGN_TIMEOUT[] PROGMEM = "Timeout";
            const char ASSIGN_ERROR[] PROGMEM = "Error ";
            const char ASSIGN_RETRY[] PROGMEM = ": Retry?";
            const char NEW_ADDRESS[] PROGMEM = "New Address";
            const char UPDATE_FAIL[] PROGMEM = "Update Failed";
            const char UPDATE_FAIL_CONTINUE[] PROGMEM = "> Continue <";
#endif
        }
      
        //RGBIO8 Config Menu
        namespace RGBIO {
#if defined(RGBIO8_ENABLE) && defined(RGBIO8_SETUP)
            const char TARGET_ADDR[] PROGMEM = "Target Address";
            const char SET_ADDR[] PROGMEM = "Set Address";
            const char IDENTIFY[] PROGMEM = "Identify";
            const char RESTART[] PROGMEM = "Restart";
#endif
        }
        
        //Init EEPROM
        namespace InitEEPROM {
            const char RESET_CONFIG[] PROGMEM = "Reset Configuration?";
        }
        
        //Adjust LCD Menu
        namespace LCDConfig {
#ifdef UI_DISPLAY_SETUP
            const char ADJUST_LCD[] PROGMEM = "Adjust LCD";
            const char BRIGHTNESS[] PROGMEM = "Brightness:";
            const char CONTRAST[] PROGMEM = "Contrast:";
#endif
        }
        
        //Triggers Menu
        namespace TriggersConfig {
#ifdef DIGITAL_INPUTS
            const char ESTOP[] PROGMEM = "E-Stop: ";
            const char SPARGE_MAX[] PROGMEM = "Sparge Max: ";
            const char HLT_MIN[] PROGMEM = "HLT Min: ";
            const char MASH_MIN[] PROGMEM = "Mash Min: ";
            const char KETTLE_MIN[] PROGMEM = "Kettle Min: ";
            const char NONE[] PROGMEM = "None";
            const char INPUT_PIN_NONE[] PROGMEM = "Input Pin (0=None):";
#endif
        }
        
#endif //UI_NO_SETUP
    }
    
    namespace Units {
#ifdef USEMETRIC
        const char VOLUNIT[] PROGMEM = "l";
        const char WTUNIT[] PROGMEM = "kg";
        const char TUNIT[] PROGMEM = "C";
#ifdef PID_FLOW_CONTROL
        const char PUNIT[] PROGMEM = "0.1*l/m";
#else
        const char PUNIT[] PROGMEM = "kPa";
#endif
#else
        const char VOLUNIT[] PROGMEM = "gal";
        const char WTUNIT[] PROGMEM = "lb";
        const char TUNIT[] PROGMEM = "F";
#ifdef PID_FLOW_CONTROL
        const char PUNIT[] PROGMEM = "0.1*q/m";
#else
        const char PUNIT[] PROGMEM = "psi";
#endif
#endif //USEMETRIC
    }

}

#endif
