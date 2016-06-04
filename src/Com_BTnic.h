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

  Update 9/22/2010 to support enhanced functions and mutiple schemas.
  
*/
#ifdef BTNIC_PROTOCOL

#include "Outputs.h"
#include "EEPROM.h"
#include "StepLogic.h"
#include "Temp.h"
#include "Timer.h"

/********************************************************************************************************************
 * BTnic Class
 ********************************************************************************************************************/
  // Using Values 65-90 & 97-122 for command codes to make terminal input easier 
  // but any value between 0-255 can be used with the following exceptions
  // Command Field Char         9  (Tab)
  // Command Term Char          10 (Line Feed)
  // Command Term Char          13 (Carriage Return)

  //Command codes for special responses
  #define CMD_REJECT            33  //!
  #define CMD_REJECT_PARAM      35  //#
  #define CMD_REJECT_INDEX      36  //$
  #define CMD_REJECT_CRC        42  //*

  #define CMD_GET_PROGNAMES     62  //%3E
  #define CMD_SET_PROGRAM       63  //%3F
  #define CMD_GET_PROGRAM       64  //%40
  #define CMD_GET_BOIL          65  //A
  #define CMD_GET_CAL           66  //B
  #define CMD_GET_EVAP          67  //C
  #define CMD_GET_OSET          68  //D
  #define CMD_GET_PROG          69  //E
  #define CMD_GET_TS            70  //F
  #define CMD_GET_VER           71  //G
  #define CMD_GET_VSET          72  //H
  #define CMD_INIT_EEPROM       73  //I
  #define CMD_SCAN_TS           74  //J
  #define CMD_SET_BOIL          75  //K
  #define CMD_SET_CAL           76  //L
  #define CMD_SET_EVAP          77  //M
  #define CMD_SET_OSET          78  //N
  #define CMD_SET_PROG          79  //O
  #define CMD_SET_TS            80  //P
  #define CMD_SET_VLVCFG        81  //Q
  #define CMD_SET_VSET          82  //R
  #define CMD_ADV_STEP          83  //S
  #define CMD_EXIT_STEP         84  //T
  #define CMD_INIT_STEP         85  //U
  #define CMD_SET_ALARM         86  //V
  #define CMD_SET_AUTOVLV       87  //W
  #define CMD_SET_SETPOINT      88  //X
  #define CMD_SET_TIMERSTATUS   89  //Y
  #define CMD_SET_TIMERVALUE    90  //Z
  #define CMD_GET_PROGNAME      91  //'['
  #define CMD_SET_PROGNAME      92  //'\'
  #define CMD_GET_PROGTEMPS     93  //']'
  #define CMD_SET_PROGTEMPS     94  //'^'
  #define CMD_GET_PROGMINS      95  //'_'
  #define CMD_SET_PROGMINS      96  //'`'
  #define CMD_GET_STATUS        97  //a
  #define CMD_SET_VLVPRF        98  //b
  #define CMD_RESET             99  //c
  #define CMD_GET_VLVCFG        100 //d
  #define CMD_GET_ALARM         101 //e
  #define CMD_GET_BOILPWR       102 //f
  #define CMD_GET_DELAYTIME     103 //g
  #define CMD_GET_GRAINTEMP     104 //h
  #define CMD_SET_BOILPWR       105 //i
  #define CMD_SET_DELAYTIME     106 //j
  #define CMD_SET_GRAINTEMP     107 //k
  #define CMD_STEPPRG           110 //n
  #define CMD_TIMER             111 //o
  #define CMD_VOL               112 //p
  #define CMD_TEMP              113 //q
  #define CMD_STEAM             114 //r
  #define CMD_HEATPWR           115 //s
  #define CMD_SETPOINT          116 //t
  #define CMD_AUTOVLV           117 //u
  #define CMD_VLVBITS           118 //v
  #define CMD_VLVPRF            119 //w
  #define CMD_GET_PROGVOLS      120 //x
  #define CMD_SET_PROGVOLS      121 //y
  #define CMD_SET_TGTVOL        123 //{
  #define CMD_GET_TGTVOL        124 //|
  #define CMD_SET_BOILCTL       125 //}
  #define CMD_GET_BOILCTL       126 //~
  
  typedef enum {
    BTNIC_STATE_IDLE,
    BTNIC_STATE_RX,
    BTNIC_STATE_EXE,
    BTNIC_STATE_TX,
  } BTNICState;
  
  #define BTNIC_BUF_LEN 1024

  #define CMDCODE_MIN 62
  #define CMDCODE_MAX 126
  #define NO_CMDINDEX -1
  
  static const byte CMD_PARAM_COUNTS[] PROGMEM =
  {
    0,  //CMD_GET_PROGNAMES (All)
    22, //CMD_SET_PROGRAM (Full)
    0,  //CMD_GET_PROGRAM (Full)
    0,  //CMD_GET_BOIL
    0,  //CMD_GET_CAL
    0,  //CMD_GET_EVAP
    0,  //CMD_GET_OSET
    0,  //CMD_GET_PROG
    0,  //CMD_GET_TS
    0,  //CMD_GET_VER
    0,  //CMD_GET_VSET
    1,  //CMD_INIT_EEPROM
    0,  //CMD_SCAN_TS
    1,  //CMD_SET_BOIL
    2,  //CMD_SET_CAL
    1,  //CMD_SET_EVAP
    8,  //CMD_SET_OSET
    6,  //CMD_SET_PROG
    8,  //CMD_SET_TS
    1,  //CMD_SET_VLVCFG
    2,  //CMD_SET_VSET
    0,  //CMD_ADV_STEP
    0,  //CMD_EXIT_STEP
    1,  //CMD_INIT_STEP
    1,  //CMD_SET_ALARM
    1,  //CMD_SET_AUTOVLV
    1,  //CMD_SET_SETPOINT
    1,  //CMD_SET_TIMERSTATUS
    1,  //CMD_SET_TIMERVALUE
    0,  //CMD_GET_PROGNAME
    1,  //CMD_SET_PROGNAME
    0,  //CMD_GET_PROGMASHTEMPS
    6,  //CMD_SET_PROGMASHTEMPS
    0,  //CMD_GET_PROGMASHMINS
    6,  //CMD_SET_PROGMASHMINS
    0,  //CMD_GET_STATUS
    2,  //CMD_SET_VLVPRF
    0,  //CMD_RESET
    0,  //CMD_GET_VLVCFG
    0,  //CMD_GET_ALARM
    0,  //CMD_GET_BOILPWR
    0,  //CMD_GET_DELAYTIME
    0,  //CMD_GET_GRAINTEMP
    1,  //CMD_SET_BOILPWR
    1,  //CMD_SET_DELAYTIME
    1,  //CMD_SET_GRAINTEMP
    0,  //Unused
    0,  //Unused
    0,  //CMD_STEPPRG
    0,  //CMD_TIMER
    0,  //CMD_VOL
    0,  //CMD_TEMP
    0,  //CMD_STEAM
    0,  //CMD_HEATPWR
    0,  //CMD_SETPOINT
    0,  //CMD_AUTOVLV
    0,  //CMD_VLVBITS
    0,  //CMD_VLVPRF
    0,  //CMD_GET_PROGVOLS
    3,  //CMD_SET_PROGVOLS
    0,  //Unused
    1,  //CMD_SET_TGTVOL
    0,  //CMD_GET_TGTVOL
    2,  //CMD_SET_BOILCNT
    0   //CMD_GET_BOILCNT
  };

  static const byte CMD_INDEX_MAXVALUE[] PROGMEM =
  {
    0,                  //CMD_GET_PROGNAMES (All)
    RECIPE_MAX - 1,     //CMD_SET_PROGRAM (Full)
    RECIPE_MAX - 1,     //CMD_GET_PROGRAM (Full)
    0,                  //CMD_GET_BOIL
    29,                 //CMD_GET_CAL (0-9 HLT, 10-19 Mash, 20-29 Kettle)
    0,                  //CMD_GET_EVAP
    VS_STEAM,           //CMD_GET_OSET
    RECIPE_MAX - 1,     //CMD_GET_PROG
    NUM_TS - 1,         //CMD_GET_TS
    0,                  //CMD_GET_VER
    VS_KETTLE,          //CMD_GET_VSET
    0,                  //CMD_INIT_EEPROM
    0,                  //CMD_SCAN_TS
    0,                  //CMD_SET_BOIL
    29,                 //CMD_SET_CAL (0-9 HLT, 10-19 Mash, 20-29 Kettle)
    0,                  //CMD_SET_EVAP
    VS_STEAM,           //CMD_SET_OSET
    RECIPE_MAX - 1,     //CMD_SET_PROG
    NUM_TS - 1,         //CMD_SET_TS
    NUM_VLVCFGS - 1,    //CMD_SET_VLVCFG
    VS_KETTLE,          //CMD_SET_VSET
    BREWSTEP_COUNT - 1, //CMD_ADV_STEP
    BREWSTEP_COUNT - 1, //CMD_EXIT_STEP
    BREWSTEP_COUNT - 1, //CMD_INIT_STEP
    0,                  //CMD_SET_ALARM
    0,                  //CMD_SET_AUTOVLV
    VS_STEAM,           //CMD_SET_SETPOINT
    TIMER_BOIL,         //CMD_SET_TIMERSTATUS
    TIMER_BOIL,         //CMD_SET_TIMERVALUE
    RECIPE_MAX - 1,     //CMD_GET_PROGNAME
    RECIPE_MAX - 1,     //CMD_SET_PROGNAME
    RECIPE_MAX - 1,     //CMD_GET_PROGTEMPS
    RECIPE_MAX - 1,     //CMD_SET_PROGTEMPS
    RECIPE_MAX - 1,     //CMD_GET_PROGMINS
    RECIPE_MAX - 1,     //CMD_SET_PROGMINS
    0,                  //CMD_GET_STATUS
    0,                  //CMD_SET_VLVPRF
    1,                  //CMD_RESET
    NUM_VLVCFGS - 1,    //CMD_GET_VLVCFG
    0,                  //CMD_GET_ALARM
    0,                  //CMD_GET_BOILPWR
    0,                  //CMD_GET_DELAYTIME
    0,                  //CMD_GET_GRAINTEMP
    0,                  //CMD_SET_BOILPWR
    0,                  //CMD_SET_DELAYTIME
    0,                  //CMD_SET_GRAINTEMP
    0,                  //Unused
    0,                  //Unused
    0,                  //CMD_STEPPRG
    TIMER_BOIL,         //CMD_TIMER
    VS_KETTLE,          //CMD_VOL
    NUM_TS - 1,         //CMD_TEMP
    0,                  //CMD_STEAM
    VS_STEAM,           //CMD_HEATPWR
    VS_STEAM,           //CMD_SETPOINT
    0,                  //CMD_AUTOVLV
    0,                  //CMD_VLVBITS
    0,                  //CMD_VLVPRF
    RECIPE_MAX - 1,     //CMD_GET_PROGVOLS
    RECIPE_MAX - 1,     //CMD_SET_PROGVOLS
    0,                  //Unused
    VS_KETTLE,          //CMD_SET_TGTVOL
    VS_KETTLE,          //CMD_GET_TGTVOL
    0,                  //CMD_SET_BOILCTL
    0                   //CMD_GET_BOILCTL
  };
  
class BTnic
{
public:
  BTnic(void); /* Constructor */
  void rx(byte); /* Receive a byte into buffer and returns true of tx ready*/
  byte tx(void); /* Return a byte from buffer or '/0' if end of buffer */
  void reset(void); /* Sets buf len to 0 */
  BTNICState getState(void); /* Return current state RX / EXE / TX */
  void eventHandler(byte, int);
  
private:
  void setState(BTNICState);
  void execCmd(void);
  void rejectCmd(byte);
  void chkBuf(void);
  void logFieldCmd(byte, int);
  void logField(char*);
  void logField_P(const char*);
  void logFieldI(unsigned long);
  void logEnd(void);
  void logStepPrg(void);
  int getCmdIndex(void);
  byte getCmdParamCount(void);
  char* getCmdParam(byte, char*, byte);
  unsigned long getCmdParamNum(byte);
  
  BTNICState _state; /* Current state: IDLE/RX/EXE/TX */
  unsigned int _bufLen; /* Length of data in buffer */
  byte _bufData[BTNIC_BUF_LEN]; /* Buffer */
  unsigned int _bufCur; /* Cursor position in buffer for tx */
};

BTnic::BTnic(void) {
  _bufLen = _bufCur = 0;
  _state = BTNIC_STATE_IDLE;
}

void BTnic::rx(byte byteIn) {
  if (_state == BTNIC_STATE_IDLE)
    setState(BTNIC_STATE_RX);
  if (byteIn == 0x01)
    _bufLen = _bufCur = 0;
  else if (byteIn == 0x0D) execCmd();
  else {
    _bufData[_bufLen++] = byteIn;
    if (_bufLen == BTNIC_BUF_LEN) execCmd();
  }
}

byte BTnic::tx(void) {
  if (_bufCur < _bufLen) return _bufData[_bufCur++];
  else return '\0';
}

void BTnic::reset(void) {
  _bufLen = _bufCur = 0;
  setState(BTNIC_STATE_IDLE);
}

BTNICState BTnic::getState(void) { 
  if (_state == BTNIC_STATE_TX && _bufCur == _bufLen) reset();
  return _state;
}

void BTnic::eventHandler(byte eventID, int eventParam) {
  //Not Implemented
}

void BTnic::setState(BTNICState state) {
  #ifdef DEBUG_BTNIC
    Serial.print("BTNIC State Change: ");
    Serial.print(_state, DEC);
    Serial.print(" -> ");
    Serial.println(state, DEC);
  #endif
  _state = state;
}

  //Check and process command. Return error code (0 if OK)
void BTnic::execCmd(void) {
  setState(BTNIC_STATE_EXE);

  // log ASCII version "GET_VER"
  if (strcasecmp(getCmdParam(0, buf, 20), "GET_VER") == 0) {
    logASCIIVersion();
    return reset();
  }

  if(!_bufLen || _bufData[0] < CMDCODE_MIN || _bufData[0] > CMDCODE_MAX) return rejectCmd(CMD_REJECT);
  if(getCmdParamCount() != pgm_read_byte(CMD_PARAM_COUNTS + _bufData[0] - CMDCODE_MIN)) return rejectCmd(CMD_REJECT_PARAM);
  int cmdIndex = getCmdIndex();
  if(cmdIndex == NO_CMDINDEX) return rejectCmd(CMD_REJECT_INDEX);

  switch (_bufData[0]) {
    case CMD_GET_STATUS: //a
      logFieldCmd(CMD_GET_STATUS, NO_CMDINDEX);
      logFieldI(alarmStatus);
      logFieldI(autoValveBitmask());
      logFieldI(actProfiles);
      logFieldI(computeValveBits());
      for (byte vessel = VS_HLT; vessel <= VS_KETTLE; vessel++) {
        logFieldI(setpoint[vessel]);
        logFieldI(temp[vessel]);
        logFieldI(getHeatPower(vessel)); 
        logFieldI(tgtVol[vessel]);
        logFieldI(volAvg[vessel]);
        #ifdef FLOWRATE_CALCS
          logFieldI(flowRate[vessel]);
        #else
          logFieldI(0);
        #endif
      }
      //Include remaining Temp Sensors.
      for (byte sensor = TS_H2OIN; sensor<NUM_TS; sensor++)
          logFieldI(temp[sensor]);

      for (byte timer = TIMER_MASH; timer <= TIMER_BOIL; timer++) {
        logFieldI(timerValue[timer]);
        logFieldI(timerStatus[timer]);
      }
      logFieldI(boilControlState);      
      logStepPrg();
      break;
    
    case CMD_SET_BOIL:  //K
      setBoilTemp(getCmdParamNum(1));
    case CMD_GET_BOIL:  //A
      logFieldCmd(CMD_GET_BOIL, NO_CMDINDEX);
      logFieldI(getBoilTemp());
      break;


    case CMD_SET_CAL:  //L
      {
        byte vessel = cmdIndex / 10;
        byte calIndex = cmdIndex - vessel * 10;
        setVolCalib(vessel, calIndex, getCmdParamNum(2), getCmdParamNum(1));
      }
    case CMD_GET_CAL: //B
      {
        logFieldCmd(CMD_GET_CAL, cmdIndex);
        byte vessel = cmdIndex / 10;
        byte calIndex = cmdIndex - vessel * 10;      
        logFieldI(calibVols[vessel][calIndex]);
        logFieldI(calibVals[vessel][calIndex]);
      }
      break;
      
      
    case CMD_SET_EVAP:  //M
      setEvapRate(min(getCmdParamNum(1), 100));
    case CMD_GET_EVAP:  //C
      logFieldCmd(CMD_GET_EVAP, NO_CMDINDEX);
      logFieldI(getEvapRate());
      break;
      
      
    case CMD_SET_OSET:  //N
      setPIDEnabled(cmdIndex, getCmdParamNum(1));
      setPIDCycle(cmdIndex, getCmdParamNum(2));
      setPIDp(cmdIndex, getCmdParamNum(3));
      setPIDi(cmdIndex, getCmdParamNum(4));
      setPIDd(cmdIndex, getCmdParamNum(5));
      if (cmdIndex == VS_STEAM) {
        setSteamZero(getCmdParamNum(6));
        setSteamTgt(getCmdParamNum(7));
        setSteamPSens(getCmdParamNum(8));
      } 
      else setHysteresis(cmdIndex, getCmdParamNum(6));
    case CMD_GET_OSET:  //D
      logFieldCmd(CMD_GET_OSET, cmdIndex);
      logFieldI(PIDEnabled[cmdIndex]);
      logFieldI(PIDCycle[cmdIndex]);
      logFieldI(getPIDp(cmdIndex));
      logFieldI(getPIDi(cmdIndex));
      logFieldI(getPIDd(cmdIndex));
      if (cmdIndex == VS_STEAM) {
        logFieldI(getSteamTgt());
        #ifndef PID_FLOW_CONTROL
          logFieldI(steamZero);
          logFieldI(steamPSens);
        #endif
      } 
      else {
        logFieldI(hysteresis[cmdIndex]);
        logFieldI(0);
        logFieldI(0);
      }
      break;

  
    case CMD_SET_PROGNAME:  //'['
      {
        char pName[20];
        getCmdParam(1, pName, 19);
        setProgName(cmdIndex, pName);
      }
    case CMD_GET_PROGNAME:  //'\'
      logFieldCmd(CMD_GET_PROGNAME, cmdIndex);
      {
        char pName[20];
        getProgName(cmdIndex, pName);
        logField(pName);
      }
      break;

    case CMD_GET_PROGNAMES:  //%3E (All Program Names)
      logFieldCmd(CMD_GET_PROGNAMES, NO_CMDINDEX);
      for (byte i = 0; i < RECIPE_MAX; i++) {
        char pName[20];
        getProgName(i, pName);
        logField(pName);
      }
      break;
      
    case CMD_SET_PROGTEMPS:  //']'
      for (byte i = 0; i < MASHSTEP_COUNT; i++) {
        setProgMashTemp(cmdIndex, i, getCmdParamNum(i + 1) * SETPOINT_DIV);
      }
    case CMD_GET_PROGTEMPS:  //'^'
      logFieldCmd(CMD_GET_PROGTEMPS, cmdIndex);
      for (byte i = 0; i < MASHSTEP_COUNT; i++) {
        logFieldI(getProgMashTemp(cmdIndex, i) / SETPOINT_DIV);
      }
      break;

    case CMD_SET_PROGMINS:  //'_'
      for (byte i = 0; i < MASHSTEP_COUNT; i++) {
        setProgMashMins(cmdIndex, i, getCmdParamNum(i + 1));
      }
    case CMD_GET_PROGMINS:  //'`'
      logFieldCmd(CMD_GET_PROGMINS, cmdIndex);
      for (byte i = 0; i < MASHSTEP_COUNT; i++) {
        logFieldI(getProgMashMins(cmdIndex, i));
      }
      break;

    case CMD_SET_PROGVOLS:  //x
      setProgBatchVol(cmdIndex, getCmdParamNum(1));
      setProgGrain(cmdIndex, getCmdParamNum(2));
      setProgRatio(cmdIndex, getCmdParamNum(3));
    case CMD_GET_PROGVOLS:  //y
      logFieldCmd(CMD_GET_PROGVOLS, cmdIndex);
      logFieldI(getProgBatchVol(cmdIndex));
      logFieldI(getProgGrain(cmdIndex));
      logFieldI(getProgRatio(cmdIndex));
      break;
      
    case CMD_SET_PROG:  //O (Partial program data)
      setProgSparge(cmdIndex, getCmdParamNum(1) * SETPOINT_DIV);
      setProgHLT(cmdIndex, getCmdParamNum(2) * SETPOINT_DIV);
      setProgBoil(cmdIndex, getCmdParamNum(3));
      setProgPitch(cmdIndex, getCmdParamNum(4) * SETPOINT_DIV);
      setProgAdds(cmdIndex, getCmdParamNum(5));
      setProgMLHeatSrc(cmdIndex, getCmdParamNum(6));
    case CMD_GET_PROG:  //E (partial program data)
      logFieldCmd(CMD_GET_PROG, cmdIndex);
      logFieldI(getProgSparge(cmdIndex) / SETPOINT_DIV);
      logFieldI(getProgHLT(cmdIndex) / SETPOINT_DIV);
      logFieldI(getProgBoil(cmdIndex));
      logFieldI(getProgPitch(cmdIndex) / SETPOINT_DIV);
      logFieldI(getProgAdds(cmdIndex));
      logFieldI(getProgMLHeatSrc(cmdIndex));
      break;
      
    case CMD_SET_PROGRAM:
      {
        char pName[20];
        getCmdParam(1, pName, 19);
        setProgName(cmdIndex, pName);
      }
      setProgBatchVol(cmdIndex, getCmdParamNum(2));
      setProgGrain(cmdIndex, getCmdParamNum(3));
      setProgRatio(cmdIndex, getCmdParamNum(4));
      for (byte i = 0; i < MASHSTEP_COUNT; i++) {
        setProgMashTemp(cmdIndex, i, getCmdParamNum(i * 2 + 5) * SETPOINT_DIV);
        setProgMashMins(cmdIndex, i, getCmdParamNum(i * 2 + 6));
      }
      setProgSparge(cmdIndex, getCmdParamNum(17) * SETPOINT_DIV);
      setProgHLT(cmdIndex, getCmdParamNum(18) * SETPOINT_DIV);
      setProgBoil(cmdIndex, getCmdParamNum(19));
      setProgPitch(cmdIndex, getCmdParamNum(20) * SETPOINT_DIV);
      setProgAdds(cmdIndex, getCmdParamNum(21));
      setProgMLHeatSrc(cmdIndex, getCmdParamNum(22));
    case CMD_GET_PROGRAM:
      logFieldCmd(CMD_GET_PROGRAM, cmdIndex);
      {
        char pName[20];
        getProgName(cmdIndex, pName);
        logField(pName);
      }
      logFieldI(getProgBatchVol(cmdIndex));
      logFieldI(getProgGrain(cmdIndex));
      logFieldI(getProgRatio(cmdIndex));
      for (byte i = 0; i < MASHSTEP_COUNT; i++) {
        logFieldI(getProgMashTemp(cmdIndex, i) / SETPOINT_DIV);
        logFieldI(getProgMashMins(cmdIndex, i));
      }
      logFieldI(getProgSparge(cmdIndex) / SETPOINT_DIV);
      logFieldI(getProgHLT(cmdIndex) / SETPOINT_DIV);
      logFieldI(getProgBoil(cmdIndex));
      logFieldI(getProgPitch(cmdIndex) / SETPOINT_DIV);
      logFieldI(getProgAdds(cmdIndex));
      logFieldI(getProgMLHeatSrc(cmdIndex));
      logFieldI(calcStrikeTemp(cmdIndex) / SETPOINT_DIV);
      logFieldI(getFirstStepTemp(cmdIndex) / SETPOINT_DIV);
      logFieldI(calcPreboilVol(cmdIndex));
      logFieldI(calcStrikeVol(cmdIndex));
      logFieldI(calcSpargeVol(cmdIndex));
      logFieldI(calcGrainVolume(cmdIndex));
      logFieldI(calcGrainLoss(cmdIndex));      
      break;
      
    case CMD_SET_TS:  //P
      {
        byte addr[TEMP_ADDR_SIZE];
        for (byte i=0; i<TEMP_ADDR_SIZE; i++) addr[i] = (byte)getCmdParamNum(i+1);
        setTSAddr(cmdIndex, addr);
      }
    case CMD_GET_TS:  //F
      logFieldCmd(CMD_GET_TS, cmdIndex);
      for (byte i=0; i<TEMP_ADDR_SIZE; i++) logFieldI(tSensor[cmdIndex][i]);
      break;
      
      
    case CMD_GET_VER:  //G
      logFieldCmd(CMD_GET_VER, NO_CMDINDEX);
      logField_P(BTVER);
      logFieldI(BUILDNUM);
      logFieldI(BTNIC);
      logFieldI(COMSCHEMA);
      #ifdef USEMETRIC
        logFieldI(0);
      #else
        logFieldI(1);
      #endif
      break;


    case CMD_SET_VSET:  //R
      setCapacity(cmdIndex, getCmdParamNum(1));
      setVolLoss(cmdIndex, getCmdParamNum(2));
    case CMD_GET_VSET:  //H
      logFieldCmd(CMD_GET_VSET, cmdIndex);
      logFieldI(getCapacity(cmdIndex));
      logFieldI(getVolLoss(cmdIndex));  
      break;


    case CMD_INIT_EEPROM:  //I
      if (getCmdParamNum(1) != 210) return rejectCmd(CMD_REJECT_PARAM);
      logFieldCmd(CMD_INIT_EEPROM, NO_CMDINDEX);
      initEEPROM();
      break;
      

    case CMD_SCAN_TS:  //J
      {
        logFieldCmd(CMD_SCAN_TS, NO_CMDINDEX);
        byte tsAddr[TEMP_ADDR_SIZE];
        getDSAddr(tsAddr);
        for (byte i=0; i<TEMP_ADDR_SIZE; i++) logFieldI(tsAddr[i]);
      }
      break;
      
      
    case CMD_SET_VLVCFG:  //Q
      setValveCfg(cmdIndex, getCmdParamNum(1));
    case CMD_GET_VLVCFG:  //d
      logFieldCmd(CMD_GET_VLVCFG, cmdIndex);
      logFieldI(vlvConfig[cmdIndex]);  
      break; 


    case CMD_INIT_STEP:  //U
    case CMD_ADV_STEP:  //S
    case CMD_EXIT_STEP:  //T
      if (_bufData[0] == CMD_INIT_STEP) {
        byte progNum = getCmdParamNum(1);
        if (progNum >= 20) return rejectCmd(CMD_REJECT_PARAM);
        programThreadInit(progNum, cmdIndex);
      }
      else if (_bufData[0] == CMD_ADV_STEP) brewStepSignal(cmdIndex, STEPSIGNAL_ADVANCE);
      else if (_bufData[0] == CMD_EXIT_STEP) brewStepSignal(cmdIndex, STEPSIGNAL_ABORT);
    case CMD_STEPPRG:  //n
        logFieldCmd(CMD_STEPPRG, NO_CMDINDEX);
        logStepPrg();
      break;



    case CMD_SET_ALARM:  //V
      setAlarm(getCmdParamNum(1));
    case CMD_GET_ALARM:  //e
      logFieldCmd(CMD_GET_ALARM, NO_CMDINDEX);
      logFieldI(alarmStatus);
      break;
      
      
    case CMD_SET_AUTOVLV:  //W
      {
        byte actModes = getCmdParamNum(1);
        for (byte i = AV_FILL; i <= AV_HLT; i++) 
          autoValve[i] = (actModes & (1<<i));
      }
    case CMD_AUTOVLV:  //u
      {
        logFieldCmd(CMD_AUTOVLV, NO_CMDINDEX);
        logFieldI(autoValveBitmask());
      }
      break;
      
      
    case CMD_SET_SETPOINT:  //X
      setSetpoint(cmdIndex, getCmdParamNum(1) * SETPOINT_DIV);
    case CMD_SETPOINT:  //t
      logFieldCmd(CMD_SETPOINT, cmdIndex);
      logFieldI(setpoint[cmdIndex] / (SETPOINT_MULT * SETPOINT_DIV));
      break;
      

    case CMD_SET_TIMERSTATUS:  //Y
    case CMD_SET_TIMERVALUE:  //Z
      if (_bufData[0] == CMD_SET_TIMERSTATUS) {
        if (getCmdParamNum(1) != timerStatus[cmdIndex])
          pauseTimer(cmdIndex);
      } else {
        timerValue[cmdIndex] = getCmdParamNum(1);
        lastTime[cmdIndex] = millis();
      }
    case CMD_TIMER:  //o
      logFieldCmd(CMD_TIMER, cmdIndex);
      logFieldI(timerValue[cmdIndex]);
      logFieldI(timerStatus[cmdIndex]);
      break; 

      
    case CMD_SET_VLVPRF:  //b
      //Check param 2 (value) and set/unset specified active profiles
      if (getCmdParamNum(2)) actProfiles |= getCmdParamNum(1);
      else actProfiles &= ~getCmdParamNum(1);
    case CMD_VLVPRF:  //w
      logFieldCmd(CMD_VLVPRF, NO_CMDINDEX);
      logFieldI(actProfiles);
      break;
      
      
    case CMD_RESET:  //c
      //Reboot (1) or just Reset Outputs?
      if (cmdIndex == 1) softReset();
      else {
        logFieldCmd(CMD_RESET, cmdIndex);
        programThreadResetAll();
        resetOutputs();
        clearTimer(TIMER_MASH);
        clearTimer(TIMER_BOIL);
      }
      break;
      

    case CMD_SET_BOILPWR:  //i
      setBoilPwr(getCmdParamNum(1));
    case CMD_GET_BOILPWR:  //f
      logFieldCmd(CMD_GET_BOILPWR, NO_CMDINDEX);
      logFieldI(boilPwr);
      break;
      
      
    case CMD_SET_DELAYTIME:  //j
      setDelayMins(getCmdParamNum(1));
    case CMD_GET_DELAYTIME:  //g
      logFieldCmd(CMD_GET_DELAYTIME, NO_CMDINDEX);
      logFieldI(getDelayMins());
      break;
      
      
    case CMD_SET_GRAINTEMP:  //k
      setGrainTemp(getCmdParamNum(1));
    case CMD_GET_GRAINTEMP:  //h
      logFieldCmd(CMD_GET_GRAINTEMP, NO_CMDINDEX);
      logFieldI(getGrainTemp());
      break;
      
    
    case CMD_VOL:  //p
      logFieldCmd(CMD_VOL, cmdIndex);
      logFieldI(volAvg[cmdIndex]);
      #ifdef FLOWRATE_CALCS
        logFieldI(flowRate[cmdIndex]);
      #else
        logFieldI(0);
      #endif
      break;
      
      
    case CMD_TEMP:  //q
      logFieldCmd(CMD_TEMP, cmdIndex);
      logFieldI(temp[cmdIndex]);
      break;
      
      
    case CMD_STEAM:  //r
      logFieldCmd(CMD_STEAM, NO_CMDINDEX);
      #ifdef PID_FLOW_CONTROL
        logFieldI(flowRate[VS_MASH]);
      #else
        logFieldI(steamPressure);  
      #endif
      break;


    case CMD_HEATPWR:  //s
      logFieldCmd(CMD_HEATPWR, cmdIndex);
      logFieldI(getHeatPower(cmdIndex));
      break;
      

    case CMD_VLVBITS:  //v
      logFieldCmd(CMD_VLVBITS, NO_CMDINDEX);
      logFieldI(computeValveBits());
      break;
      
    case CMD_SET_TGTVOL:  //{
      tgtVol[cmdIndex] = min(getCmdParamNum(1), getCapacity(cmdIndex));
    case CMD_GET_TGTVOL:  //|
      logFieldCmd(CMD_GET_TGTVOL, cmdIndex);
      logFieldI(tgtVol[cmdIndex]);
      break;
      
    case CMD_SET_BOILCTL: //}
      boilControlState = (ControlState)getCmdParamNum(1);
      switch (boilControlState) {
        case CONTROLSTATE_OFF:
          PIDOutput[VS_KETTLE] = 0;
          setpoint[VS_KETTLE] = 0;
          break;
        case CONTROLSTATE_AUTO:
          setpoint[VS_KETTLE] = getBoilTemp();
          break;
        case CONTROLSTATE_ON:
          setpoint[VS_KETTLE] = 1;
          PIDOutput[VS_KETTLE] = PIDCycle[VS_KETTLE] * getCmdParamNum(2);
        break;
      }
    case CMD_GET_BOILCTL: //~
      logFieldCmd(CMD_GET_BOILCTL, NO_CMDINDEX);
      logFieldI(boilControlState);
      logFieldI(PIDOutput[VS_KETTLE] / PIDCycle[VS_KETTLE]);
      break;
      
    default: 
      return rejectCmd(CMD_REJECT); //Reject Command Code (CMD_REJECT);
  }
  logEnd();
}

void BTnic::logStepPrg() {
  for (byte i = 0; i < PROGRAMTHREAD_MAX; i++) {
    logFieldI(programThreadActiveStep(i));
    char pName[20];
    programThreadRecipeName(i, pName);
    logField(pName);
    logFieldI(programThreadRecipeIndex(i));
  }
}
  
void BTnic::rejectCmd(byte rejectCode) {
  //Unknown Command: Shift buffer two chars and insert rejectCode and field delimeter
  if (_bufLen) {
    _bufLen = min(_bufLen + 2, BTNIC_BUF_LEN);
    _bufCur = _bufLen - 1;
    while (_bufCur > 1) _bufData[_bufCur] = _bufData[(_bufCur--) - 2];
    _bufData[1] = 0x09;
  } else _bufLen = 1;
  _bufData[0] = rejectCode;
  logEnd();  
}

void BTnic::logFieldCmd(byte cmdCode, int cmdIndex) {
  char tmpbuf[8];
  tmpbuf[0] = cmdCode;
  tmpbuf[1] = '\0';
  if (cmdIndex != -1) {
    char indexBuf[7];
    itoa(cmdIndex, indexBuf, 10);
    strcat(tmpbuf, indexBuf);
  }
  _bufLen = 0;
  logField(tmpbuf);
}

void BTnic::logField(char *string) {
  if (_bufLen) _bufData[_bufLen++] = 0x09;  //Tab Char if not being used for cmd field
  while (*string != '\0') _bufData[_bufLen++] = *(string++);
}
  
void BTnic::logField_P(const char *string) {
  _bufData[_bufLen++] = 0x09;  //Tab Char
  while (pgm_read_byte(string) != 0) _bufData[_bufLen++] = pgm_read_byte(string++);
}

void BTnic::logFieldI(unsigned long param) {
  char tmpbuf[11];
  ultoa(param, tmpbuf, 10);
  logField(tmpbuf);
}
  
void BTnic::logEnd(void) {
  _bufLen = min(_bufLen, BTNIC_BUF_LEN - 2);
  _bufData[_bufLen++] = 0x0D; //Carriage Return
  _bufData[_bufLen++] = 0x0A; //New Line
  _bufCur = 0;
  setState(BTNIC_STATE_TX);
}

int BTnic::getCmdIndex() {
  byte maxValue = pgm_read_byte(CMD_INDEX_MAXVALUE +(_bufData[0] - CMDCODE_MIN));
  if (!maxValue) return 0;
  
  _bufCur = 1;
  
  char tmpbuf[11];
  while (_bufCur < _bufLen && _bufCur < 11 && _bufData[_bufCur] != 0x09) {
    if(!isdigit(_bufData[_bufCur]))
      return NO_CMDINDEX;
    tmpbuf[_bufCur - 1] = _bufData[_bufCur++];
  }
  
  if (_bufCur == 1) return NO_CMDINDEX;   //Missing Index
  tmpbuf[_bufCur - 1] = '\0';
  int cmdIndex = atoi(tmpbuf);
  if (cmdIndex < 0 || cmdIndex > maxValue) return NO_CMDINDEX;
  else return cmdIndex;
}
  
byte BTnic::getCmdParamCount() {
  byte paramCount = 0;
  for (byte pos = 0; pos < _bufLen; pos++) if (_bufData[pos] == 0x09) paramCount++;
  return paramCount;
}
  
char* BTnic::getCmdParam(byte paramNum, char *retStr, byte limit) {
  byte pos = 0;
  byte param = 0;
  while (pos < _bufLen && param < paramNum) { if (_bufData[pos++] == 0x09) param++; }
  byte retPos = 0;
  while (pos < _bufLen && _bufData[pos] != 0x09 && retPos < limit) { retStr[retPos++] = _bufData[pos++]; }
  retStr[retPos] = '\0';
  return retStr;
}
  
unsigned long BTnic::getCmdParamNum(byte paramNum) {
  char tmpbuf[20];
  getCmdParam(paramNum, tmpbuf, 10);
  return strtoul(tmpbuf, NULL, 10);
}

/********************************************************************************************************************
 * End of BTnic Class
 ********************************************************************************************************************/

#endif //BTNIC_PROTOCOL
