#ifndef TEMP_H
#define TEMP_H

#include <Arduino.h>
#include "HardwareProfile.h"

#define TEMP_ADDR_SIZE  8     // Number of bytes in an address (i.e. 64 bit)

void tempInit();
void updateTemps();
//This function search for an address that is not currently assigned!
void getDSAddr(byte addrRet[TEMP_ADDR_SIZE]);

#endif