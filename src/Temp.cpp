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
#include "Temp.h"
#include "HardwareProfile.h"
#include "Enum.h"
#include "BrewTroller.h"

#if TS_ONEWIRE_RES == 12
#define CONV_DELAY 750
#define RES_VALUE 0x7F
#elif TS_ONEWIRE_RES == 11
#define CONV_DELAY 375
#define RES_VALUE 0x5F
#elif TS_ONEWIRE_RES == 10
#define CONV_DELAY 188
#define RES_VALUE 0x3F
#else //Default to 9-bit
#define CONV_DELAY 94
#define RES_VALUE 0x1F
#endif

#define MAX_SCAN_COUNT       20    // Number of devices to scan for
#define SCRATCHPAD_SIZE      9     // Size of the scratchpad in bytes
#define CRC_SIZE             8     // Size of the CRC in bytes
#define CRC_INDEX            8     // Index of CRC in the scratchpad

#define DS18B20_IDENT        0x28
#define DS18S20_IDENT        0x10

#define CONVERT_TEMP_CMD     0x44
#define WRITE_SCRATCHPAD_CMD 0x4E
#define COPY_SCRATCHPAD_CMD  0x48
#define READ_SCRATCHPAD_CMD  0xBE

#define TL_REGISTER          0x46
#define TH_REGISTER          0x4B

#ifdef TS_ONEWIRE
  #ifdef TS_ONEWIRE_GPIO
    #include <OneWire.h>
    OneWire ds(TEMP_PIN);
  #endif
  #ifdef TS_ONEWIRE_I2C
    #include <DS2482.h>
    DS2482 ds(DS2482_ADDR);
    const uint8_t dsConfig = DS2482_CONFIG_APU | DS2482_CONFIG_SPU;
    bool masterIsConfigured = false;
  #endif
  //One Wire Bus on

  boolean tsReady();
  boolean validAddr(byte* addr);
  //Returns Int representing hundreths of degree
  int readTemp(byte* addr);
  #if defined MASH_AVG
    void mashAvg();
  #endif

  boolean tsReady() {
    #ifdef TS_ONEWIRE_I2C
      if (!masterIsConfigured) {
          tempInit(); // if the master isn't configured, try again
      }
      if (!masterIsConfigured) return false; // if we still aren't setup, abort
    #endif
    #if TS_ONEWIRE_PPWR == 0 //Poll if parasite power is disabled
      if (ds.read() == 0xFF) return 1;
    #endif
    return 0;
  }
  
  boolean validAddr(byte* addr) {
    for (byte i = 0; i < TEMP_ADDR_SIZE; i++) if (addr[i]) return 1;
    return 0;
  }
  
  //This function searchs for an address that is not currently assigned!
  void getDSAddr(byte addrRet[TEMP_ADDR_SIZE]) {
    //Leaving stub for external functions (serial and setup) that use this function
    byte scanAddr[TEMP_ADDR_SIZE];
    ds.reset_search();
    //Limit scan count in case the One Wire Search loop issue occurs
    for (byte limit = 0; limit <= MAX_SCAN_COUNT; ++limit) {
      if (!ds.search(scanAddr)) {
        //No Sensor found, Return
        ds.reset_search();
        break;
      }
      if (scanAddr[0] == DS18B20_IDENT || scanAddr[0] == DS18S20_IDENT) {
        bool found = false;
        for (byte i = 0; i <  NUM_TS; i++) {
          if (memcmp(scanAddr, tSensor[i], TEMP_ADDR_SIZE) == 0) {
            found = true;
            break;
          }
        }
        if (!found) {
          memcpy(addrRet, scanAddr, TEMP_ADDR_SIZE);
          return;
        }
      }
    }
    memset(addrRet, 0, TEMP_ADDR_SIZE);
  }

  //Returns Int representing hundreths of degree
  int readTemp(byte* addr) {
    #ifdef TS_ONEWIRE_I2C
      if (!masterIsConfigured) {
          tempInit(); // if the master isn't configured, try again
      }
      if (!masterIsConfigured) return BAD_TEMP; // if we still aren't setup, abort
    #endif
    long tempOut;
    byte data[SCRATCHPAD_SIZE];
    ds.reset();
    ds.select(addr);   
    ds.write(READ_SCRATCHPAD_CMD, TS_ONEWIRE_PPWR); //Read Scratchpad
    #ifdef TS_ONEWIRE_FASTREAD
      for (byte i = 0; i < 2; i++)
        data[i] = ds.read();
      if ((data[0] & data[1]) == 0xFF)
        return BAD_TEMP;
    #else
      for (byte i = 0; i < SCRATCHPAD_SIZE; i++) data[i] = ds.read();
      if (ds.crc8(data, CRC_SIZE) != data[CRC_INDEX]) return BAD_TEMP;
    #endif

    tempOut = (data[1] << 8) + data[0];
    
    if ( addr[0] == 0x10) tempOut = tempOut * 50; //9-bit DS18S20
    else tempOut = tempOut * 25 / 4; //12-bit DS18B20, etc.
      
    #ifdef USEMETRIC
      return int(tempOut);  
    #else
      return int((tempOut * 9 / 5) + 3200);
    #endif
  }

  void owTempInit()
  {
      for (byte i = 0; i < NUM_TS; i++) temp[i] = BAD_TEMP;
      #ifdef TS_ONEWIRE_I2C
        masterIsConfigured = ds.configure(dsConfig);
        if (masterIsConfigured) {
      #endif
          ds.reset();
          ds.skip();
          ds.write(WRITE_SCRATCHPAD_CMD, TS_ONEWIRE_PPWR); //Write to scratchpad
          ds.write(TH_REGISTER, TS_ONEWIRE_PPWR); //Default value of TH reg (user byte 1)
          ds.write(TL_REGISTER, TS_ONEWIRE_PPWR); //Default value of TL reg (user byte 2)
          ds.write(RES_VALUE, TS_ONEWIRE_PPWR);
          ds.reset();
          ds.skip();
          ds.write(COPY_SCRATCHPAD_CMD, TS_ONEWIRE_PPWR); //Copy scratchpad to EEPROM
      #ifdef TS_ONEWIRE_I2C
        }
      #endif
  }

  unsigned long convStart;

  void owTempUpdate()
  {
      #ifdef TS_ONEWIRE_I2C
        if (!masterIsConfigured) {
            tempInit(); // if the master isn't configured, try again
        }
        if (!masterIsConfigured) return; // if we still aren't setup, abort
      #endif
      if (convStart == 0) {
          ds.reset();
          ds.skip();
          ds.write(CONVERT_TEMP_CMD, TS_ONEWIRE_PPWR); //Start conversion
          convStart = millis();
      }
      else if (tsReady() || millis() - convStart >= CONV_DELAY) {
          for (byte i = 0; i < NUM_TS; i++) {
              if (validAddr(tSensor[i]))
                  temp[i] = readTemp(tSensor[i]);
              else
                  temp[i] = BAD_TEMP;
          }

          convStart = 0;

          #if defined MASH_AVG
            mashAvg();
          #endif
      }
    }
#else
  void owTempInit() {};
  void owTempUpdate() {};
  void getDSAddr(byte addrRet[TEMP_ADDR_SIZE]){};
#endif // TS_ONEWIRE

#if defined(TC74_BOARD_TEMP) && defined(TC74_I2C_ADDR)
  int getBoardTemp()
  {
      Wire.beginTransmission(TC74_I2C_ADDR);
      Wire.write(0x00);
      if (Wire.endTransmission() != 0) return BAD_TEMP;
      if (Wire.requestFrom(TC74_I2C_ADDR, 1) != 1) return BAD_TEMP;
      while (Wire.available() == 0) delay(1);
      long celsius = Wire.read() * 100;

      #ifdef USEMETRIC
        return int(celsius);
      #else
        return int((celsius * 9 / 5) + 3200);
      #endif
  }

  #if defined(MONITOR_BOARD_TEMP)
    // Averages 24 samples over 2:00 mins
    #define BOARD_SAMPLE_SIZE 24
    #define BOARD_SAMPLE_DURATION 5000
    int      samples[BOARD_SAMPLE_SIZE];
    byte     samplePos = 0;
    uint32_t lastBoardSample;
    uint32_t boardTempSum =0;
    int      avgBoardTemp = 0;

    void boardTempInit()
    {
        memset(samples, 0, sizeof(samples));
        lastBoardSample = millis();
    }

    void boardTempUpdate()
    {
        if (millis() - lastBoardSample >= BOARD_SAMPLE_DURATION)
        {
            boardTempSum -= samples[samplePos];
            samples[samplePos] = getBoardTemp();
            boardTempSum += samples[samplePos++];
            avgBoardTemp = int(boardTempSum / BOARD_SAMPLE_SIZE);
            if (samplePos == BOARD_SAMPLE_SIZE) samplePos = 0;
            lastBoardSample = millis();
        }
    }
    int getAvgBoardTemp() {
        return avgBoardTemp;
    }
  #else
    void boardTempInit()   {}
    void boardTempUpdate() {}
    int  getAvgBoardTemp() { return BAD_TEMP;  }
  #endif
#else
  int  getAvgBoardTemp() { return BAD_TEMP; }
  int  getBoardTemp() { return BAD_TEMP; }
  void boardTempInit() {}
  void boardTempUpdate() {}
#endif

void tempInit() {
    owTempInit();
    boardTempInit();
}

void updateTemps() {
    owTempUpdate();
    boardTempUpdate();
}

#if defined MASH_AVG
  void mashAvg() {
    byte sensorCount = 1;
    unsigned long avgTemp = temp[TS_MASH];
    #if defined MASH_AVG_AUX1
      if (temp[TS_AUX1] != BAD_TEMP) {
        avgTemp += temp[TS_AUX1];
        sensorCount++;
      }
    #endif
    #if defined MASH_AVG_AUX2
      if (temp[TS_AUX2] != BAD_TEMP) {
        avgTemp += temp[TS_AUX2];
        sensorCount++;
      }
    #endif
    #if defined MASH_AVG_AUX3
      if (temp[TS_AUX3] != BAD_TEMP) {
        avgTemp += temp[TS_AUX3];
        sensorCount++;
      }
    #endif
    temp[TS_MASH] = avgTemp / sensorCount;
  }
#endif // MASH_AVG