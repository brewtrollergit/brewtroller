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
#include <EEPROM.h>
#include <avr/eeprom.h>
#include "BrewTroller.h"
#include "Events.h"
#include "EEPROM.h"
#include "HardwareProfile.h"

void loadSetup() {
  //**********************************************************************************
  //TSensors: HLT (0-7), MASH (8-15), KETTLE (16-23), H2OIN (24-31), H2OOUT (32-39),
  //          BEEROUT (40-47), AUX1 (48-55), AUX2 (56-63), AUX3 (64-71)
  //**********************************************************************************
  EEPROMreadBytes(0, *tSensor, 72);
  #ifdef HLT_AS_KETTLE
    EEPROMreadBytes(0, tSensor[TS_KETTLE], 8);
  #elif defined KETTLE_AS_MASH
    EEPROMreadBytes(16, tSensor[TS_MASH], 8);
  #elif defined SINGLE_VESSEL_SUPPORT
    EEPROMreadBytes(0, tSensor[TS_MASH], 8);
    EEPROMreadBytes(0, tSensor[TS_KETTLE], 8);
  #endif
 
  //**********************************************************************************
  //PID Enabled (72); Bit 1 = HLT, Bit 2 = Mash, Bit 3 = Kettle, Bit 4 = Steam
  //PIDp HLT (73), Mash (78), Kettle (83), Steam (88)
  //PIDi HLT (74), Mash (79), Kettle (84), Steam (89)
  //PIDd HLT (75), Mash (80), Kettle (85), Steam (90)
  //PIDCycle HLT (76), Mash (81), Kettle (86), Steam (91)
  //Hysteresis HLT (77), Mash (82), Kettle (87), Steam (92)
  //**********************************************************************************
  {
    byte options = EEPROM.read(72);
    for (byte i = VS_HLT; i <= VS_STEAM; i++) {
      PIDEnabled[i] = bitRead(options, i);
      PIDCycle[i] = EEPROM.read(76 + i * 5);
      hysteresis[i] = EEPROM.read(77 + i * 5);
    }
  }
  
  //**********************************************************************************
  //boilPwr (112)
  //**********************************************************************************
  boilPwr = EEPROM.read(112);
  //**********************************************************************************
  //steamZero (114)
  //**********************************************************************************
  steamZero = EEPROMreadInt(114);
  //**********************************************************************************
  //steamPSens (117-118)
  //**********************************************************************************
  steamPSens = EEPROMreadInt(117);

  //**********************************************************************************
  //calibVols HLT (119-158), Mash (159-198), Kettle (199-238)
  //calibVals HLT (239-258), Mash (259-278), Kettle (279-298)
  //**********************************************************************************
  eeprom_read_block(&calibVols, (unsigned char *) 119, 120);
  eeprom_read_block(&calibVals, (unsigned char *) 239, 60);

  //Load HLT calibrations to kettle
  #ifdef HLT_AS_KETTLE
    eeprom_read_block(&calibVols[VS_KETTLE], (unsigned char *) 119, 40);
    eeprom_read_block(&calibVals[VS_KETTLE], (unsigned char *) 239, 20);
  #elif defined KETTLE_AS_MASH
    eeprom_read_block(&calibVols[VS_MASH], (unsigned char *) 199, 40);
    eeprom_read_block(&calibVals[VS_MASH], (unsigned char *) 279, 20);
  #elif defined SINGLE_VESSEL_SUPPORT
    eeprom_read_block(&calibVols[VS_MASH], (unsigned char *) 119, 40);
    eeprom_read_block(&calibVals[VS_MASH], (unsigned char *) 239, 20);
    eeprom_read_block(&calibVols[VS_KETTLE], (unsigned char *) 119, 40);
    eeprom_read_block(&calibVals[VS_KETTLE], (unsigned char *) 239, 20);
  #endif

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

  //**********************************************************************************
  //Timer/Alarm Status (306)
  //**********************************************************************************
  byte options = EEPROM.read(306);
  for (byte i = TIMER_MASH; i <= TIMER_BOIL; i++) {
    timerStatus[i] = bitRead(options, i);
    lastTime[i] = millis();
  }
  alarmStatus = bitRead(options, 2);
#ifdef ALARM_PIN
  alarmPin.set(alarmStatus);
#endif
  #ifdef DEBUG_TIMERALARM
    logStart_P(LOGDEBUG);
    logField("TimerAlarmStatus");
    logFieldI(bitRead(options, 0));
    logFieldI(bitRead(options, 1));
    logFieldI(bitRead(options, 2));
    logEnd();
  #endif
  


  //**********************************************************************************
  //401-480 Valve Profiles
  //**********************************************************************************
  #ifdef PVOUT
    loadVlvConfigs();
  
    #ifdef PVOUT_TYPE_MODBUS
      for (byte i = 0; i < PVOUT_MODBUS_MAXBOARDS; i++) 
        loadVlvModbus(i);
    #endif
  #endif
}

#ifdef PVOUT
  void loadVlvConfigs() {
    eeprom_read_block(&vlvConfig, (unsigned char *) 401, 80);
  }
  
  #ifdef PVOUT_TYPE_MODBUS
    void loadVlvModbus(byte board) {
      if (ValvesMB[board]) {
        delete ValvesMB[board];
        ValvesMB[board] = NULL;
      }
      byte addr = getVlvModbusAddr(board);
      if (addr != PVOUT_MODBUS_ADDRNONE)
        ValvesMB[board] = new PVOutMODBUS(addr, getVlvModbusReg(board), getVlvModbusCoilCount(board), getVlvModbusOffset(board));
    }
  #endif 
#endif

//*****************************************************************************************************************************
// Individual EEPROM Get/Set Variable Functions
//*****************************************************************************************************************************

//**********************************************************************************
//TSensors: HLT (0-7), MASH (8-15), KETTLE (16-23), H2OIN (24-31), H2OOUT (32-39), 
//          BEEROUT (40-47), AUX1 (48-55), AUX2 (56-63), AUX3 (64-71)
//**********************************************************************************
void setTSAddr(byte sensor, byte addr[8]) {
    #ifdef HLT_AS_KETTLE
    if (sensor == TS_HLT || sensor == TS_KETTLE) {
      //Also copy HLT setting to Kettle
      memcpy(tSensor[TS_KETTLE], addr, 8);
      sensor = VS_HLT; //Set sensor for EEPROM write
    }
  #elif defined KETTLE_AS_MASH
    if (sensor == TS_MASH || sensor == TS_KETTLE) {
      //Also copy Kettle setting to Mash
      memcpy(tSensor[TS_MASH], addr, 8);
      sensor = VS_KETTLE; //Set sensor for EEPROM write
    }
  #elif defined SINGLE_VESSEL_SUPPORT
    if (sensor == TS_HLT || sensor == TS_MASH || sensor == TS_KETTLE) {
      //Also copy HLT setting to Mash/Kettle
      memcpy(tSensor[TS_MASH], addr, 8);
      memcpy(tSensor[TS_KETTLE], addr, 8);
      sensor = VS_HLT; //Set sensor for EEPROM write
    }
  #endif
  memcpy(tSensor[sensor], addr, 8);
  EEPROMwriteBytes(sensor * 8, addr, 8);
}

//**********************************************************************************
//PID Enabled (72); Bit 1 = HLT, Bit 2 = Mash, Bit 3 = Kettle, Bit 4 = Steam
//**********************************************************************************
void setPIDEnabled(byte vessel, boolean setting) {
  PIDEnabled[vessel] = setting;
  byte options = EEPROM.read(72);
  bitWrite(options, vessel, setting);
  EEPROM.write(72, options);
}


//**********************************************************************************
//PIDp HLT (73), Mash (78), Kettle (83), Steam (88)
//**********************************************************************************
void setPIDp(byte vessel, byte value) {
  pid[vessel].SetTunings(value, pid[vessel].GetI_Param(), pid[vessel].GetD_Param());
  EEPROM.write(73 + vessel * 5, value);
}
byte getPIDp(byte vessel) { return EEPROM.read(73 + vessel * 5); }

//**********************************************************************************
//PIDi HLT (74), Mash (79), Kettle (84), Steam (89)
//**********************************************************************************
void setPIDi(byte vessel, byte value) {
  pid[vessel].SetTunings(pid[vessel].GetP_Param(), value, pid[vessel].GetD_Param());
  EEPROM.write(74 + vessel * 5, value);
}
byte getPIDi(byte vessel) { return EEPROM.read(74 + vessel * 5); }

//**********************************************************************************
//PIDd HLT (75), Mash (80), Kettle (85), Steam (90)
//**********************************************************************************
void setPIDd(byte vessel, byte value) {
  pid[vessel].SetTunings(pid[vessel].GetP_Param(), pid[vessel].GetI_Param(), value);
  EEPROM.write(75 + vessel * 5, value);
}
byte getPIDd(byte vessel) { return EEPROM.read(75 + vessel * 5); }

//**********************************************************************************
//PIDCycle HLT (76), Mash (81), Kettle (86), Steam (91)
//**********************************************************************************
void setPIDCycle(byte vessel, byte value) {
  PIDCycle[vessel] = value;
  EEPROM.write(76 + vessel * 5, value);
}

//**********************************************************************************
//Hysteresis HLT (77), Mash (82), Kettle (87), Steam (92)
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
//volLoss HLT (105-106), Mash (107-108), Kettle (109-110)
//**********************************************************************************
void setVolLoss(byte vessel, unsigned int value) {
  EEPROMwriteInt(105 + vessel * 2, value);
}
unsigned int getVolLoss(byte vessel) { return EEPROMreadInt(105 + vessel * 2); }

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
//steamZero (114-115)
//**********************************************************************************
void setSteamZero(unsigned int value) {
  steamZero = value;
  EEPROMwriteInt(114, value);
}

//**********************************************************************************
//steamTgt (116)
//**********************************************************************************
void setSteamTgt(byte value) { EEPROM.write(116, value); }
byte getSteamTgt() { return EEPROM.read(116); }

//**********************************************************************************
//steamPSens (117-118)
//**********************************************************************************
void setSteamPSens(unsigned int value) {
  steamPSens = value;
  #ifndef PID_FLOW_CONTROL
  #ifdef USEMETRIC
    pid[VS_STEAM].SetInputLimits(0, 50000 / steamPSens);
  #else
    pid[VS_STEAM].SetInputLimits(0, 7250 / steamPSens);
  #endif
  #endif
  EEPROMwriteInt(117, value);
}

//**********************************************************************************
//calibVols HLT (119-158), Mash (159-198), Kettle (199-238)
//calibVals HLT (239-258), Mash (259-278), Kettle (279-298)
//**********************************************************************************
void setVolCalib(byte vessel, byte slot, unsigned int value, unsigned long vol) {
  #ifdef HLT_AS_KETTLE
    if (vessel == VS_HLT || vessel == VS_KETTLE) {
      //Also copy HLT setting to Kettle
      calibVols[VS_KETTLE][slot] = vol;
      calibVals[VS_KETTLE][slot] = value;
      vessel = VS_HLT; //Set vessel for EEPROM write
    }
  #elif defined KETTLE_AS_MASH
    if (vessel == VS_MASH || vessel == VS_KETTLE) {
      //Also copy Kettle setting to Mash
      calibVols[VS_MASH][slot] = vol;
      calibVals[VS_MASH][slot] = value;
      vessel = VS_KETTLE; //Set vessel for EEPROM write
    }
  #elif defined SINGLE_VESSEL_SUPPORT
    calibVols[VS_MASH][slot] = vol;
    calibVals[VS_MASH][slot] = value;  
    calibVols[VS_KETTLE][slot] = vol;
    calibVals[VS_KETTLE][slot] = value;
    vessel = VS_HLT; //Set vessel for EEPROM write
  #endif
  
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
  #if defined PID_FLOW_CONTROL || defined USESTEAM
    if (vessel == VS_STEAM) setpoint[vessel] = value;
    else setpoint[vessel] = value * SETPOINT_MULT;
  #else
    setpoint[vessel] = value * SETPOINT_MULT;
  #endif
  EEPROM.write(299 + vessel, value);
  eventHandler(EVENT_SETPOINT, vessel);
}

//**********************************************************************************
//timers (302-305)
//**********************************************************************************
void setTimerRecovery(byte timer, unsigned int newMins) {
  if(newMins != 0) EEPROMwriteInt(302 + timer * 2, newMins);
}

//**********************************************************************************
//Timer/Alarm Status (306)
//**********************************************************************************
void setTimerStatus(byte timer, boolean value) {
  timerStatus[timer] = value;
  byte options = EEPROM.read(306);
  bitWrite(options, timer, value);
  EEPROM.write(306, options);
  
  #ifdef DEBUG_TIMERALARM
    logStart_P(LOGDEBUG);
    logField("setTimerStatus");
    logFieldI(value);
    options = EEPROM.read(306);
    logFieldI(bitRead(options, timer));    
    logEnd();
  #endif
}

void setAlarmStatus(boolean value) {
  alarmStatus = value;
  byte options = EEPROM.read(306);
  bitWrite(options, 2, value);
  EEPROM.write(306, options);
  
  #ifdef DEBUG_TIMERALARM
    logStart_P(LOGDEBUG);
    logField("setAlarmStatus");
    logFieldI(value);
    options = EEPROM.read(306);
    logFieldI(bitRead(options, 2));
    logEnd();
  #endif
}



//**********************************************************************************
//Triggered Boil Addition Alarms (307-308)
//**********************************************************************************
unsigned int getBoilAddsTrig() { return EEPROMreadInt(307); }
void setBoilAddsTrig(unsigned int adds) { EEPROMwriteInt(307, adds); }

//**********************************************************************************
// ***OPEN*** (309-312)
//**********************************************************************************


//**********************************************************************************
//Program Threads (313-316)
//**********************************************************************************

void eepromLoadProgramThread(byte index, struct ProgramThread *thread) {
  eeprom_read_block((void *) thread, (unsigned char *) 313 + index * sizeof(struct ProgramThread), sizeof(struct ProgramThread));
}

void eepromSaveProgramThread(byte index, struct ProgramThread *thread) {
  eeprom_write_block((void *) thread, (unsigned char *) 313 + index * sizeof(struct ProgramThread), sizeof(struct ProgramThread));
}

//**********************************************************************************
// ***OPEN*** (317-397)
//**********************************************************************************

//**********************************************************************************
//Delay Start (Mins) (398-399)
//**********************************************************************************
unsigned int getDelayMins() { return EEPROMreadInt(398); }
void setDelayMins(unsigned int mins) {
  EEPROMwriteInt(398, mins);
}

//**********************************************************************************
//Grain Temp (400)
//**********************************************************************************
void setGrainTemp(byte grainTemp) { EEPROM.write(400, grainTemp); }
byte getGrainTemp() { return EEPROM.read(400); }

//*****************************************************************************************************************************
// Valve Profile Configuration (401-480; 481-785 Reserved)
//*****************************************************************************************************************************
void setValveCfg(byte profile, unsigned long value) {
  #ifdef PVOUT
    vlvConfig[profile] = value;
    EEPROMwriteLong(401 + profile * 4, value);
  #endif
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
//LCD Bright/Contrast (2048-2049) ATMEGA1284P Only
//**********************************************************************************

//**********************************************************************************
//Trigger Pins (2050-2054) ATMEGA1284P Only  +Reserved: 2055-2064
//**********************************************************************************
#ifdef DIGITAL_INPUTS
  byte getTriggerPin(byte triggerIndex) {
    #if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
      return EEPROM.read(2050 + triggerIndex);
    #else
      return 0;
    #endif
  }
  
  void setTriggerPin(byte triggerIndex, byte inputIndex) {
    #if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
      EEPROM.write(2050 + triggerIndex, inputIndex);
      triggerSetup(); //Call triggerSetup() to reattach
    #endif
  }
#endif


//**********************************************************************************
//Modbus Relay Boards (2065-2074) ATMEGA1284P Only + Reserved: 2075-2084 
//**********************************************************************************
byte getVlvModbusAddr(byte board) {
  return EEPROM.read(2065 + board * 5);
}

unsigned int getVlvModbusReg(byte board) {
  return EEPROMreadInt(2065 + board * 5 + 1);
}

byte getVlvModbusCoilCount(byte board) {
  return EEPROM.read(2065 + board * 5 + 3);
}

byte getVlvModbusOffset(byte board) {
  return EEPROM.read(2065 + board * 5 + 4);
}

void setVlvModbusAddr(byte board, byte addr) {
  EEPROM.write(2065 + board * 5, addr);
}

void setVlvModbusReg(byte board, unsigned int reg) {
  EEPROMwriteInt(2065 + board * 5 + 1, reg);
}

void setVlvModbusCoilCount(byte board, byte count) {
  EEPROM.write(2065 + board * 5 + 3, count);
}

void setVlvModbusOffset(byte board, byte offset) {
  EEPROM.write(2065 + board * 5 + 4, offset);
}

void setVlvModbusDefaults(byte board) {
  setVlvModbusAddr(board, PVOUT_MODBUS_ADDRNONE);
  setVlvModbusReg(board, PVOUT_MODBUS_DEFCOILREG);
  setVlvModbusCoilCount(board, PVOUT_MODBUS_DEFCOILCOUNT);
  byte defaultOffset = PVOUT_COUNT;
  if (board)
    for (byte i = 0; i < board; i++)
      defaultOffset += getVlvModbusOffset(board);
  setVlvModbusOffset(board, defaultOffset);
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
      for (byte vessel = VS_HLT; vessel <= VS_STEAM; vessel++) EEPROM.write(76 + vessel * 5, EEPROM.read(76 + vessel * 5) * 10);
      //Set cfgVersion = 1
      EEPROM.write(2047, 1);
    case 1:
      //Set triggers to disabled by default
      for (byte trig = 0; trig < NUM_TRIGGERS; trig++) EEPROM.write(2050 + trig, 0);
      EEPROM.write(2047, 2);
    case 2:
      for (byte i = 0; i < PVOUT_MODBUS_MAXBOARDS; i++)
        setVlvModbusDefaults(i);
      EEPROM.write(2047, 3);
    case 3:
      for (uint8_t i = 0; i < PVOUT_MODBUS_MAXBOARDS; i++) {
          setVlvModbusDefaults(i);
      }
      EEPROM.write(2057, 4);
  }
  return 0;
}

void initEEPROM() {
  //Format EEPROM to 0's
#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
  for (int i = 0; i < 4096; i++) EEPROM.write(i, 0);
#else
  for (int i = 0; i < 2048; i++) EEPROM.write(i, 0);
#endif

  //Set BT 1.3 Fingerprint (252)
  EEPROM.write(2046, 252);

  //Default Output Settings: p: 3, i: 4, d: 2, cycle: 4s, Hysteresis 0.3C(0.5F)
  for (byte vessel = VS_HLT; vessel <= VS_STEAM; vessel++) {
    setPIDp(vessel, 3);
    setPIDi(vessel, 4);
    setPIDd(vessel, 2);
    setPIDCycle(vessel, 4);
    if (vessel != VS_STEAM)
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
    thread.activeStep = BREWSTEP_NONE;
    thread.recipe = RECIPE_NONE;
    eepromSaveProgramThread(i, &thread);
  }
  
  //Set default LCD Bright/Contrast
  #if (defined __AVR_ATmega1284P__ || defined __AVR_ATmega1284__) && defined UI_DISPLAY_SETUP && defined UI_LCD_4BIT
    EEPROM.write(2048, LCD_DEFAULT_BRIGHTNESS);
    EEPROM.write(2049, LCD_DEFAULT_CONTRAST);
  #endif
  
  //Set cfgVersion = 0
  EEPROM.write(2047, 0);

  //restart
  softReset();
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

