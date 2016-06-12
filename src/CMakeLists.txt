# Source files for BrewTroller
set(SOURCE_FILES
        BrewCore.cpp
        BrewCore.h
        BrewTroller.cpp
        BrewTroller.h
        Com.cpp
        Com.h
        Com_BTnic.h
        Com_BTPD.cpp
        Com_BTPD.h
        Com_RGBIO8.cpp
        Com_RGBIO8.h
        Com_S0ASC.cpp
        EEPROM.cpp
        EEPROM.h
        Enum.h
        Events.cpp
        Events.h
        HardwareProfile.h
        Outputs.cpp
        Outputs.h
        PVOut.h
        PVOut.cpp
        StepLogic.cpp
        StepLogic.h
        Temp.cpp
        Temp.h
        Timer.cpp
        Timer.h
        UI.cpp
        UI.h
        UI_LCD.h
        UIStrings.h
        UIStrings.cpp
        Util.cpp
        Util.h
        Volume.cpp
        Volume.h)
# Workaround to enable generated Config header to include Enum header
#    *The Config header should get refactored to remove its dependancy on the Enum header*
set(ENUM_HEADER "${CMAKE_CURRENT_LIST_DIR}/Enum.h")

####################
# User selectable compile time options
####################
set(board  "OT_DX1_HERMS" CACHE STRING "BrewTroller Hardware Board Target, available options:
BT_10_21                --> 1.0 - 2.1
BT_22_24                --> 2.2 - 2.4
BT_3X                   --> 3.X
BT_4P                   --> 4.X Pro
OT_BX1_HERMS            --> BX1 HERMS
OT_BX1_HERMSI2CLCD      --> BX1 HERMS + I2CLCD
OT_BX1_RIMSDF           --> BX1 RIMS / Direct Fired
OT_BX1_RIMSDFI2CLCD     --> BX1 RIMS / Direct Fired + I2CLCD
OT_BX1_SINGLEVESSEL     --> BX1 Single Vessel
OT_BX1_STEAMPWM         --> BX1 Steam or PWM Pump
OT_BX1_STEAMPWMI2CLCD   --> BX1 Steam or PWM Pump + I2CLCD
OT_DX1_HERMS            --> DX1 HERMS
OT_DX1_RIMSDF           --> DX1 RIMS / Direct Fired
OT_DX1_SINGLEVESSEL     --> DX1 Single Vessel
OT_DX1_STEAMPWM         --> DX1 Steam or PWM Pump
OT_EX1_HERMS            --> EX1 HERMS
OT_EX1_RIMSDF           --> EX1 RIMS / Direct Fired
OT_EX1_SINGLEVESSEL     --> EX1 Single Vessel
OT_EX1_STEAMPWM         --> EX1 Steam or PWM Pump
BT_PHOENIX_HERMS        --> Phoenix HERMS
BT_PHOENIX_RIMS         --> Phoenix RIMS / Direct Fired
BT_PHOENIX_SINGLEVESSEL --> Phoenix Single Vessel
BT_PHOENIX_STEAMPWM     --> Phoenix Steam or PWM Pump") # Default to DX1 Herms build
add_definitions("-D${board}")
message(STATUS "Building BrewTroller for   " ${board})

#####
# Basic Options
#####
option(USEMETRIC "Use Metric Units" OFF)

#####
# Vessel Options
#####
option(HLT_AS_KETTLE "Use HLT Vessel as Kettle Vessel" OFF)
option(KETTLE_AS_MASH "Use Kettle Vessel as Mash Vessel" OFF)
option(SINGLE_VESSEL_SUPPORT "Enable Single Vessel System Configuration" OFF)

#####
# Volume Options
#####
option(VOLUME_MANUAL "Disable Volume Sensors" OFF)

# Generate Configuration header
configure_file(Config.in.h "${PROJECT_BINARY_DIR}/Config.h")
include_directories(${PROJECT_BINARY_DIR})

#####
# Home Screen Option
#####
option(CUSTOM_HOME_SCREEN "Use Custom Home Screen" OFF)
if(CUSTOM_HOME_SCREEN)
    message(STATUS "Enabling custom home screen")
    add_definitions("-DCUSTOM_HOME_SCREEN")
endif(CUSTOM_HOME_SCREEN)


add_avr_executable(BrewTroller-${board} ${SOURCE_FILES})

# Link libraries
target_link_libraries(BrewTroller-${board}.elf BT-libs)