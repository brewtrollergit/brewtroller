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


//*****************************************************************************************************************************
// Special thanks to Jason von Nieda (vonnieda) for the design and code for this cool add-on to BrewTroller.
//*****************************************************************************************************************************

#include <Arduino.h>
#include <stdio.h>
#include "BrewTroller.h"
#include <Wire.h>
#include "HardwareProfile.h"
#include "Com_BTPD.h"
#include "Config.h"

#ifdef BTPD_SUPPORT

#define BTPD_TOP_LINE_INDEX 0
#define BTPD_BOT_LINE_INDEX 1

struct DisplayLine
{
    PDType type;
    int    ref;
};

struct PIDDisplay
{
    byte        address;
    DisplayLine lines[2];
};

unsigned long lastBTPD;

#ifdef BTPD_TIMERS
char lineBuf[11] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
#endif

float getFloat(DisplayLine& line);
void  sendFloatBTPD(byte chan, byte position, float line);
void  sendFloatsBTPD(byte chan, float line1, float line2);
#ifdef BTPD_TIMERS
void  sendStringBTPD(byte chan, const char *string, int length);
char* setFloatStr(char* buf, DisplayLine& line);
char* setTimeStr(char* buf, byte timer);
void  convertTimeSegment(char* buf, byte val);
void  sendTimeBTPD(byte chan, byte timer);
#endif

//************************************ Custimization Instructions ************************************
// To Customize what is displayed on your PID displays, update the displays structure below. Each
// PIDDIsplay has a I2C address field to identify the board and two DisplayLine types that will control what
// is presented on each line of the display. The first DisplayLine represents the top line and the
// second the bottom line. Each DisplayLine needs a type of display (see PDType in header) set and a
// reference value that will depend on what type was chosen. The syntax for filling the
// display variable below is:
//
//   {
//    [i2c address of the board],
//    {
//        { [top    line type], [top    line type refrence] },
//        { [bottom line type], [bottom line type refrence] },
//    }
//  },
//
// The default will display the setpoint (top line) and temp (bottom line) for the HLT, MASH, and KETTLE
// temp sensors depending on how many PID displays are enabled
//
// There seven types of things that can be displayed:
// 1. PDT_NONE:     Displays four dashes
// 2. PDT_TEMP:     Temperature for the sensor index stored in ref. If the value is invalid you will also see
//                  five dashes
// 3. PDT_TEMPSP:   Temperature setpoint for the sensor index stored in ref
// 4. PDT_VOLUME:   Volume for the vessel index stored in ref
// 5. PDT_VOLUMESP: Volume target for the vessel index stored in ref
// 6. PDT_TIMER:    Current timer count for timer index stored in ref (only if BTPD_TIMERS is defined)
// 7. PDT_FIXED:    Value stored in ref / 100.0
//
//****************************************************************************************************

PIDDisplay displays[BTPD_COUNT] = {
    // Board 1 - HLT SetPoint/Temp
    {
        0x20,
        {
            { PDT_TEMPSP, TIMER_MASH },
            { PDT_TEMP,   TS_HLT }
        }
    },
#if BTPD_COUNT > 1
    // Board 2 - Mash SetPoint/Temp
    {
        0x21,
        {
            { PDT_TEMPSP, TS_MASH },
            { PDT_TEMP,   TS_MASH }
        }
    },
#endif
#if BTPD_COUNT > 2
    // Board 3 - Kettle SetPount/Temp
    {
        0x23,
        {
            { PDT_TEMPSP, TS_KETTLE },
            { PDT_TEMP,   TS_KETTLE }
        }
    },
#endif
};

//****************************************************************************************************


// The PID displays support three ways to change values:
// 1. Using 4 byte big endian double (Called BT mode)
// 2. Using 2 byte big endian int16 that is divided by 10.0 (Called BCS Mode)
// 3. Using an 8-10 byte string
//
// The first two options allow you to address either the top line, bottom line, or both lines from one
// command. Thus the command will either have 8 bytes or 4 bytes depending on mode.  Using the string
// mode, is the only way to get a ':' into the display for showing time.
//
// In order to support addressing the displays per line versus per display, this gets a little tricky
// when one of the lines needs to show a timer. Because of this the code below is a little more complicated
// than it probably should be, adding three cases instead of one, but this still works very well. When
// BTPD_TIMERS is disabled, this complexity goes away and the default case can be used.
//
// The tricky part is in the third case when a timer is chosen with another line that needs a float. In
// This case the float needs to be converted to a string and passed with the single string command so that
// the display can be updated properly. To do this snprintf is used which adds about 3-4K to the final HEX
// size
//
// Short of requiring everyone to change their PID display code, which isn't easly changable for the average
// user, we could use a period instead of a colon and this code could be reverted to the default case with
// only minor changes to the time creation code.

void updateBTPD() {
    byte chan;

    if (millis() - lastBTPD > BTPD_INTERVAL) {
        for (int x = 0; x < BTPD_COUNT; ++x) {
            chan = displays[x].address;
#ifndef BTPD_TIMERS
            sendFloatsBTPD(chan, getFloat(displays[x].lines[0]), getFloat(displays[x].lines[1]));
#else
            byte tsum = displays[x].lines[0].type + displays[x].lines[1].type;

            if (tsum < PDT_TIMER) // Two floats
            {
                sendFloatsBTPD(chan, getFloat(displays[x].lines[0]), getFloat(displays[x].lines[1]));
            }
            else if (tsum == 2 * PDT_TIMER) // Two timer strings
            {
                setTimeStr(lineBuf, displays[x].lines[0].ref);
                setTimeStr(lineBuf + 5, displays[x].lines[1].ref);
                sendStringBTPD(displays[x].address, lineBuf, 10);
            }
            else // Timer and something else
            {
                char *pos;
                if (displays[x].lines[0].type == PDT_TIMER)
                    pos = setTimeStr(lineBuf, displays[x].lines[0].ref);
                else
                    pos = setFloatStr(lineBuf, displays[x].lines[0]);

                if (displays[x].lines[1].type == PDT_TIMER)
                    pos = setTimeStr(pos, displays[x].lines[1].ref);
                else
                    pos = setFloatStr(pos, displays[x].lines[1]);

                //Length can vary, so set null terminator so we can use strlen
                *pos = 0;

                sendStringBTPD(displays[x].address, lineBuf, strlen(lineBuf));
            }
#endif
        }
        lastBTPD = millis();
    }
}

void changePIDDisplay(byte display, PDType type, PDPos position, int ref)
{
    if (display == 0 || display > BTPD_COUNT) return;

    DisplayLine* line;

    if (position == PDL_TOP) line = &(displays[display - 1].lines[BTPD_TOP_LINE_INDEX]);
    else line = &(displays[display - 1].lines[BTPD_BOT_LINE_INDEX]);

    line->type = type;
    line->ref = ref;
}

float getFloat(DisplayLine& line)
{
    switch (line.type)
    {
    case PDT_TEMP:
        if (temp[line.ref] == BAD_TEMP) {
            return -1.0;
        }
        else {
            return temp[line.ref] / 100.0;
        }
        break;
    case PDT_TEMPSP:
        return setpoint[line.ref] / 100.0;
    case PDT_VOLUMESP:
        return tgtVol[line.ref] / 100.0;
    case PDT_VOLUME:
        return volAvg[line.ref] / 1000.0;
        break;
    case PDT_FIXED:
        return line.ref / 100.0;
    case PDT_NONE:
    default:
        return -1.0;
    }
}

void sendFloatBTPD(byte chan, byte position, float line) {
    Wire.beginTransmission(chan);
    Wire.write(position);
    Wire.write(0x00);
    Wire.write((uint8_t *)&line, 4);
    Wire.endTransmission();
}

void sendFloatsBTPD(byte chan, float line1, float line2) {
    Wire.beginTransmission(chan);
    Wire.write(0xff);
    Wire.write(0x00);
    Wire.write((uint8_t *)&line1, 4);
    Wire.write((uint8_t *)&line2, 4);
    Wire.endTransmission();
}

#ifdef BTPD_TIMERS
void sendStringBTPD(byte chan, const char *string, int len) {
    Wire.beginTransmission(chan);
    Wire.write((uint8_t *)string, len);
    Wire.endTransmission();
}

void convertTimeSegment(char* buf, byte val)
{
    if (val > 99)
    {
        *buf++ = '-';
        *buf++ = '-';
    }
    else if (val < 10)
    {
        *buf++ = '0';
        *buf++ = val + 48;
    }
    else
    {
        *buf++ = (val / 10) + 48;
        *buf++ = (val % 10) + 48;
    }
}

// Converts a line value to a string based on the types
char* setFloatStr(char* buf, DisplayLine& line)
{
    double d = (double)getFloat(line);

    if (d < 0.0)
    {
        memset(buf, '-', 4);
        return buf + 4;
    }
    else
    {
        return buf + sprintf_P(buf, PSTR("%5.1f"), d);
    }
}

// The time string will be formatted in one of 3 ways
// 1. "--:--" when the timer is paused, not running, greater than the minimum amount, or the 
//    referenced timer is doesn't exist
// 2. "HH:MM" when the timer >= one hour
// 3. "MM:SS" when the timer < one hour
//
// The maximum valu eofr each field is 99:59 Anything outside of this will result in an empty
// timer value on the PID display
char* setTimeStr(char* buf, byte timer) {
    byte AA = 0;
    byte BB = 0;

    // Force empty time on edge cases
    if ((timer > NUM_TIMERS - 1) || (timerValue[timer] > 0 && !timerStatus[timer])) {
        AA = 100;
        BB = 100;
    }
    else if (alarmStatus || timerStatus[timer]) {
        byte hours1 = timerValue[timer] / 3600000;
        byte mins1 = (timerValue[timer] - hours1 * 3600000) / 60000;
        byte secs1 = (timerValue[timer] - hours1 * 3600000 - mins1 * 60000) / 1000;
        if (hours1 > 0) {
            AA = hours1;
            BB = mins1;
        }
        else {
            AA = mins1;
            BB = secs1;
        }
    }

    convertTimeSegment(buf, AA);
    *(buf+2) = ':';
    convertTimeSegment(buf+3, BB);

    // Time string is always 5 bytes
    return buf + 5;
}

#endif //BTPD_TIMERS
#endif //BTPD_SUPPORT
