/*
   Copyright (C) 2014 BrewTroller

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


BrewTroller - Open Source Brewery Automation
Documentation, Forums and more information available at http://www.brewtroller.com
*/

#ifndef __BREWTROLLER_HWPROFILE__
#define __BREWTROLLER_HWPROFILE__

#if defined(BT_10_21)
    #include "HWProfiles/BrewTroller 1.0 - 2.1/HWProfile.h"

#elif defined(BT_22_24)
    #include "HWProfiles/BrewTroller 2.2 - 2.4/HWProfile.h"

#elif defined(BT_3X)
    #include "HWProfiles/BrewTroller 3.x/HWProfile.h"

#elif defined(BT_4P)
    #include "HWProfiles/BrewTroller 4.0 Pro/HWProfile.h"

#elif defined(OT_BX1_HERMS)
    #include "HWProfiles/OpenTroller BX1/HERMS/HWProfile.h"
#elif defined(OT_BX1_HERMSI2CLCD)
    #include "HWProfiles/OpenTroller BX1/HERMS I2CLCD/HWProfile.h"
#elif defined(OT_BX1_RIMSDF)
    #include "HWProfiles/OpenTroller BX1/RIMS or Direct Fired/HWProfile.h"
#elif defined(OT_BX1_RIMSDFI2CLCD)
    #include "HWProfiles/OpenTroller BX1/RIMS or Direct Fired I2CLCD/HWProfile.h"
#elif defined(OT_BX1_SINGLEVESSEL)
    #include "HWProfiles/OpenTroller BX1/Single Vessel/HWProfile.h"
#elif defined(OT_BX1_STEAMPWM)
    #include "HWProfiles/OpenTroller BX1/Steam or PWM Pump/HWProfile.h"
#elif defined(OT_BX1_STEAMPWMI2CLCD)
    #include "HWProfiles/OpenTroller BX1/Steam or PWM Pump I2CLCD/HWProfile.h"

#elif defined(OT_DX1_HERMS)
    #include "HWProfiles/OpenTroller DX1/HERMS/HWProfile.h"
#elif defined(OT_DX1_RIMSDF)
    #include "HWProfiles/OpenTroller DX1/RIMS or Direct Fired/HWProfile.h"
#elif defined(OT_DX1_SINGLEVESSEL)
    #include "HWProfiles/OpenTroller DX1/Single Vessel/HWProfile.h"
#elif defined(OT_DX1_STEAMPWM)
    #include "HWProfiles/OpenTroller DX1/Steam or PWM Pump/HWProfile.h"

#elif defined(OT_EX1_HERMS)
    #include "HWProfiles/OpenTroller EX1/HERMS/HWProfile.h"
#elif defined(OT_EX1_RIMSDF)
    #include "HWProfiles/OpenTroller EX1/RIMS or Direct Fired/HWProfile.h"
#elif defined(OT_EX1_SINGLEVESSEL)
    #include "HWProfiles/OpenTroller EX1/Single Vessel/HWProfile.h"
#elif defined(OT_EX1_STEAMPWM)
    #include "HWProfiles/OpenTroller EX1/Steam or PWM Pump/HWProfile.h"

#elif defined(BT_PHOENIX_HERMS)
    #include "HWProfiles/BrewTroller Phoenix/HERMS/HWProfile.h"
#elif defined(BT_PHOENIX_RIMS)
    #include "HWProfiles/BrewTroller Phoenix/RIMS or Direct Fired/HWProfile.h"
#elif defined(BT_PHOENIX_SINGLE_VESSEL)
    #include "HWProfiles/BrewTroller Phoenix/Single Vessel/HWProfile.h"
#elif defined(BT_PHOENIX_STEAM_PWM)
    #include "HWProfiles/BrewTroller Phoenix/Steam or PWM Pump/HWProfile.h"
#else
    #if !defined(BT_HWPROFILE)
        #error "BrewTroller Hardware type not defined"
    #endif
#endif

#if defined PVOUT_TYPE_GPIO || defined PVOUT_TYPE_MUX
#define PVOUT
#endif

#endif
