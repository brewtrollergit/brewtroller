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

#ifndef __BT_UI_STRINGS_H__
#define __BT_UI_STRINGS_H__

#include "Config.h"
#include "HardwareProfile.h"

//Declare progmem attribute so other (CLANG) static analyzers don't complain
#ifndef __AVR__
#define __progmem__
#endif

#ifndef NOUI

//TODO: Namespace Strings into Top-Level NS (UI::Strings) and into use-case NSes (::MainMenu)

namespace UIStrings {
    
    namespace Generic {
        //Generic Menu strings
        extern const char OK[];
        extern const char CANCEL[];
        extern const char EXIT[];
        extern const char ABORT[];
        extern const char MENU[];
        extern const char SPACE[];
        extern const char CONTINUE[];
        extern const char TARGET[];
        extern const char ACTUAL[];
        extern const char GREATER_SYM[];
        extern const char LESS_SYM[];
        extern const char ON[];
        extern const char OFF[];
        extern const char COLON[];
        extern const char TEMPBLANK[];
        extern const char VOLBLANK[];
        extern const char AUTO[];
        extern const char MANUAL[];
        extern const char SET_TIMER[];
        extern const char PAUSE_TIMER[];
        extern const char START_TIMER[];
        extern const char COLON_SPACE[];
        extern const char PERC_SYM[];
        extern const char EMPTY[];
        extern const char BTVER[];
        extern const char DOT[];
    }
    
    namespace Shared {
        extern const char ALLOFF[];
        extern const char HLT_TARGET[]; // Fill & Sparge
        extern const char HLT_TARGET_VOL[]; // Fill & Sparge
        extern const char VALVE_ON[]; // Main Menu, Boil, Hops
        extern const char VALVE_OFF[]; // Main Menu, Boil, Hops
        extern const char HLT_SETPOINT[]; // Mash, Edit Prog
        extern const char HLTDESC[]; // ML Heat source, Assign Sensor
        extern const char MASHDESC[]; // ML Heat source, Assign Sensor
        extern const char SEC[]; // ML Heat Source, PID Cycle Time
        extern const char ZERO[]; // Hop Schedule, Valve Profile config
        extern const char DELETE[]; // Delete Dialog, Volume Calibration, RS485 Config
    }
    
    //EEPROM Setup/Missing Screen
    namespace EEPROMInit {
        extern const char MISSING_CONFIG[];
        extern const char INIT_EEPROM[];
        extern const char PLEASE_WAIT[];
    }
    
    //Home screen strings
    namespace HomeScreen {
        extern const char BUILD[];
        extern const char BT_URL[];
    }
    
    //Fill screen strings
    namespace FillScreen {
        extern const char FILL[];
        extern const char REFILL[];
    }
    
    //Valve profile name strings
    namespace ValveProfile {
        extern const char FILLHLT[];
        extern const char FILLMASH[];
        extern const char ADDGRAIN[];
        extern const char MASHHEAT[];
        extern const char MASHIDLE[];
        extern const char SPARGEIN[];
        extern const char SPARGEOUT[];
        extern const char BOILADDS[];
        extern const char KETTLELID[];
        extern const char CHILLH2O[];
        extern const char CHILLBEER[];
        extern const char BOILRECIRC[];
        extern const char DRAIN[];
        extern const char HLTHEAT[];
        extern const char HLTIDLE[];
        extern const char KETTLEHEAT[];
        extern const char KETTLEIDLE[];
        extern const char USER1[];
        extern const char USER2[];
        extern const char USER3[];
    }
    
    //Grain in step strings
    namespace GrainInScreen {
        extern const char GRAININ[];
    }
    
    //Mash step strings
    namespace MashStep {
        extern const char DELAY[];
        extern const char PREHEAT[];
        extern const char DOUGHIN[];
        extern const char ACID[];
        extern const char PROTEIN[];
        extern const char SACCH[];
        extern const char SACCH2[];
        extern const char MASHOUT[];
        extern const char ENDMASH[];
#ifdef DIRECT_FIRED_RIMS
        extern const char BADTEMP[];
        extern const char OCTOTHORPE[];
        extern const char ASTERISK[];
        extern const char DASH[];
#endif
    }
    
    //Sparge step strings
    namespace SpargeScreen {
        extern const char SPARGE[];
        extern const char FILLBOTH[];
        extern const char FLYSPARGE[];
    }
    
    //Boil step strings
    namespace BoilScreen {
        extern const char BOIL[];
        extern const char BOIL_AUTO[];
    }
    
    //Chill step state strings
    namespace ChillScreen {
        extern const char CHILL[];
        extern const char BEER[];
        extern const char H20[];
        extern const char IN[];
        extern const char OUT[];
        extern const char CHILL_BOTH[];
    }
    
    //AUX Screen Strings
    namespace AUXScreen {
        extern const char AUX_TEMPS[];
        extern const char AUX[];
        extern const char AUX_ONE[];
        extern const char AUX_TWO[];
#ifndef DIRECT_FIRED_RIMS
        extern const char AUX_THREE[];
#endif
    }
    
    //Main Menu Strings
    namespace MainMenu {
        extern const char EDIT_PROGRAM[];
        extern const char START_PROGRAM[];
        extern const char RESET_ALL[];
        extern const char SYSTEM_SETUP[];
        extern const char DRAIN_ERR1[];
        extern const char DRAIN_ERR2[];
        extern const char DRAIN_ERR3[];
    }
    
    //Fill Menu Strings
    namespace FillMenu {
        extern const char AUTO_FILL[];
        extern const char MASH_TARGET[];
        extern const char MASH_TARGET_VOL[];
    }
    
    //Mash Menu Strings
    namespace MashMenu {
        extern const char MASH_SETPOINT[];
        extern const char MASH_TIMER[];
    }
    
    //Sparge Menu Strings
    namespace SpargeMenu {
        extern const char AUTO_IN[];
        extern const char AUTO_OUT[];
        extern const char AUTO_FLY[];
        extern const char KETTLE_TARGET[];
        extern const char KETTLE_TARGET_VOL[];
    }
    
    //Boil Menu Strings
    namespace BoilMenu {
        extern const char BOIL_CTRL[];
        extern const char BOIL_TEMP[];
        extern const char BOIL_POWER[];
        extern const char BOIL_TIMER[];
    }
    
    //Program Strings
    namespace Program {
        //Program Menu Strings
        namespace ProgramMenu {
            extern const char PROG_NAME[];
            extern const char EDIT_PROG[];
            extern const char GRAIN_TEMP[];
            extern const char START[];
            extern const char DELAY_START[];
            extern const char START_FAILED[];
            extern const char START_ERR1[];
            extern const char START_ERR2[];
            extern const char START_ERR3[];
            extern const char BATCH_VOL[];
            extern const char GRAIN_WT[];
            extern const char BOIL_LEN[];
            extern const char MINUTES_IND[];
            extern const char MASH_RATIO[];
            extern const char RATIO_TO_1[];
            extern const char NO_SPARGE[];
            extern const char HLT_TEMP[];
            extern const char SPARGE_TEMP[];
            extern const char PITCH_TEMP[];
            extern const char MASH_SCHED[];
            extern const char HEAT_STRIKE_IN[];
            extern const char MASH[];
            extern const char UNKWN[];
            extern const char PROG_CALCS[];
            extern const char BATCH_VOLUME[];
            extern const char GRAIN_WEIGHT[];
            extern const char RATIO_UNITS[];
            extern const char STRIKE_TEMP[];
            extern const char STRIKE_VOL[];
            extern const char SPARGE_VOL[];
            extern const char PREBOIL_VOL[];
            extern const char GRAIN_VOL[];
            extern const char GRAIN_LOSS[];
        }
        
        //Hop Schedule
        namespace HopSchedule {
            extern const char AT_BOIL[];
            extern const char MIN_SPACE[];
            extern const char MIN_2SPACE[];
            extern const char MIN_3SPACE[];
        }
        
        //HLT Warning
        namespace HLTWarning {
            extern const char HLT_CAP_WARN[];
        }
        
        //Mash Warning
        namespace MashWarning {
            extern const char MASH_CAP_WARN[];
        }
        
        //Boil Warning
        namespace BoilWarning {
            extern const char BOIL_CAP_WARN[];
        }
        
        //Program Error Strings
        namespace StepError {
            extern const char FAILED_ADV1[];
            extern const char FAILED_ADV2[];
        }
    }
    
    //Vessel Strings
    namespace Vessel {
        extern const char TITLE_VS_HLT[];
        extern const char TITLE_VS_MASH[];
        extern const char TITLE_VS_KETTLE[];
#ifdef PID_FLOW_CONTROL
        extern const char TITLE_VS_PUMP[];
#elif defined USESTEAM
        extern const char TITLE_VS_STEAM[];
#endif
    }
    
    //E-Stop Strings
    namespace EStop {
        extern const char ESTOP_TRIGGERED[];
        extern const char CLEAR_ALARM[];
        extern const char CLEAR_ESTOP[];
    }
    
    //Direct Fired RIMS Strings
    namespace DFRims {
#ifdef DIRECT_FIRED_RIMS
        extern const char AT[];
        extern const char SET[];
        extern const char RIMS[];
#endif
    }
    
    //Abort Confirm Dialog
    namespace AbortDialog {
        extern const char ABORT_OP1[];
        extern const char ABORT_OP2[];
        extern const char ABORT_OP3[];
        extern const char RESET[];
    }
    
    //Confirm Delete Dialog
    namespace DeleteDialog {
        extern const char DELETE_ITEM[];
    }
    
    //Print Timer
    namespace Timer {
        extern const char TIMER_SEP[];
    }
    
    //System Setup Menu
    namespace SystemSetup {
#ifndef UI_NO_SETUP
        extern const char TEMP_SENSORS[];
        extern const char OUTPUTS[];
        extern const char VOLS_CAPS[];
#ifdef PVOUT
        extern const char VALVE_PROFILES[];
#ifdef PVOUT_TYPE_MODBUS
        extern const char RS485_OUTPUTS[];
#endif
#endif
#ifdef UI_DISPLAY_SETUP
        extern const char DISPLAY_S[];
#endif
#ifdef RGBIO8_ENABLE
#ifdef RGBIO8_SETUP
        extern const char RGB_SETUP[];
#endif
#endif
#ifdef DIGITAL_INPUTS
        extern const char TRIGGERS[];
#endif
        
        extern const char* const TITLE_VS[];
        
        namespace TempSensorAssign {
            //Temp sensor assign menu
            extern const char BREW_KETTLE[];
            extern const char WATER_IN[];
            extern const char WATER_OUT[];
            extern const char BEER_OUT[];
            extern const char AUX1[];
            extern const char AUX2[];
            extern const char AUX3[];
#ifdef RIMS_TEMP_SENSOR
            extern const char RIMS_SENSOR[];
#endif
            extern const char ASSIGN_SENSOR[];
            extern const char SCAN_BUS[];
            extern const char DELETE_ADDRESS[];
            extern const char DISCONNECT_WARN1[];
            extern const char DISCONNECT_WARN2[];
            extern const char ASSIGN_BAD_TEMP[];
        }
        
        //Output Config Menu
        namespace OutputConfig {
            extern const char HLT_MODE[];
            extern const char MASH_MODE[];
            extern const char KETTLE_MODE[];
#ifdef PID_FLOW_CONTROL
            extern const char SPARGE_PUMP_MODE[];
#elif defined USESTEAM
            extern const char STEAM_MODE[];
            extern const char PRES_UNIT[];
            extern const char CALIB_ZERO[];
#endif
            extern const char PID_MODE[];
            extern const char ON_OFF_MODE[];
            extern const char PIDCYCLE[];
            extern const char PIDGAIN[];
            extern const char HYSTERESIS[];
#ifdef PID_FLOW_CONTROL
            extern const char PUMPFLOW[];
#elif defined USESTEAM
            extern const char STEAMPRESS[];
            extern const char STEAMSENSOR[];
            extern const char STEAMZERO[];
#endif
        }
        
        //PID Config Menu
        namespace PIDConfig {
            extern const char PID_SEP[];
        }
        
        //Volumes Config Menu
        namespace VolumeConfig {
            extern const char EVAP_RATE[];
            extern const char CAPACITY[];
            extern const char DEADSPACE[];
            extern const char CALIBRATION[];
            extern const char EVAP_RATE_UNIT[];
        }
        
        //Volume Calibration Menu
        namespace VolumeCalibration {
            extern const char OPEN_PAREN[];
            extern const char CLOSE_PAREN[];
            extern const char OPEN[];
            extern const char CURR_VOL[];
            extern const char CALIBRATE[];
            extern const char UPDATE[];
            extern const char TO[];
            extern const char MANUAL_ENTRY[];
            extern const char MANUAL_VOL_ENTRY[];
        }
        
        //Valve Configuration Menu
        namespace ValveProfileConfig {
            extern const char TEST[];
            extern const char SAVE[];
            extern const char ONE[];
            extern const char CARROT[];
            extern const char OPEN_SQR[];
            extern const char CLOSE_SQR[];
        }
        
        //MODBUS Output Config Menu
        namespace MODBUSOutputConfig {
#if defined PVOUT && defined PVOUT_TYPE_MODBUS
            extern const char BOARD[];
            extern const char DISABLED[];
            extern const char CONNECTED[];
            extern const char TIMEOUT[];
            extern const char ERROR[];
            extern const char ADDRESS[];
            extern const char NA[];
            extern const char REGISTER[];
            extern const char COUNT[];
            extern const char OFFSET[];
            extern const char AUTO_ASSIGN[];
            extern const char ID_MODE[];
            extern const char RELAY_ADDRESS[];
            extern const char COIL_REGISTER[];
            extern const char COIL_COUNT[];
            extern const char OUTPUT_OFFSET[];
            extern const char CLICK_RESET[];
            extern const char OUTPUT_BOARD[];
            extern const char CLICK_TO_ACTIVATE[];
            extern const char ASSIGN_TIMEOUT[];
            extern const char ASSIGN_ERROR[];
            extern const char ASSIGN_RETRY[];
            extern const char NEW_ADDRESS[];
            extern const char UPDATE_FAIL[];
            extern const char UPDATE_FAIL_CONTINUE[];
#endif
        }
        
        //RGBIO8 Config Menu
        namespace RGBIO {
#ifdef RGBIO8_ENABLE
#ifdef RGBIO8_SETUP
            extern const char TARGET_ADDR[];
            extern const char SET_ADDR[];
            extern const char IDENTIFY[];
            extern const char RESTART[];
#endif
#endif
        }
        
        //Init EEPROM
        namespace InitEEPROM {
            extern const char RESET_CONFIG[];
        }
        
        //Adjust LCD Menu
        namespace LCDConfig {
#ifdef UI_DISPLAY_SETUP
            extern const char ADJUST_LCD[];
            extern const char BRIGHTNESS[];
            extern const char CONTRAST[];
#endif
        }
        
        //Triggers Menu
        namespace TriggersConfig {
#ifdef DIGITAL_INPUTS
            extern const char ESTOP[];
            extern const char SPARGE_MAX[];
            extern const char HLT_MIN[];
            extern const char MASH_MIN[];
            extern const char KETTLE_MIN[];
            extern const char NONE[];
            extern const char INPUT_PIN_NONE[];
#endif
        }
        
#endif //UI_NO_SETUP
    }
    
    namespace Units {
        extern const char VOLUNIT[];
        extern const char WTUNIT[];
        extern const char TUNIT[];
        extern const char PUNIT[];
    }
    
#endif //NOUI

}

#endif