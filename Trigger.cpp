/*  
   Copyright (C) 2009 - 2012 Open Source Control Systems, Inc.

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


BrewTroller - Open Source Brewing Controller
Documentation, Forums and more information available at http://www.brewtroller.com
*/

#include "Trigger.h"


unsigned long Trigger::compute(unsigned long filterValue) {
  //Filter is set but filter condition is not active; return no disables
  if (filterBits && !(filterValue & filterBits)) {
    releaseMillis = 0;
    return 0;
  }
  
  unsigned long now = millis();
  
  //Check if release hysteresis is active
  if (releaseMillis > now)
    return disableMask;
    
  //Not in hysteresis so if not active return 0
  if (activeLow == getRawValue()) {
    releaseMillis = 0;
    return 0;
  }
  
  //Trigger is active
  //If release isn't set then trigger just went active, store timestamp for release
  if (releaseMillis == 0)
    releaseMillis = now + releaseHysteresis * 1000;
 
  return disableMask;
}

TriggerGPIO::TriggerGPIO(byte p, boolean aLow, unsigned long filter, unsigned long dMask, byte rHysteresis) {
  inputPin.setup(p, INPUT);
  activeLow = aLow;
  filterBits = filter;
  disableMask = dMask;
  releaseHysteresis = rHysteresis;
  releaseMillis = 0;
}

TriggerGPIO::~TriggerGPIO() {
  
}

boolean TriggerGPIO::getRawValue(void) {
  return inputPin.get();
}
  
TriggerValue::TriggerValue(unsigned long *v, boolean aLow, unsigned long t, unsigned long filter, unsigned long dMask, byte rHysteresis) {
  value = v;
  threshold = t;
  activeLow = aLow;
  filterBits = filter;
  disableMask = dMask;
  releaseHysteresis = rHysteresis;
  releaseMillis = 0;
}

TriggerValue::~TriggerValue() {
  
}

boolean TriggerValue::getRawValue(void) {
  return (*value > threshold);
}