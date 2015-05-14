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
unsigned long lastHop, grainInStart;
unsigned int boilAdds, triggered;

void programThreadsUpdate() {
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++)
    if (programThread[i].activeStep != INDEX_NONE)
      programThreadSignal(programThread + i, STEPSIGNAL_UPDATE);
}

/**
 * Used to determine if the given step is the active step in the program.
 */
boolean brewStepIsActive(byte brewStep) {
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++)
    if (programThread[i].activeStep == brewStep)
      return true; 
  return false;
}

/**
 * Usd to determine if the given ZONE is the active ZONE in the program.
 * Returns true is any step in the given ZONE is the active step, false otherwise.
 */
boolean zoneIsActive(byte brewZone) {
  byte stepMin, stepMax;
  switch (brewZone) {
    case ZONE_MASH:
      stepMin = BREWSTEP_FILL;
      stepMax = BREWSTEP_SPARGE;
      break;
    case ZONE_BOIL:
      stepMin = BREWSTEP_FILL;
      stepMax = BREWSTEP_SPARGE;
      break;
  }    

  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++)
    if (programThread[i].activeStep >= stepMin && programThread[i].activeStep <= stepMax)
      return true;
  return false;
}

byte programThreadActiveStep(byte threadIndex) {
  return programThread[threadIndex].activeStep;
}

void programThreadRecipeName(byte threadIndex, char *returnValue) {
  getProgName(programThread[threadIndex].recipe, returnValue);
}

byte programThreadRecipeIndex(byte threadIndex) {
  return programThread[threadIndex].recipe;  
}

struct ProgramThread *programThreadAcquire() {
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++)
    if (programThread[i].activeStep == INDEX_NONE)
      return programThread + i;
  return 0;
}

void (*brewStepFunc(byte brewStep))(enum StepSignal, struct ProgramThread *) {
  static void (*brewStepFunctionMap[BREWSTEP_COUNT])(enum StepSignal, struct ProgramThread *) = {
    &brewStepFill,
    &brewStepDelay,
    &brewStepPreheat,
    &brewStepGrainIn,
    &brewStepRefill,
    &brewStepDoughIn,
    &brewStepAcid,
    &brewStepProtein,
    &brewStepSacch,
    &brewStepSacch2,
    &brewStepMashOut,
    &brewStepMashHold,
    &brewStepSparge,
    &brewStepBoil,
    &brewStepChill
  };
  
  if (brewStep < BREWSTEP_COUNT)
    return brewStepFunctionMap[brewStep];
  return 0;
}

struct ProgramThread *programThreadInit(byte recipe) {
  return programThreadInit(recipe, BREWSTEP_FILL);
}

struct ProgramThread *programThreadInit(byte recipe, byte brewStep) {
  //Invalid recipe or Recipe 'None'
  if(recipe >= RECIPE_MAX)
    return 0;

  if (brewStepZoneInUse(brewStep))
    return 0;

  struct ProgramThread *thread = programThreadAcquire();
  if (!thread)
    return 0;

  //Determine what function we need to call
  void (*stepFunc)(enum StepSignal, struct ProgramThread *) = brewStepFunc(brewStep);
  if (!stepFunc)
    return 0;

  //If we made it without an abort, save the thread without an activeStep
  thread->activeStep = INDEX_NONE;
  thread->recipe = recipe;
  programThreadSave(thread);
  
  //Signal function directly as there is no activeStep for signalProgramThread()
  (*stepFunc)(STEPSIGNAL_INIT, thread);
  
  //Abort if the brew step is still unset
  if (thread->activeStep == INDEX_NONE)
    return 0;
  return thread;
}

void programThreadSave(struct ProgramThread *thread) {
  int index = thread - programThread;
  eepromSaveProgramThread(index, thread);
}

void programThreadSetStep(struct ProgramThread *thread, byte brewStep) {
  byte lastStep = thread->activeStep;
  thread->activeStep = brewStep;
  programThreadSave(thread);
  if (brewStep == INDEX_NONE)
    eventHandler(EVENT_STEPEXIT, lastStep);
  else
    eventHandler(EVENT_STEPINIT, thread->activeStep);
}

boolean brewStepZoneInUse(byte brewStep) {
  if (brewStep >= BREWSTEP_FILL && brewStep <= BREWSTEP_MASHHOLD && zoneIsActive(ZONE_MASH))
    return 1;  
  else if (brewStep == BREWSTEP_SPARGE && (zoneIsActive(ZONE_MASH) || zoneIsActive(ZONE_BOIL)))
    return 1;  
  return 0;
}

void programThreadSignal(struct ProgramThread *thread, enum StepSignal signal) {
  void (*stepFunc)(enum StepSignal, struct ProgramThread *) = brewStepFunc(thread->activeStep);
  if (stepFunc)
    (*stepFunc)(signal, thread);
}

//Supports BTnic commands against brewsteps with no knowledge of threads
void brewStepSignal(byte brewStep, enum StepSignal signal) {
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++) {
    if (programThread[i].activeStep == brewStep) {
      void (*stepFunc)(enum StepSignal, struct ProgramThread *) = brewStepFunc(brewStep);
      if (stepFunc)
        (*stepFunc)(signal, programThread + i);
    }
  }
}

void programThreadResetAll() {
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++)
    programThreadSignal(programThread + i, STEPSIGNAL_ABORT); //Abort any active program threads
}

void brewStepFill(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      tgtVol[VS_HLT] = 0;
      if (brewStepConfiguration.fillSpargeBeforePreheat)
        tgtVol[VS_HLT] = calcSpargeVol(thread->recipe);
      tgtVol[VS_MASH] = calcStrikeVol(thread->recipe);
      if (getProgMLHeatSrc(thread->recipe) == VS_HLT) {
        tgtVol[VS_HLT] = min(tgtVol[VS_HLT] + tgtVol[VS_MASH], getCapacity(VS_HLT));
        tgtVol[VS_MASH] = 0;
      }
      if (brewStepConfiguration.autoStartFill)
        autoValve[AV_FILL] = 1;
      programThreadSetStep(thread, BREWSTEP_FILL);
      break;
    case STEPSIGNAL_UPDATE:
      if (brewStepConfiguration.autoExitFill && volAvg[VS_HLT] >= tgtVol[VS_HLT] && volAvg[VS_MASH] >= tgtVol[VS_MASH])
        brewStepFill(STEPSIGNAL_ADVANCE, thread);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      tgtVol[VS_HLT] = 0;
      tgtVol[VS_MASH] = 0;
      autoValve[AV_FILL] = 0;
      outputs->setProfileState(OUTPUTPROFILE_FILLHLT, 0);
      outputs->setProfileState(OUTPUTPROFILE_FILLMASH, 0);
      if (signal == STEPSIGNAL_ADVANCE) {
        if (getDelayMins())
          brewStepDelay(STEPSIGNAL_INIT, thread);
        else
          brewStepPreheat(STEPSIGNAL_INIT, thread);
      }
      break;
  }
}

void brewStepDelay(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      //Load delay minutes from EEPROM if timer is not already populated via Power Loss Recovery
      if (getDelayMins() && !timerValue[TIMER_MASH])
        setTimer(TIMER_MASH, getDelayMins());
        programThreadSetStep(thread, BREWSTEP_DELAY);
      break;
    case STEPSIGNAL_UPDATE:
      if (timerValue[TIMER_MASH] == 0)
        brewStepDelay(STEPSIGNAL_ADVANCE, thread);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      clearTimer(TIMER_MASH);
      setAlarm(0);
	  //Clear Delay mins EEPROM setting so it will not be used next no-delay start.
	  if (getDelayMins()) 
		  setDelayMins(0);
      if (signal == STEPSIGNAL_ADVANCE)
        brewStepPreheat(STEPSIGNAL_INIT, thread);
      break;
  }
}

void brewStepPreheat(enum StepSignal signal, struct ProgramThread *thread) {
  static byte preheatVessel;
  switch (signal) {
    case STEPSIGNAL_INIT:
      preheatVessel = getProgMLHeatSrc(thread->recipe);
      
      if (preheatVessel == VS_HLT) {
        setSetpoint(TS_HLT, calcStrikeTemp(thread->recipe));
        setSetpoint(TS_MASH, 0);
      } else {
        setSetpoint(TS_HLT, getProgHLT(thread->recipe));
        setSetpoint(TS_MASH, calcStrikeTemp(thread->recipe));
      }
      
      preheated[VS_HLT] = 0;
      preheated[VS_MASH] = 0;
      //No timer used for preheat
      clearTimer(TIMER_MASH);
      programThreadSetStep(thread, BREWSTEP_PREHEAT);
      break;
    case STEPSIGNAL_UPDATE:
      if (!preheated[preheatVessel] && temp[preheatVessel] >= setpoint[preheatVessel]) {
        preheated[preheatVessel] = 1;
        setAlarm(1);
      }
    
      if (brewStepConfiguration.autoExitPreheat && preheated[preheatVessel])
        brewStepPreheat(STEPSIGNAL_ADVANCE, thread);

      #if defined SMART_HERMS_HLT && defined SMART_HERMS_PREHEAT
        smartHERMSHLT();
      #endif
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      clearTimer(TIMER_MASH);
      setSetpoint(VS_HLT, 0);
      setSetpoint(VS_MASH, 0);
      if (signal == STEPSIGNAL_ADVANCE)
        brewStepGrainIn(STEPSIGNAL_INIT, thread);
      break;
  }
}

void brewStepGrainIn(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      //Disable HLT and Mash heat output during 'Add Grain' to avoid dry running heat elements and burns from HERMS recirc
      grainInStart = 0;
      setSetpoint(VS_HLT, 0);
      setSetpoint(VS_MASH, 0);
      setAlarm(1);
      outputs->setProfileState(OUTPUTPROFILE_ADDGRAIN, 1);
      if(getProgMLHeatSrc(thread->recipe) == VS_HLT) {
        unsigned long spargeVol = calcSpargeVol(thread->recipe);
        unsigned long mashVol = calcStrikeVol(thread->recipe);
        tgtVol[VS_HLT] = (min((spargeVol + mashVol), getCapacity(VS_HLT)) - mashVol);
        tgtVol[VS_MASH] = mashVol;

        if (brewStepConfiguration.autoStrikeTransfer)
           autoValve[AV_SPARGEIN] = 1;
      }
      programThreadSetStep(thread, BREWSTEP_GRAININ);
      break;
    case STEPSIGNAL_UPDATE:
      if (brewStepConfiguration.autoExitGrainInMinutes && !autoValve[AV_SPARGEIN]) {
        if (!grainInStart)
          grainInStart = millis();
        else if ((millis() - grainInStart) / 60000 > brewStepConfiguration.autoExitGrainInMinutes)
          brewStepGrainIn(STEPSIGNAL_ADVANCE, thread);
      }
      //Turn off Sparge In AutoValve if tgtVol has been reached
      //Because this function is called before processautovalves() if we clear the auto valve here the bit in the active profile will still be set until the
      // user exits the grain in step, causing it to not shut off when target volume is reached. 
      if (autoValve[AV_SPARGEIN] && volAvg[VS_HLT] <= tgtVol[VS_HLT]) {
        autoValve[AV_SPARGEIN] = 0;
        outputs->setProfileState(OUTPUTPROFILE_SPARGEIN, 0);
      } else if (volAvg[VS_HLT] <= tgtVol[VS_HLT])
        outputs->setProfileState(OUTPUTPROFILE_SPARGEIN, 0);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      tgtVol[VS_HLT] = 0;
      autoValve[AV_SPARGEIN] = 0;
      outputs->setProfileState(OUTPUTPROFILE_ADDGRAIN, 0);
      outputs->setProfileState(OUTPUTPROFILE_SPARGEIN, 0);
      setSetpoint(VS_HLT, 0);
      setSetpoint(VS_MASH, 0);
      if (signal == STEPSIGNAL_ADVANCE) {
        brewStepRefill(STEPSIGNAL_INIT, thread);
      }
      break;
  }
}

void brewStepRefill(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      //Check if we delayed filling the HLT (default logic) OR if strike was heated in HLT and it wasn't big enough to hold both strike and sparge
      if (!brewStepConfiguration.fillSpargeBeforePreheat || ((getProgMLHeatSrc(thread->recipe) == VS_HLT) && (calcStrikeVol(thread->recipe) + calcSpargeVol(thread->recipe) > getCapacity(VS_HLT)))) {
        tgtVol[VS_HLT] = calcSpargeVol(thread->recipe);
      }

      tgtVol[VS_MASH] = 0;
      if (brewStepConfiguration.autoStartFill)
        autoValve[AV_FILL] = 1;
      programThreadSetStep(thread, BREWSTEP_REFILL);
      break;
    case STEPSIGNAL_UPDATE:
      if (tgtVol[VS_HLT] == 0 || (brewStepConfiguration.autoExitFill && volAvg[VS_HLT] >= tgtVol[VS_HLT] && volAvg[VS_MASH] >= tgtVol[VS_MASH]))
        brewStepRefill(STEPSIGNAL_ADVANCE, thread);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      tgtVol[VS_HLT] = 0;
      tgtVol[VS_MASH] = 0;
      autoValve[AV_FILL] = 0;
      outputs->setProfileState(OUTPUTPROFILE_FILLHLT, 0);
      outputs->setProfileState(OUTPUTPROFILE_FILLMASH, 0);
      if (signal == STEPSIGNAL_ADVANCE)
        brewStepDoughIn(STEPSIGNAL_INIT, thread);
      break;
  }
}

void brewStepMashHelper(byte mashStep, enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      setSetpoint(TS_HLT, getProgHLT(thread->recipe));
      setSetpoint(TS_MASH, getProgMashTemp(thread->recipe, mashStep));
     
      preheated[VS_MASH] = 0;
      //Set timer only if empty (for purposes of power loss recovery)
      if (!timerValue[TIMER_MASH]) setTimer(TIMER_MASH, getProgMashMins(thread->recipe, mashStep)); 
      //Leave timer paused until preheated
      timerStatus[TIMER_MASH] = 0;
      programThreadSetStep(thread, BREWSTEP_DOUGHIN + mashStep);
      break;
    case STEPSIGNAL_UPDATE:
      #ifdef SMART_HERMS_HLT
        smartHERMSHLT();
      #endif
      if (!preheated[VS_MASH] && temp[TS_MASH] >= setpoint[VS_MASH]) {
        preheated[VS_MASH] = 1;
        //Unpause Timer
        if (!timerStatus[TIMER_MASH]) pauseTimer(TIMER_MASH);
      }
      //Exit Condition (and skip unused mash steps)
      if (setpoint[VS_MASH] == 0 || (preheated[VS_MASH] && timerValue[TIMER_MASH] == 0))
        brewStepMashHelper(mashStep, STEPSIGNAL_ADVANCE, thread);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      clearTimer(TIMER_MASH);
      setSetpoint(VS_HLT, 0);
      setSetpoint(VS_MASH, 0);
      if (signal == STEPSIGNAL_ADVANCE) {
        void (*stepFunc)(enum StepSignal, struct ProgramThread *) = brewStepFunc(thread->activeStep + 1);
        if (stepFunc)
          (*stepFunc)(STEPSIGNAL_INIT, thread);
      }
      break;
  }  
}

void brewStepDoughIn(enum StepSignal signal, struct ProgramThread *thread) {
  brewStepMashHelper(MASHSTEP_DOUGHIN, signal, thread);
}

void brewStepAcid(enum StepSignal signal, struct ProgramThread *thread) {
  brewStepMashHelper(MASHSTEP_ACID, signal, thread);
}

void brewStepProtein(enum StepSignal signal, struct ProgramThread *thread) {
  brewStepMashHelper(MASHSTEP_PROTEIN, signal, thread);
}

void brewStepSacch(enum StepSignal signal, struct ProgramThread *thread) {
  brewStepMashHelper(MASHSTEP_SACCH, signal, thread);
}

void brewStepSacch2(enum StepSignal signal, struct ProgramThread *thread) {
  brewStepMashHelper(MASHSTEP_SACCH2, signal, thread);
}

void brewStepMashOut(enum StepSignal signal, struct ProgramThread *thread) {
  brewStepMashHelper(MASHSTEP_MASHOUT, signal, thread);
}

void brewStepMashHold(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      setAlarm(1);
      //Set HLT to Sparge Temp
      setSetpoint(TS_HLT, getProgSparge(thread->recipe));
      //Cycle through steps and use last non-zero step for mash setpoint
      if (!setpoint[TS_MASH]) {
        byte i = MASHSTEP_MASHOUT;
        while (setpoint[TS_MASH] == 0 && i >= MASHSTEP_DOUGHIN && i <= MASHSTEP_MASHOUT)
          setSetpoint(TS_MASH, getProgMashTemp(thread->recipe, i--));
      }
      programThreadSetStep(thread, BREWSTEP_MASHHOLD);
      break;
    case STEPSIGNAL_UPDATE:
      if (brewStepConfiguration.autoExitMash && !zoneIsActive(ZONE_BOIL) && temp[VS_HLT] >= setpoint[VS_HLT])
        brewStepMashHold(STEPSIGNAL_ADVANCE, thread);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      setSetpoint(VS_HLT, 0);
      setSetpoint(VS_MASH, 0);
      if (signal == STEPSIGNAL_ADVANCE)
        brewStepSparge(STEPSIGNAL_INIT, thread);
      break;
  }
}

void brewStepSparge(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      #ifdef SPARGE_IN_PUMP_CONTROL
        prevSpargeVol[1] = volAvg[VS_HLT]; // init the value at the start of sparge
        prevSpargeVol[0] = 0;
      #endif
      tgtVol[VS_KETTLE] = calcPreboilVol(thread->recipe);
      if (brewStepConfiguration.autoStartFlySparge)
        autoValve[AV_FLYSPARGE] = 1;

      programThreadSetStep(thread, BREWSTEP_SPARGE);
      break;
    case STEPSIGNAL_UPDATE:
      if (brewStepConfiguration.autoExitSparge && volAvg[VS_KETTLE] >= tgtVol[VS_KETTLE])
        brewStepSparge(STEPSIGNAL_ADVANCE, thread);
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      tgtVol[VS_HLT] = 0;
      tgtVol[VS_KETTLE] = 0;
      resetSpargeOutputs();
      if (signal == STEPSIGNAL_ADVANCE)
        brewStepBoil(STEPSIGNAL_INIT, thread);
      break;
  }
}

void brewStepBoil(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      setSetpoint(VS_KETTLE, getBoilTemp());
      preheated[VS_KETTLE] = 0;
      boilAdds = getProgAdds(thread->recipe);
      
      //Set timer only if empty (for purposes of power loss recovery)
      if (!timerValue[TIMER_BOIL]) {
        //Clean start of Boil
        setTimer(TIMER_BOIL, getProgBoil(thread->recipe));
        triggered = 0;
        setBoilAddsTrig(triggered);
      } else {
        //Assuming power loss recovery
        triggered = getBoilAddsTrig();
      }
      //Leave timer paused until preheated
      timerStatus[TIMER_BOIL] = 0;
      lastHop = 0;
      boilControlState = CONTROLSTATE_AUTO;
      programThreadSetStep(thread, BREWSTEP_BOIL);
      break;
    case STEPSIGNAL_UPDATE:
      if (!(triggered & 32768) && temp[TS_KETTLE] != BAD_TEMP && temp[TS_KETTLE] >= brewStepConfiguration.preBoilAlarm * 100) {
        setAlarm(1);
        triggered |= 32768; 
        setBoilAddsTrig(triggered);
      }
      if (!preheated[VS_KETTLE] && temp[TS_KETTLE] >= setpoint[VS_KETTLE] && setpoint[VS_KETTLE] > 0) {
        preheated[VS_KETTLE] = 1;
        //Unpause Timer
        if (!timerStatus[TIMER_BOIL])
          pauseTimer(TIMER_BOIL);
      }
      //Turn off hop valve profile after 5s
      if (lastHop > 0 && millis() - lastHop > brewStepConfiguration.boilAdditionSeconds * 1000) {
        outputs->setProfileState(OUTPUTPROFILE_HOPADD, 0);
        lastHop = 0;
      }
      if (preheated[VS_KETTLE]) {
        //Boil Addition
        if ((boilAdds ^ triggered) & 1) {
          outputs->setProfileState(OUTPUTPROFILE_HOPADD, 1);
          lastHop = millis();
          setAlarm(1); 
          triggered |= 1; 
          setBoilAddsTrig(triggered); 
        }
        //Timed additions (See hoptimes[] array in BrewTroller.pde)
        for (byte i = 0; i < 11; i++) {
          if (((boilAdds ^ triggered) & (1<<(i + 1))) && timerValue[TIMER_BOIL] <= hoptimes[i] * 60000) { 
            outputs->setProfileState(OUTPUTPROFILE_HOPADD, 1);
            lastHop = millis();
            setAlarm(1); 
            triggered |= (1<<(i + 1)); 
            setBoilAddsTrig(triggered);
          }
        }
        
        if (brewStepConfiguration.autoBoilWhirlpoolMinutes && timerValue[TIMER_BOIL] <= brewStepConfiguration.autoBoilWhirlpoolMinutes * 60000)
          outputs->setProfileState(OUTPUTPROFILE_WHIRLPOOL, 1);
      }
      //Exit Condition  
      if(preheated[VS_KETTLE] && timerValue[TIMER_BOIL] == 0) {
        //Kill Kettle power at end of timer...
        setSetpoint(VS_KETTLE, 0);
        //...but wait for last hop addition to complete before leaving step
        if(lastHop == 0)
          brewStepBoil(STEPSIGNAL_ADVANCE, thread);
      }
      break;
    case STEPSIGNAL_ABORT:
      programThreadSetStep(thread, INDEX_NONE);
    case STEPSIGNAL_ADVANCE:
      outputs->setProfileState(OUTPUTPROFILE_HOPADD, 0);
      outputs->setProfileState(OUTPUTPROFILE_WHIRLPOOL, 0);
      setSetpoint(VS_KETTLE, 0);
      clearTimer(TIMER_BOIL);
      if (signal == STEPSIGNAL_ADVANCE)
        brewStepChill(STEPSIGNAL_INIT, thread);
      break;
  }
}

void brewStepChill(enum StepSignal signal, struct ProgramThread *thread) {
  switch (signal) {
    case STEPSIGNAL_INIT:
      pitchTemp = getProgPitch(thread->recipe);
      programThreadSetStep(thread, BREWSTEP_CHILL);
      break;
    case STEPSIGNAL_UPDATE:
      break;
    case STEPSIGNAL_ABORT:
    case STEPSIGNAL_ADVANCE:
      programThreadSetStep(thread, INDEX_NONE);
      autoValve[AV_CHILL] = 0;
      outputs->setProfileState(OUTPUTPROFILE_WORTOUT, 0);
      outputs->setProfileState(OUTPUTPROFILE_CHILL, 0);
      break;
  }
}

void resetSpargeOutputs() {
  autoValve[AV_SPARGEIN] = 0;
  autoValve[AV_SPARGEOUT] = 0;
  autoValve[AV_FLYSPARGE] = 0;
  outputs->setProfileState(OUTPUTPROFILE_SPARGEIN, 0);
  outputs->setProfileState(OUTPUTPROFILE_SPARGEOUT, 0);
  outputs->setProfileState(OUTPUTPROFILE_MASHHEAT, 0);
  outputs->setProfileState(OUTPUTPROFILE_MASHIDLE, 0);
}

#ifdef SMART_HERMS_HLT
void smartHERMSHLT() {
  if (!setpoint[VS_MASH]) return;
  setpoint[VS_HLT] = setpoint[VS_MASH] * 2 - temp[TS_MASH];
  //Constrain HLT Setpoint to Mash Setpoint + MASH_HEAT_LOSS (minimum) and HLT_MAX_TEMP (Maximum)
  setpoint[VS_HLT] = constrain(setpoint[VS_HLT], setpoint[VS_MASH] + MASH_HEAT_LOSS * 100, HLT_MAX_TEMP * 100);
}
#endif
  
unsigned long calcStrikeVol(byte recipe) {
  unsigned int mashRatio = getProgRatio(recipe);
  if (!mashRatio)
    return calcTotalLiquorVol(recipe);
    
  unsigned long retValue = round(getProgGrain(recipe) * mashRatio / 100.0);

  //Convert qts to gal for US
  #ifndef USEMETRIC
    retValue = round(retValue / 4.0);
  #endif
  
  //Add extra strike volume needed for loss during strike transfer
  retValue += getStrikeLoss();

  return retValue;
}

unsigned long calcTotalLiquorVol(byte recipe) {
  unsigned long retValue = calcPreboilVol(recipe);

  //Add Water Lost in Spent Grain
  retValue += calcGrainLoss(recipe);
  
  //Add extra sparge volume needed for loss during transfer of sprage liquor
  retValue += getSpargeLoss();
  
  //Add extra strike volume needed for loss during strike transfer
  retValue += getStrikeLoss();
  
  //Add extra volume needed for loss during transfer from mash to boil
  retValue += getMashLoss();
}

unsigned long calcSpargeVol(byte recipe) {
  unsigned long retValue = calcTotalLiquorVol(recipe);
  
  //Subtract Strike Water Volume
  retValue -= calcStrikeVol(recipe);
  
  retValue = max(retValue, getMinimumSpargeVolume() * 100ul);
  return retValue;
}

unsigned long calcPreboilVol(byte recipe) {
  unsigned long retValue = getProgBatchVol(recipe);
  
  //Add loss in boil kettle and plumbing
  //Note: Will affect mash efficiency and hop utilization
  //Alternatively set to 0, adjust batch volume and scale recipe
  retValue += getBoilLoss(); 
  
  //Shrinkage should nto be calculated as filling volume at ground water temperature
  // will first expand at boil and then shrink at pitch temp resulting in no change.
  #ifdef VOL_SHRINKAGE
    retValue /= VOL_SHRINKAGE;
  #endif
  
  //Add evaporative losses based on boil time and system evaporation rate
  retValue += (unsigned long)getEvapRate() * EvapRateConversion * getProgBoil(recipe) / 60;
  return retValue;
}

unsigned long calcGrainLoss(byte recipe) {
  unsigned long retValue;
  retValue = round(getProgGrain(recipe) * (getGrainLiquorLoss() / 10000.0));
  return retValue;
}

unsigned long calcGrainVolume(byte recipe) {
  return round (getProgGrain(recipe) * (getGrainDisplacement() / 1000.0));
}

/**
 * Calculates the strike temperature for the mash.
 */
byte calcStrikeTemp(byte recipe) {
  //Metric temps are stored as quantity of 0.5C increments
  float strikeTemp = (float)getFirstStepTemp(recipe) / SETPOINT_DIV;
  float grainTemp = (float)getGrainTemp() / SETPOINT_DIV;
  float grainWeight = getProgGrain(recipe) / 1000.0;
  float strikeVol = calcStrikeVol(recipe) / 1000.0;
  float mashThermoDynamic = 0.0;
  
  //If we are not heating strike directly in the mash we should account for the mash tun heat capacity
  if (getProgMLHeatSrc(recipe) != VS_MASH)
    mashThermoDynamic = getMashTunHeatCapacity() / 1000.0;
  
  #ifdef USEMETRIC
    const float kGrainThermoDynamic = 0.41;
  #else
    const float kGrainThermoDynamic = 0.05;
  #endif
  
  float totalSpecificHeat = strikeTemp * (kGrainThermoDynamic * grainWeight + strikeVol + mashThermoDynamic);
  float grainSpecificHeat = kGrainThermoDynamic * grainWeight * grainTemp;
  float mashTunSpecificHeat = mashThermoDynamic * (temp[VS_MASH] / 100.00);
  strikeTemp = (totalSpecificHeat - grainSpecificHeat - mashTunSpecificHeat) / strikeVol;
 
  //Return value in EEPROM format which is 0-255F or 0-255 x 0.5C
  return strikeTemp * SETPOINT_DIV;
}

byte getFirstStepTemp(byte recipe) {
  byte firstStep = 0;
  byte i = MASHSTEP_DOUGHIN;
  while (firstStep == 0 && i <= MASHSTEP_MASHOUT) firstStep = getProgMashTemp(recipe, i++);
  return firstStep;
}
