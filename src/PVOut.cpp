#include <Arduino.h>
#include <pin.h>
#include "Config.h"
#include "HardwareProfile.h"
#include <ModbusMaster.h>
#include <HardwareSerial.h>
#include "PVOut.h"

// ---------------------- GPIOOutputBank ----------------------

GPIOOutputBank::GPIOOutputBank(byte count) {
    pinCount = count;
    valvePin = (pin *) malloc(pinCount * sizeof(pin));
}

GPIOOutputBank::~GPIOOutputBank() {
    free(valvePin);
}

void GPIOOutputBank::setup(byte pinIndex, byte digitalPin) {
    valvePin[pinIndex].setup(digitalPin, OUTPUT);
}

void GPIOOutputBank::init(void) {
    set(0);
}

void GPIOOutputBank::set(uint32_t outputBits) {
    for (byte i = 0; i < pinCount; i++) {
        if (outputBits & (1<<i))
            valvePin[i].set(); 
        else 
            valvePin[i].clear();
    }
    this->outputBits = outputBits;
}

uint32_t GPIOOutputBank::get() {
    return outputBits;
}

// ---------------------- MUXOutputBank ----------------------

MUXOutputBank::MUXOutputBank(byte latchPin, byte dataPin, byte clockPin, byte enablePin, boolean enableLogic) {
    latchPin.setup(latchPin, OUTPUT);
    dataPin.setup(dataPin, OUTPUT);
    clockPin.setup(clockPin, OUTPUT);
    enablePin.setup(enablePin, OUTPUT);
    enableLogic = enableLogic;
}

void MUXOutputBank::init(void) {
    if (enableLogic) {
        //MUX in Reset State
        latchPin.clear(); //Prepare to copy pin states
        enablePin.clear(); //Force clear of pin registers
        latchPin.set();
        delayMicroseconds(10);
        latchPin.clear();
        enablePin.set(); //Disable clear
    } else {
        set(0);
        enablePin.clear();
    }
}

void MUXOutputBank::set(uint32_t outputBits) {
    //ground latchPin and hold low for as long as you are transmitting
    latchPin.clear();
    //clear everything out just in case to prepare shift register for bit shifting
    dataPin.clear();
    clockPin.clear();

    //for each bit in the long myDataOut
    for (byte i = 0; i < 32; i++)  {
        clockPin.clear();
        //create bitmask to grab the bit associated with our counter i and set data pin accordingly (NOTE: 32 - i causes bits to be sent most significant to least significant)
        if (outputBits & ((uint32_t)1<<(31 - i)) )
            dataPin.set(); 
        else 
            dataPin.clear();
        //register shifts bits on upstroke of clock pin
        clockPin.set();
        //zero the data pin after shift to prevent bleed through
        dataPin.clear();
    }

    //stop shifting
    clockPin.clear();
    latchPin.set();
    delayMicroseconds(10);
    latchPin.clear();
    this->outputBits = outputBits;
}

uint32_t MUXOutputBank::get() {
    return outputBits;
}

// ---------------------- MUXOutputBank ----------------------

//#ifdef PVOUT_TYPE_MODBUS
MODBUSOutputBank::MODBUSOutputBank(uint8_t addr, unsigned int coilStart, uint8_t coilCount, uint8_t offset) {
    slaveAddr = addr;
    slave = ModbusMaster(RS485_SERIAL_PORT, slaveAddr);
#ifdef RS485_RTS_PIN
    slave.setupRTS(RS485_RTS_PIN);
#endif
    slave.begin(RS485_BAUDRATE, RS485_PARITY);
   
    //Modbus Coil Register index starts at 1 but is transmitted with a 0 index
    coilReg = coilStart - 1;
    outputCount = coilCount;
    bitOffset = offset;
}

void MODBUSOutputBank::init(void) {
    set(0);
}

void MODBUSOutputBank::set(uint32_t outputBits) {
    byte outputPos = 0;
    byte bytePos = 0;
    while (outputPos < outputCount) {
        byte byteData = 0;
        byte bitPos = 0;
        while (outputPos < outputCount && bitPos < 8)
            bitWrite(byteData, bitPos++, (outputBits >> outputPos++) & 1);
        slave.setTransmitBuffer(bytePos++, byteData);
    }
    slave.writeMultipleCoils(coilReg, outputCount);
    this->outputBits = outputBits;
}

uint32_t MODBUSOutputBank::get() {
    return outputBits;
}

byte MODBUSOutputBank::count() {
    return outputCount;
}

uint32_t MODBUSOutputBank::offset() {
    return bitOffset;
}

byte MODBUSOutputBank::detect() {
    return slave.readCoils(coilReg, outputCount);
}
byte MODBUSOutputBank::setAddr(byte newAddr) {
    byte result = 0;
    result |= slave.writeSingleRegister(PVOUT_MODBUS_REGSLAVEADDR, newAddr);
    if (!result) {
        slave.writeSingleRegister(PVOUT_MODBUS_REGRESTART, 1);
        slaveAddr = newAddr;
    }
    return result;
}
byte MODBUSOutputBank::setIDMode(byte value) {
    return slave.writeSingleRegister(PVOUT_MODBUS_REGIDMODE, value);
}

byte MODBUSOutputBank::getIDMode() {
    if (slave.readHoldingRegisters(PVOUT_MODBUS_REGIDMODE, 1) == 0)
        return slave.getResponseBuffer(0);
    return 0;
}
//#endif