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

void loadSetup() {
  //**********************************************************************************
  //TSensors: HLT (0-7), MASH (8-15), KETTLE (16-23), H2OIN (24-31), H2OOUT (32-39),
  //          BEEROUT (40-47), AUX1 (48-55), AUX2 (56-63), AUX3 (64-71)
  //**********************************************************************************
  EEPROMreadBytes(0, *tSensor, 72);
 
  //**********************************************************************************
  //PIDp HLT (73), Mash (78), Kettle (83), Reserved (88)
  //PIDi HLT (74), Mash (79), Kettle (84), Reserved (89)
  //PIDd HLT (75), Mash (80), Kettle (85), Reserved (90)
  //PIDCycle HLT (76), Mash (81), Kettle (86), Reserved (91)
  //Hysteresis HLT (77), Mash (82), Kettle (87), Reserved (92)
  //PWM Pin (309-312)
  //**********************************************************************************
  for (byte i = VS_HLT; i <= VS_KETTLE; i++)
    hysteresis[i] = EEPROM.read(77 + i * 5);

  //**********************************************************************************
  //boilPwr (112)
  //**********************************************************************************
  boilPwr = EEPROM.read(112);
  
  for (byte i = 0; i <= VS_KETTLE; i++)
    vSensor[i] = EEPROM.read(114 + i);
  
  //**********************************************************************************
  //OPEN (118)
  //**********************************************************************************

  //**********************************************************************************
  //calibVols HLT (119-158), Mash (159-198), Kettle (199-238)
  //calibVals HLT (239-258), Mash (259-278), Kettle (279-298)
  //**********************************************************************************
  eeprom_read_block(&calibVols, (unsigned char *) 119, 120);
  eeprom_read_block(&calibVals, (unsigned char *) 239, 60);

  //**********************************************************************************
  //setpoints (299-301)
  //**********************************************************************************
  for (byte i=VS_HLT; i<=VS_KETTLE; i++) { 
    setpoint[i] = EEPROM.read(299 + i) * SETPOINT_MULT;
    eventHandler(EVENT_SETPOINT, i);
  }
  
  //**********************************************************************************
  //timers (302-305)
  //**********************************************************************************
  for (byte i=TIMER_MASH; i<=TIMER_BOIL; i++) { timerValue[i] = EEPROMreadInt(302 + i * 2) * 60000; }
  
  loadOutputSystem();

  for (byte i = VS_HLT; i <= VS_KETTLE; i++)
    loadPWMOutput(i);

  //**********************************************************************************
  //Timer/Alarm Status (306)
  //**********************************************************************************
  byte options = EEPROM.read(306);
  for (byte i = TIMER_MASH; i <= TIMER_BOIL; i++) {
    timerStatus[i] = bitRead(options, i);
    lastTime[i] = millis();
  }
  alarmStatus = bitRead(options, 2);
  outputs->setProfileState(OUTPUTPROFILE_ALARM, alarmStatus);
    
  #ifdef ESTOP_PIN
    loadEStop();
  #endif
  
  for (byte i = 0; i < USERTRIGGER_COUNT; i++)
    loadTriggerInstance(i);
    
  #ifdef RGBIO8_ENABLE
    loadRGBIO8();
  #endif

  loadBubbler();
  eepromLoadBrewStepConfiguration();

  for(byte i = 0; i < PROGRAMTHREAD_MAX; i++)
    eepromLoadProgramThread(i, &programThread[i]);  
}

void loadPWMOutput(byte i) {
  byte pwmPin = getPWMPin(i);
  byte pwmCycle = getPWMPeriod(i);
  byte pwmResolution = getPWMResolution(i);
  byte pidLimit = getPIDLimit(i);
  if (pwmOutput[i])
    delete pwmOutput[i];
  if (pwmPin != INDEX_NONE)
    pwmOutput[i] = new analogOutput_SWPWM(pwmPin, pwmCycle, pwmResolution);
    
  pid[i].SetInputLimits(0, 25500);
  pid[i].SetOutputLimits(0, (unsigned long)pwmResolution * pidLimit / 100);
  pid[i].SetTunings(getPIDp(i), getPIDi(i), getPIDd(i));
  pid[i].SetMode(AUTO);
  pid[i].SetSampleTime(PID_UPDATE_INTERVAL);
}

void loadOutputSystem() {
  //Refresh output object
  if (outputs)
    delete outputs;
  outputs = new OutputSystem();
  outputs->init();
  loadOutputProfiles();
  #ifdef OUTPUTBANK_MODBUS
    loadOutModbus();
  #endif
  analogOutput_SWPWM::setup(outputs);
}

void loadOutputProfiles() {
  for (byte i = 0; i < OUTPUTPROFILE_USERCOUNT; i++)
    outputs->setProfileMask(i, getOutputProfile(i));
}

#ifdef OUTPUTBANK_MODBUS
  void loadOutModbus() {
    for (byte i = 0; i < OUTPUTBANK_MODBUS_MAXBOARDS; i++) {
      byte addr = getOutModbusAddr(i);
      if (addr != OUTPUTBANK_MODBUS_ADDRNONE)
        outputs->newModbusBank(addr, getOutModbusReg(i), getOutModbusCoilCount(i));
    }
  }
#endif

void loadTriggerInstance(byte i) {
  if (trigger[i])
    delete trigger[i];
  trigger[i] = NULL;
#ifdef DIGITAL_INPUTS
  byte triggerPinMap[] = DIGITAL_INPUTS_PINS;
#endif

  struct TriggerConfiguration trigConfig;
  loadTriggerConfiguration(i, &trigConfig);
  
  if (trigConfig.type == TRIGGERTYPE_VOLUME)
    trigger[i] = new TriggerValue(&volAvg[trigConfig.index], trigConfig.threshold, trigConfig.activeLow, trigConfig.profileFilter, trigConfig.disableMask, trigConfig.releaseHysteresis);
#ifdef DIGITAL_INPUTS  
  else if (trigConfig.type == TRIGGERTYPE_GPIO)
    trigger[i] = new TriggerGPIO(triggerPinMap[trigConfig.index], trigConfig.activeLow, trigConfig.profileFilter, trigConfig.disableMask, trigConfig.releaseHysteresis);
#endif
  else if (trigConfig.type == TRIGGERTYPE_SETPOINTDELAY)
    trigger[i] = new TriggerSetpointDelay(&setpoint[trigConfig.index], trigConfig.activeLow, trigConfig.profileFilter, trigConfig.disableMask, trigConfig.releaseHysteresis);
}

#ifdef ESTOP_PIN
  void loadEStop() {
    outputs->setOutputEnableMask(OUTPUTENABLE_ESTOP, 0xFFFFFFFFul); //Enable all pins in estop enable mask
    if (estopPin)
      delete estopPin;
    estopPin = NULL;
    if (getEStopEnabled()) {
      estopPin = new pin;
      estopPin->setup(ESTOP_PIN, INPUT);
    }
  }
#endif

#ifdef RGBIO8_ENABLE
  void loadRGBIO8() {
    for(byte i = 0; i < RGBIO8_MAX_OUTPUT_RECIPES; i++) {
      unsigned int recipe[4];
      getRGBIORecipe(i, recipe);
      RGBIO8::setOutputRecipe(i, recipe[0], recipe[1], recipe[2], recipe[3]);
    }
    
    for (byte i = 0; i < RGBIO8_MAX_BOARDS; i++) {
      if (rgbio[i])
        delete rgbio[i];
      byte addr = getRGBIOAddr(i);
      if (addr != RGBIO8_UNASSIGNED) {
        rgbio[i] = new RGBIO8(addr);
        for (int j = 0; j < 8; j++) {
          byte assignment = getRGBIOAssignment(i, j);
          if (assignment != RGBIO8_UNASSIGNED) {
            byte recipe = getRGBIOAssignmentRecipe(i, j);
            rgbio[i]->assign(j, assignment, recipe);
          }
        }
      }
    }
    RGBIO8::setup(outputs);
  }
#endif

void loadBubbler() {  
  if (bubbler)
    delete bubbler;
  bubbler = NULL;
  byte bubbleOut = getBubblerOutput();
  if (bubbleOut != INDEX_NONE) {
    bubbler = new Bubbler(outputs, bubbleOut, getBubblerInterval(), getBubblerDuration(), getBubblerDelay());
  }
}

//*****************************************************************************************************************************
// Individual EEPROM Get/Set Variable Functions
//*****************************************************************************************************************************

//**********************************************************************************
//TSensors: HLT (0-7), MASH (8-15), KETTLE (16-23), H2OIN (24-31), H2OOUT (32-39), 
//          BEEROUT (40-47), AUX1 (48-55), AUX2 (56-63), AUX3 (64-71)
//**********************************************************************************
void setTSAddr(byte sensor, byte addr[8]) {
  memcpy(tSensor[sensor], addr, 8);
  EEPROMwriteBytes(sensor * 8, addr, 8);
}

//**********************************************************************************
//OPEN (72)
//**********************************************************************************


//**********************************************************************************
//PIDp HLT (73), Mash (78), Kettle (83), RESERVED (88)
//**********************************************************************************
void setPIDp(byte vessel, byte value) {
  pid[vessel].SetTunings(value, pid[vessel].GetI_Param(), pid[vessel].GetD_Param());
  EEPROM.write(73 + vessel * 5, value);
}
byte getPIDp(byte vessel) { return EEPROM.read(73 + vessel * 5); }

//**********************************************************************************
//PIDi HLT (74), Mash (79), Kettle (84), RESERVED (89)
//**********************************************************************************
void setPIDi(byte vessel, byte value) {
  pid[vessel].SetTunings(pid[vessel].GetP_Param(), value, pid[vessel].GetD_Param());
  EEPROM.write(74 + vessel * 5, value);
}
byte getPIDi(byte vessel) { return EEPROM.read(74 + vessel * 5); }

//**********************************************************************************
//PIDd HLT (75), Mash (80), Kettle (85), RESERVED (90)
//**********************************************************************************
void setPIDd(byte vessel, byte value) {
  pid[vessel].SetTunings(pid[vessel].GetP_Param(), pid[vessel].GetI_Param(), value);
  EEPROM.write(75 + vessel * 5, value);
}
byte getPIDd(byte vessel) { return EEPROM.read(75 + vessel * 5); }

//**********************************************************************************
//PWM Period HLT (76), Mash (81), Kettle (86), RESERVED (91)
//**********************************************************************************
byte getPWMPeriod(byte vessel) { return EEPROM.read(76 + vessel * 5); }
void setPWMPeriod(byte vessel, byte value) {
  EEPROM.write(76 + vessel * 5, value);
}

//**********************************************************************************
//Hysteresis HLT (77), Mash (82), Kettle (87), RESERVED (92)
//**********************************************************************************
void setHysteresis(byte vessel, byte value) {
  hysteresis[vessel] = value;
  EEPROM.write(77 + vessel * 5, value);
}

//**********************************************************************************
//Capacity HLT (93-96), Mash (97-100), Kettle (101-104)
//**********************************************************************************
void setCapacity(byte vessel, unsigned long value) {
  EEPROMwriteLong(93 + vessel * 4, value);
}
unsigned long getCapacity(byte vessel) { return EEPROMreadLong(93 + vessel * 4); }

//**********************************************************************************
// Mash Tun heat Capacity (105-106)
//**********************************************************************************
void setMashTunHeatCapacity(unsigned int value) {
  EEPROMwriteInt(105, value);
}
unsigned int getMashTunHeatCapacity() {
  return EEPROMreadInt(105);
}

//**********************************************************************************
// Minimum Sparge Volume (107-108)
//**********************************************************************************
void setMinimumSpargeVolume(unsigned int value) {
  EEPROMwriteInt(107, value);
}
unsigned int getMinimumSpargeVolume() {
  return EEPROMreadInt(107);
}


//**********************************************************************************
// OPEN (109-111)
//**********************************************************************************


//**********************************************************************************
//Boil Temp (111)
//**********************************************************************************
byte getBoilTemp() { return EEPROM.read(111); }
void setBoilTemp(byte boilTemp) { EEPROM.write(111, boilTemp); }

//**********************************************************************************
//Boil Power (112)
//**********************************************************************************
void setBoilPwr(byte value) { 
  boilPwr = value;
  EEPROM.write(112, value); 
}

//**********************************************************************************
//evapRate (113)
//**********************************************************************************
void setEvapRate(byte value) {
  EEPROM.write(113, value);
}
byte getEvapRate() { return EEPROM.read(113); }

//**********************************************************************************
//Volume Sensors (114-117) HLT/Mash/Kettle/Reserved
//**********************************************************************************
void setVolumeSensor(byte vessel, byte index) {
  vSensor[vessel] = index;
  EEPROM.write(114 + vessel, index);
}
byte getVolumeSensor(byte vessel) {
  return EEPROM.read(114 + vessel);
}
//**********************************************************************************
//Open (118)
//**********************************************************************************

//**********************************************************************************
//calibVols HLT (119-158), Mash (159-198), Kettle (199-238)
//calibVals HLT (239-258), Mash (259-278), Kettle (279-298)
//**********************************************************************************
void setVolCalib(byte vessel, byte slot, unsigned int value, unsigned long vol) {
  calibVols[vessel][slot] = vol;
  calibVals[vessel][slot] = value;
  EEPROMwriteLong(119 + vessel * 40 + slot * 4, vol);
  EEPROMwriteInt(239 + vessel * 20 + slot * 2, value);
}

//*****************************************************************************************************************************
// Power Loss Recovery Functions
//*****************************************************************************************************************************

//**********************************************************************************
//setpoints (299-301)
//**********************************************************************************
void setSetpoint(byte vessel, int value) {
  setpoint[vessel] = value * SETPOINT_MULT;
  EEPROM.write(299 + vessel, value);
  eventHandler(EVENT_SETPOINT, vessel);
}

//**********************************************************************************
//timers (302-305)
//**********************************************************************************
void setTimerRecovery(byte timer, unsigned int newMins) { 
  if(newMins != -1) EEPROMwriteInt(302 + timer * 2, newMins); 
}

//**********************************************************************************
//Timer/Alarm Status (306)
//**********************************************************************************
void setTimerStatus(byte timer, boolean value) {
  timerStatus[timer] = value;
  byte options = EEPROM.read(306);
  bitWrite(options, timer, value);
  EEPROM.write(306, options);
}

void setAlarmStatus(boolean value) {
  alarmStatus = value;
  byte options = EEPROM.read(306);
  bitWrite(options, 2, value);
  EEPROM.write(306, options);
}



//**********************************************************************************
//Triggered Boil Addition Alarms (307-308)
//**********************************************************************************
unsigned int getBoilAddsTrig() { return EEPROMreadInt(307); }
void setBoilAddsTrig(unsigned int adds) { EEPROMwriteInt(307, adds); }

//**********************************************************************************
// PWM Outputs (309-312)
//**********************************************************************************
unsigned int getPWMPin(byte vessel) { return EEPROM.read(309 + vessel); }
void setPWMPin(byte vessel, byte pin) { EEPROM.write(309 + vessel, pin); }

//**********************************************************************************
//Program Threads (313-316)
//**********************************************************************************

void eepromLoadProgramThread(byte index, struct ProgramThread *thread) {
  eeprom_read_block((void *) thread, (unsigned char *) 313 + index * sizeof(struct ProgramThread), sizeof(struct ProgramThread));
  if (thread->activeStep != INDEX_NONE) {
    programThreadSignal(programThread + index, STEPSIGNAL_INIT);
    eventHandler(EVENT_STEPINIT, thread->activeStep);  
  }
}

void eepromSaveProgramThread(byte index, struct ProgramThread *thread) {
  eeprom_write_block((void *) thread, (unsigned char *) 313 + index * sizeof(struct ProgramThread), sizeof(struct ProgramThread));
}

//**********************************************************************************
// ***OPEN*** (317-393)
//**********************************************************************************

//**********************************************************************************
// PWM Resolution (390-393) 0-255 for HLT/Mash/Kettle + 1 Reserved
//**********************************************************************************

void setPWMResolution(byte vessel, byte limit) {
  EEPROM.write(390 + vessel, limit);
}

byte getPWMResolution(byte vessel) {
  return EEPROM.read(390 + vessel);
}

//**********************************************************************************
// PID Limit (394-397) 0-100% for HLT/Mash/Kettle + 1 Reserved
//**********************************************************************************

void setPIDLimit(byte vessel, byte limit) {
  EEPROM.write(394 + vessel, limit);
}

byte getPIDLimit(byte vessel) {
  return EEPROM.read(394 + vessel);
}


//**********************************************************************************
//Delay Start (Mins) (398-399)
//**********************************************************************************
unsigned int getDelayMins() { return EEPROMreadInt(398); }
void setDelayMins(unsigned int mins) {
  if(mins != 0) EEPROMwriteInt(398, mins);
void setDelayMins(unsigned int mins) {
  EEPROMwriteInt(398, mins);
void setDelayMins(unsigned int mins) { 
  EEPROMwriteInt(398, mins); 
}

//**********************************************************************************
//Grain Temp (400)
//**********************************************************************************
void setGrainTemp(byte grainTemp) { EEPROM.write(400, grainTemp); }
byte getGrainTemp() { return EEPROM.read(400); }

//*****************************************************************************************************************************
// Output Profile Configuration (401-480; 481-785 Reserved)
//*****************************************************************************************************************************
void setOutputProfile(byte profile, unsigned long value) {
  outputs->setProfileMask(profile, value);
  EEPROMwriteLong(401 + profile * 4, value);
}
unsigned long getOutputProfile(byte profile) {
  return EEPROMreadLong(401 + profile * 4);
}

//*****************************************************************************************************************************
// Program Load/Save Functions (786-1985) - 20 Program Slots Total
//*****************************************************************************************************************************
#define PROGRAM_SIZE 60
#define PROGRAM_START_ADDR 786

//**********************************************************************************
//Program Name (P:1-19)
//**********************************************************************************
void setProgName(byte preset, char *name) {
  for (byte i = 0; i < 19; i++) EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + i, name[i]);
}

void getProgName(byte preset, char *name) {
  for (byte i = 0; i < 19; i++) name[i] = EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + i);
  name[19] = '\0';
}

//**********************************************************************************
//OPEN (P:20)
//**********************************************************************************

//**********************************************************************************
//Sparge Temp (P:21)
//**********************************************************************************
void setProgSparge(byte preset, byte sparge) { EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 21, sparge); }
byte getProgSparge(byte preset) { return EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 21); }

//**********************************************************************************
//Boil Mins (P:22-23)
//**********************************************************************************
void setProgBoil(byte preset, int boilMins) { 
  if (boilMins != -1) EEPROMwriteInt(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 22, boilMins); 
}
unsigned int getProgBoil(byte preset) { return EEPROMreadInt(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 22); }

//**********************************************************************************
//Mash Ratio (P:24-25)
//**********************************************************************************
void setProgRatio(byte preset, unsigned int ratio) { EEPROMwriteInt(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 24, ratio); }
unsigned int getProgRatio(byte preset) { return EEPROMreadInt(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 24); }

//**********************************************************************************
//Mash Temps (P:26-31)
//**********************************************************************************
void setProgMashTemp(byte preset, byte mashStep, byte mashTemp) { EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 26 + mashStep, mashTemp); }
byte getProgMashTemp(byte preset, byte mashStep) { return EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 26 + mashStep); }

//**********************************************************************************
//Mash Times (P:32-37)
//**********************************************************************************
void setProgMashMins(byte preset, byte mashStep, byte mashMins) { 
  //This one is very tricky. Since it is better to avoid memory allocation changes. Here is the trick. 
  //setProgMashMins is not supposed to received a value larger than 119 unless someone change it. But it can receive -1 
  //when the user CANCEL its action of editing the mashing time value. -1 is converted as 255 (in a byte format). That is why
  //the condition is set on 255 instead of -1. 
  if (mashMins != 255) EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 32 + mashStep, mashMins); 
}
byte getProgMashMins(byte preset, byte mashStep) { return EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 32 + mashStep); }

//**********************************************************************************
//Batch Vol (P:38-41)
//**********************************************************************************
unsigned long getProgBatchVol(byte preset) { return EEPROMreadLong(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 38); }
void setProgBatchVol (byte preset, unsigned long vol) { EEPROMwriteLong(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 38, vol); }

//**********************************************************************************
//Mash Liquor Heat Source (P:42)
//**********************************************************************************
void setProgMLHeatSrc(byte preset, byte vessel) { EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 42, vessel); }
byte getProgMLHeatSrc(byte preset) { return EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 42); }

//**********************************************************************************
//HLT Temp (P:43)
//**********************************************************************************
void setProgHLT(byte preset, byte HLT) { EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 43, HLT); }
byte getProgHLT(byte preset) { return EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 43); }

//**********************************************************************************
//Pitch Temp (P:44)
//**********************************************************************************
void setProgPitch(byte preset, byte pitch) { EEPROM.write(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 44, pitch); }
byte getProgPitch(byte preset) { return EEPROM.read(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 44); }

//**********************************************************************************
//Boil Addition Alarms (P:45-46)
//**********************************************************************************
void setProgAdds(byte preset, unsigned int adds) { EEPROMwriteInt(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 45, adds); }
unsigned int getProgAdds(byte preset) { return EEPROMreadInt(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 45); }

//**********************************************************************************
//Grain Weight (P:47-50)
//**********************************************************************************
void setProgGrain(byte preset, unsigned long grain) { EEPROMwriteLong(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 47, grain); }
unsigned long getProgGrain(byte preset) { return EEPROMreadLong(PROGRAM_START_ADDR + preset * PROGRAM_SIZE + 47); }

//**********************************************************************************
//OPEN (P:51-59)
//**********************************************************************************

//**********************************************************************************
//BrewTroller Fingerprint (2046)
//**********************************************************************************

//**********************************************************************************
//EEPROM Version (2047)
//**********************************************************************************

//**********************************************************************************
//LCD Bright/Contrast (2048-2049) LCD4Bit
//**********************************************************************************

//**********************************************************************************
//EStop Enabled (2050)
//**********************************************************************************
boolean getEStopEnabled() {
  return EEPROM.read(2050);
}

void setEStopEnabled(boolean enabled) {
  EEPROM.write(2050, enabled);
}

//**********************************************************************************
//OPEN (2051-2064)
//**********************************************************************************

//**********************************************************************************
//Modbus Relay Boards (2065-2076) + Reserved: 2077-2088 
//**********************************************************************************
byte getOutModbusAddr(byte board) {
  return EEPROM.read(2065 + board * 4);
}

unsigned int getOutModbusReg(byte board) {
  return EEPROMreadInt(2065 + board * 4 + 1);
}

byte getOutModbusCoilCount(byte board) {
  return EEPROM.read(2065 + board * 4 + 3);
}

void setOutModbusAddr(byte board, byte addr) {
  EEPROM.write(2065 + board * 4, addr);
}

void setOutModbusReg(byte board, unsigned int reg) {
  EEPROMwriteInt(2065 + board * 4 + 1, reg);
}

void setOutModbusCoilCount(byte board, byte count) {
  EEPROM.write(2065 + board * 4 + 3, count);
}

void setOutModbusDefaults(byte board) {
  setOutModbusAddr(board, OUTPUTBANK_MODBUS_ADDRNONE);
  setOutModbusReg(board, OUTPUTBANK_MODBUS_DEFCOILREG);
  setOutModbusCoilCount(board, OUTPUTBANK_MODBUS_DEFCOILCOUNT);
}

//RGBIO Recipes (2089 - 2120): 8 bytes per recipe x 4 recipes
void getRGBIORecipe(byte recipeIndex, unsigned int* recipe) {
  eeprom_read_block((void *)recipe, (unsigned char *) (2089 + recipeIndex * 8), 8);
}

void setRGBIORecipe(byte recipeIndex, unsigned int* recipe) {
  eeprom_write_block((void *) recipe, (unsigned char *) (2089 + recipeIndex * 8), 8);
}

//RGBIO Board Addresses (2121 - 2124): 1 bytes per board x 4 boards
byte getRGBIOAddr(byte boardIndex) {
  return EEPROM.read(2121 + boardIndex);
}

void setRGBIOAddr(byte boardIndex, byte addr) {
  EEPROM.write(2121 + boardIndex, addr);
}

//RGBIO Assignments (2125 - 2156): 8 bytes per board x 4 boards
byte getRGBIOAssignment(byte boardIndex, byte channelIndex) {
  byte assignment = EEPROM.read(2125 + boardIndex * 8 + channelIndex);
  if (assignment == RGBIO8_UNASSIGNED)
    return assignment;
  return assignment & B00011111;
}

void setRGBIOAssignment(byte boardIndex, byte channelIndex, byte outputIndex, byte recipeIndex) {
  byte assignment = outputIndex;
  if (assignment != RGBIO8_UNASSIGNED) {
      assignment &= B00011111;
      assignment |= (recipeIndex << 5) & B01100000;
  }
  EEPROM.write(2125 + boardIndex * 8 + channelIndex, assignment);
}

byte getRGBIOAssignmentRecipe(byte boardIndex, byte channelIndex) {
  byte assignment = EEPROM.read(2125 + boardIndex * 8 + channelIndex);
  if (assignment == RGBIO8_UNASSIGNED)
    return 0;
  return (assignment & B01100000) >> 5;
}

//**********************************************************************************
//Trigger Configuration - 13-Bytes * 10 Triggers (2157-2286)
//**********************************************************************************
struct TriggerConfiguration* loadTriggerConfiguration(byte triggerIndex, struct TriggerConfiguration *configuration) {
  eeprom_read_block((void *) configuration, (unsigned char *) 2157 + triggerIndex * sizeof(struct TriggerConfiguration), sizeof(struct TriggerConfiguration));
  return configuration;
}

void saveTriggerConfiguration(byte triggerIndex, struct TriggerConfiguration *configuration) {
  eeprom_write_block((void *) configuration, (unsigned char *) 2157 + triggerIndex * sizeof(struct TriggerConfiguration), sizeof(struct TriggerConfiguration));
}

//**********************************************************************************
//Intermittent Bubbler Configuration -  (2187-2190)
//**********************************************************************************
byte getBubblerOutput() {
  return EEPROM.read(2187);
}

void setBubblerOutput(byte outputIndex) {
  return EEPROM.write(2187, outputIndex);
}

byte getBubblerInterval() {
  return EEPROM.read(2188);
}

void setBubblerInterval(byte interval) {
  return EEPROM.write(2188, interval);
}

byte getBubblerDuration() {
  return EEPROM.read(2189);
}

void setBubblerDuration(byte duration) {
  return EEPROM.write(2189, duration);
}

byte getBubblerDelay() {
  return EEPROM.read(2190);
}

void setBubblerDelay(byte delayTime) {
  return EEPROM.write(2190, delayTime);
}

//*****************************************************************************************************************************
// Calculation Factors (2191-2202) + Reserved (2203-2204)
//*****************************************************************************************************************************
unsigned int getGrainDisplacement() {
  return EEPROMreadInt(2191);
}

void setGrainDisplacement(unsigned int displacement) {
  EEPROMwriteInt(2191, displacement);
}

unsigned int getGrainLiquorLoss() {
  return EEPROMreadInt(2193);
}

void setGrainLiquorLoss(unsigned int loss) {
  EEPROMwriteInt(2193, loss);
}

unsigned int getStrikeLoss() {
  return EEPROMreadInt(2195);
}

void setStrikeLoss(unsigned int loss) {
  EEPROMwriteInt(2195, loss);
}

unsigned int getSpargeLoss() {
  return EEPROMreadInt(2197);
}

void setSpargeLoss(unsigned int loss) {
  EEPROMwriteInt(2197, loss);
}

unsigned int getMashLoss() {
  return EEPROMreadInt(2199);
}

void setMashLoss(unsigned int loss) {
  EEPROMwriteInt(2199, loss);
}

unsigned int getBoilLoss() {
  return EEPROMreadInt(2201);
}

void setBoilLoss(unsigned int loss) {
  EEPROMwriteInt(2201, loss);
}

//*****************************************************************************************************************************
// Brew Step Configuration (2205 - 2214) + Reserved (2215-2224)
//*****************************************************************************************************************************
void eepromLoadBrewStepConfiguration() {
  void *configuration = &brewStepConfiguration;
  eeprom_read_block(configuration, (unsigned char *) 2205, sizeof(struct BrewStepConfiguration));
}

void eepromSaveBrewStepConfiguration() {
  void *configuration = &brewStepConfiguration;
  eeprom_write_block(configuration, (unsigned char *) 2205, sizeof(struct BrewStepConfiguration));
}

void initializeBrewStepConfiguration() {
  brewStepConfiguration.fillSpargeBeforePreheat = 0;
  brewStepConfiguration.autoStartFill = 0;
  brewStepConfiguration.autoExitFill = 0;
  brewStepConfiguration.autoExitPreheat = 0;
  brewStepConfiguration.autoStrikeTransfer = 0;
  brewStepConfiguration.autoExitGrainInMinutes = 0;
  brewStepConfiguration.autoExitMash = 0;
  brewStepConfiguration.autoStartFlySparge = 0;
  brewStepConfiguration.autoExitSparge = 0;
  brewStepConfiguration.autoBoilWhirlpoolMinutes = 0;
  brewStepConfiguration.boilAdditionSeconds = 0;
  #ifdef USEMETRIC
    brewStepConfiguration.preBoilAlarm = 96;
  #else
    brewStepConfiguration.preBoilAlarm = 205;
  #endif
  brewStepConfiguration.flySpargeHysteresis = 0;
}

//*****************************************************************************************************************************
// Check/Update/Format EEPROM
//*****************************************************************************************************************************
boolean checkConfig() {
  byte cfgVersion = EEPROM.read(2047);
  byte BTFinger = EEPROM.read(2046);

  //If the BT 1.3 fingerprint is missing force a init of EEPROM
  //FermTroller will bump to a cfgVersion starting at 7
  if (BTFinger != 252 || cfgVersion == 255) {
    //Force default LCD Bright/Contrast to allow user to see 'Missing Config' EEPROM prompt
    #if (defined __AVR_ATmega1284P__ || defined __AVR_ATmega1284__) && defined UI_DISPLAY_SETUP && defined UI_LCD_4BIT
      EEPROM.write(2048, LCD_DEFAULT_BRIGHTNESS);
      EEPROM.write(2049, LCD_DEFAULT_CONTRAST);
    #endif
    return 1;
  }

  //In the future, incremental EEPROM settings will be included here
  switch(cfgVersion) {
    case 0:
      //Supported PID cycle is changing from 1-255 to .1-25.5
      //All current PID cycle settings will be multiplied by 10 to represent tenths (s)
      for (byte vessel = VS_HLT; vessel <= VS_KETTLE; vessel++)
        EEPROM.write(76 + vessel * 5, EEPROM.read(76 + vessel * 5) * 10);
      //Set cfgVersion = 1
    case 1:
    case 2:
    case 3:
      //Reset profiles as bits have shifted
      for (byte i = 0; i < OUTPUTPROFILE_USERCOUNT; i++)
        setOutputProfile(i, 0);
      //MODBUS Outputs Defaults
      for (byte i = 0; i < OUTPUTBANK_MODBUS_MAXBOARDS; i++)
        setOutModbusDefaults(i);
      //RGBIO Defaults
      for (byte i = 0; i < RGBIO8_MAX_BOARDS; i++) {
        setRGBIOAddr(i, RGBIO8_UNASSIGNED);
        for (byte j = 0; j < 8; j++)
          setRGBIOAssignment(i, j, RGBIO8_UNASSIGNED, 0);
      }
      
      // Recipe 0, used for Heat Outputs
      // Off:       0xF00 (Red)
      // Auto Off:  0xFFF (White)
      // Auto On:   0xF40 (Orange)
      // On:        0x0F0 (Green)
      {
        unsigned int recipe[4] = {0xF00, 0xFFF, 0xF40, 0x0F0};
        setRGBIORecipe(0, recipe);
      }

      // Recipe 1, used for Pump/Valve Outputs
      // Off:       0xF00 (Red)
      // Auto Off:  0xFFF (White)
      // Auto On:   0x00F (Blue)
      // On:        0x0F0 (Green)
      {
        unsigned int recipe[4] = {0xF00, 0xFFF, 0x00F, 0x0F0};
        setRGBIORecipe(1, recipe);
      }

      for (byte i = 0; i <= VS_KETTLE; i++) {
        setPWMPin(i, INDEX_NONE);
        setVolumeSensor(i, INDEX_NONE);
        setPIDLimit(i, 100);
        setPWMResolution(i, 120);
      }
    case 4:
      //Set triggers to disabled by default
      setEStopEnabled(0);
      for (byte i = 0; i < 6; i++) {
        struct TriggerConfiguration *defaultConfig;
        defaultConfig->type = TRIGGERTYPE_NONE;
        defaultConfig->index = 0;
        defaultConfig->activeLow = 0;
        defaultConfig->threshold = 0;
        defaultConfig->profileFilter = 0;
        defaultConfig->disableMask = 0;
        defaultConfig->releaseHysteresis = 0;
        saveTriggerConfiguration(i, defaultConfig);
      }
    case 5:
      setBubblerOutput(INDEX_NONE); //Disabled
      setBubblerInterval(30); //30s
      setBubblerDuration(5); //0.5s
      setBubblerDelay(5); //0.5s
    case 6:
    #ifdef USEMETRIC
      setGrainDisplacement(1250); //1.250 litres /kg
      setGrainLiquorLoss(17884); //1.7884 litres /kg
    #else
      setGrainDisplacement(150); //0.15 Gallons /lb
      setGrainLiquorLoss(2143);  //0.2143 Gallons /lb
    #endif
    case 7:
      initializeBrewStepConfiguration();
      eepromSaveBrewStepConfiguration();
    case 8:
      setStrikeLoss(0);
      setSpargeLoss(0);
      setMashLoss(0);
      setBoilLoss(0);
    case 9:
      setMashTunHeatCapacity(0);
      setMinimumSpargeVolume(0);
      EEPROM.write(2047, 10);
  }
  return 0;
}

void initEEPROM() {
  //Format EEPROM to 0's
  for (int i = 0; i < 4096; i++) {
    #ifndef NOUI
    byte col = i / 205;
    if (i == col * 205) {
      LCD.writeCustChar(0, col, '-');
      LCD.update();
    }
    #endif
    EEPROM.write(i, 0);
  }

  //Set BT 1.3 Fingerprint (252)
  EEPROM.write(2046, 252);

  //Default Output Settings: p: 3, i: 4, d: 2, cycle: 4s, Hysteresis 0.3C(0.5F)
  for (byte vessel = VS_HLT; vessel <= VS_KETTLE; vessel++) {
    setPIDp(vessel, 3);
    setPIDi(vessel, 4);
    setPIDd(vessel, 2);
    setPWMPeriod(vessel, 1);
    #ifdef USEMETRIC
      setHysteresis(vessel, 3);
    #else
      setHysteresis(vessel, 5);      
    #endif
  }

  //Default Grain Temp = 60F/16C
  //If F else C
  #ifdef USEMETRIC
    setGrainTemp(16);
  #else
    setGrainTemp(60);
  #endif
  
  //Set Default Boil temp 212F/100C
  #ifdef USEMETRIC
    setBoilTemp(100);
  #else
    setBoilTemp(212);
  #endif

  setBoilPwr(100);

  //Set all steps idle
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++) {
    struct ProgramThread thread;
    thread.activeStep = INDEX_NONE;
    thread.recipe = INDEX_NONE;
    eepromSaveProgramThread(i, &thread);
  }
  #ifdef LCD_DEFAULT_BRIGHTNESS
    EEPROM.write(2048, LCD_DEFAULT_BRIGHTNESS);
  #endif
  #ifdef LCD_DEFAULT_CONTRAST
    EEPROM.write(2049, LCD_DEFAULT_CONTRAST);
  #endif
  
  //Set cfgVersion = 0
  EEPROM.write(2047, 0);
  if (checkConfig())
    loadSetup();
}

//*****************************************************************************************************************************
// EEPROM Type Read/Write Functions
//*****************************************************************************************************************************
long EEPROMreadLong(int address) {
  long out;
  eeprom_read_block((void *) &out, (unsigned char *) address, 4);
  return out;
}

void EEPROMwriteLong(int address, long value) {
  eeprom_write_block((void *) &value, (unsigned char *) address, 4);
}

int EEPROMreadInt(int address) {
  int out;
  eeprom_read_block((void *) &out, (unsigned char *) address, 2);
  return out;
}

void EEPROMwriteInt(int address, int value) {
  eeprom_write_block((void *) &value, (unsigned char *) address, 2);
}

void EEPROMwriteBytes(int addr, byte bytes[], byte numBytes) {
  for (byte i = 0; i < numBytes; i++) {
    EEPROM.write(addr + i, bytes[i]);
  }
}

void EEPROMreadBytes(int addr, byte bytes[], byte numBytes) {
  for (byte i = 0; i < numBytes; i++) {
    bytes[i] = EEPROM.read(addr + i);
  }
}

