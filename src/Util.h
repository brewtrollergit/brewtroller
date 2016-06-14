#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>

void strLPad(char retString[], byte len, char pad);

void truncFloat(char retStr[], byte len);
void vftoa(unsigned long val, char retStr[], unsigned int divisor, boolean decimal);

unsigned long pow10(byte power);
unsigned long pow2(byte power);

#endif