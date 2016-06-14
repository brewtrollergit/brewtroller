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

#include <Arduino.h>
#include "BrewTroller.h"
#include "HardwareProfile.h"
#include "Enum.h"
#include "Outputs.h"
#include "Com.h"
#include "UI.h"
#include "Events.h"
#include "Timer.h"
#include "EEPROM.h"

void eventHandler(byte eventID, int eventParam) {
  //Global Event handler
  //EVENT_STEPINIT: Nothing to do here (Pass to UI handler below)
  //EVENT_STEPEXIT: Nothing to do here (Pass to UI handler below)
  if (eventID == EVENT_SETPOINT) {
    //Setpoint Change (Update AutoValve Logic)
    byte avProfile = vesselAV(eventParam);
    byte vlvHeat = vesselVLVHeat(eventParam);
    byte vlvIdle = vesselVLVIdle(eventParam);
    
    if (setpoint[eventParam]) autoValve[avProfile] = 1;
    else { 
      autoValve[avProfile] = 0; 
      if (vlvConfigIsActive(vlvIdle)) bitClear(actProfiles, vlvIdle);
      if (vlvConfigIsActive(vlvHeat)) bitClear(actProfiles, vlvHeat);
    } 
  }
  
  #ifndef NOUI
    //Pass Event Info to UI Event Handler
    uiEvent(eventID, eventParam);
  #endif

  //Pass Event Info to Com Event Handler
  comEvent(eventID, eventParam);

}

#ifdef DIGITAL_INPUTS
  void triggerSetup() {
    //If EEPROM is not initialized skip trigger init
    if (checkConfig()) return;
    //For each logical trigger type see what the assigned trigger pin is (if any)
    for (byte i = 0; i < NUM_TRIGGERS; i++) {
      if (TriggerPin[i] != NULL) TriggerPin[i]->detachPCInt();
      if (getTriggerPin(i)) {
        TriggerPin[i] = &digInPin[getTriggerPin(i) - 1];
        if (i == TRIGGER_ESTOP) TriggerPin[i]->attachPCInt(CHANGE, eStopISR);
        else if (i == TRIGGER_SPARGEMAX) TriggerPin[i]->attachPCInt(RISING, spargeMaxISR);
        else if (i == TRIGGER_HLTMIN) TriggerPin[i]->attachPCInt(FALLING, hltMinISR);
        else if (i == TRIGGER_MASHMIN) TriggerPin[i]->attachPCInt(FALLING, mashMinISR);
        else if (i == TRIGGER_KETTLEMIN) TriggerPin[i]->attachPCInt(FALLING, kettleMinISR);
      }
    }
  }

  void eStopISR() {
    //Either clear E-Stop condition if e-Stop trigger goes high
    //or perform E-Stop actions on trigger low
    if (TriggerPin[TRIGGER_ESTOP]->get()) estop = 0;
    else {
      estop = 1;
      setAlarm(1);
      processHeatOutputs();
      #ifdef PVOUT
        updateOutputs();
      #endif
      updateTimers();
    }
  }
  
  void spargeMaxISR() {
    bitClear(actProfiles, VLV_SPARGEIN);
  }
  
  void hltMinISR() {
    heatPin[VS_HLT].set(LOW);
    heatStatus[VS_HLT] = 0;
    bitClear(actProfiles, VLV_HLTHEAT);
  }
  
  void mashMinISR() {
    heatPin[VS_MASH].set(LOW);
    heatStatus[VS_MASH] = 0;
    bitClear(actProfiles, VLV_MASHHEAT);
    #ifdef DIRECT_FIRED_RIMS
      heatPin[VS_STEAM].set(LOW);
      heatStatus[VS_STEAM] = 0;
    #endif
  }
  
  void kettleMinISR() {
    heatPin[VS_KETTLE].set(LOW);
    heatStatus[VS_KETTLE] = 0;    
    bitClear(actProfiles, VLV_KETTLEHEAT);
  }
#endif
