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

void strLPad(char retString[], byte len, char pad) {
  char strVal[len + 1];
  strcpy(strVal, retString);
  memset(retString, pad, len);
  retString[len - strlen(strVal)] = '\0';
  strcat(retString, strVal);
}

//Converts a "Virtual Float" (fixed decimal value represented in tenths, hundredths, thousandths, etc.) to a string
void vftoa(unsigned long val, char retStr[], unsigned int divisor, boolean decimal) {
  char lbuf[11];
  itoa(divisor - 1, lbuf, 10);
  byte precision = strlen(lbuf);
  if (divisor == 1) precision = 0;
  unsigned long whole = val / divisor;
  ultoa(whole, retStr, 10);
  if (precision) {
    if (decimal) strcat(retStr, ".");
    ultoa((val - whole * divisor) * round((float)pow(10, precision) / divisor), lbuf, 10);
    strLPad(lbuf, precision, '0');
    strcat(retStr, lbuf);
  }
}

//Truncate a string representation of a float to (length) chars but do not end string with a decimal point
void truncFloat(char retStr[], byte len) {
  retStr[len] = '\0';
  if (retStr[len - 1] == '.') retStr[len - 1] = '\0';
}

unsigned long pow10(byte power) {
  unsigned long retValue = 1;
  for (byte i = 0; i < power; i++) retValue *= 10;
  return retValue;
}

unsigned long pow2(byte power) {
    unsigned long retValue = 1;
    for (byte i = 0; i < power; i++) retValue *= 2;
    return retValue;
}
