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
#include <pin.h>
#include "BrewCore.h"
#include "BrewTroller.h"
#include "Outputs.h"
#include "Timer.h"
#include "Temp.h"
#include "Volume.h"
#include "StepLogic.h"
#include "Com.h"

byte scheduler;

enum schedulerTasks {
  SCHEDULETASK_TIMERS,
#ifndef NOUI
  SCHEDULETASK_LCD,
#endif
  SCHEDULETASK_TEMPS,
  SCHEDULETASK_BUZZER,
  SCHEDULETASK_VOLS,
#ifdef FLOWRATE_CALCS
  SCHEDULETASK_FLOWRATES,
#endif
  SCHEDULETASK_PROGRAMS,
  SCHEDULETASK_COMS,
#ifdef PVOUT
  SCHEDULETASK_OUTPUTPROFILES,
#endif
  SCHEDULETASK_COUNT
};

void brewCore() {

  //START HIGH PRIORITY: Time-sensitive updates perfromed on each iteration
  #ifdef HEARTBEAT
    heartbeat();
  #endif
  processHeatOutputs();
  //END HIGH PRIORITY
  
  //START NORMAL PRIORITY: Updated in turn
  switch (scheduler) {
#ifndef NOUI
    case SCHEDULETASK_LCD:
      LCD.update();
      break;
#endif  

    case SCHEDULETASK_TIMERS:
      //Timers: Timer.pde
      updateTimers();
      break;
      
    case SCHEDULETASK_TEMPS:
     //temps: Temp.pde
     updateTemps();
     break;

    case SCHEDULETASK_BUZZER:
      //Alarm update allows to have a beeping alarm
      updateBuzzer();
      break;
      
    case SCHEDULETASK_VOLS:
      //Volumes: Volume.pde
      updateVols();
      break;
      
#ifdef FLOWRATE_CALCS
    case SCHEDULETASK_FLOWRATES:
      updateFlowRates();
      break;
#endif      
      
    case SCHEDULETASK_PROGRAMS:
      //Step Logic: StepLogic.pde
      programThreadsUpdate();
      break;
      
    case SCHEDULETASK_COMS:
      //Communications: Com.pde
      updateCom();
      break;
      
#ifdef PVOUT
    case SCHEDULETASK_OUTPUTPROFILES:
      //Auto Valve Logic: Outputs.pde
      processAutoValve();
      
      //Set Valve Outputs based on active valve profiles (if changed): Outputs.pde
      updateOutputs();
      break;
#endif 
  }
  
  if(++scheduler >= SCHEDULETASK_COUNT)
    scheduler = 0;
}

#ifdef HEARTBEAT
  unsigned long hbStart = 0;
  void heartbeat() {
    if (millis() - hbStart > 750) {
      hbPin.toggle();
      hbStart = millis();
    }
  }
#endif
