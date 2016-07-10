#include <Arduino.h>
#include <pin.h>
#include "Config.h"
#include "HardwareProfile.h"
#include <ModbusMaster.h>
#include <HardwareSerial.h>
#include "PVOut.h"
#include "Util.h"

OutputBank::OutputBank(byte size, byte bitPos)
    : m_outputBits(0), m_size(size), m_bitPos(bitPos)
{
    m_mask = pow2(size)-1;
}

byte OutputBank::size() const
{
    return m_size;
}

uint32_t OutputBank::get()
{
    return m_outputBits;
}

uint32_t OutputBank::computeBits(uint32_t bits)
{
    return (bits >> m_bitPos) & m_mask;
}

uint32_t OutputBank::combineBits(uint32_t bits)
{
    return bits | (m_outputBits << m_bitPos);
}

// ---------------------- GPIOOutputBank ----------------------

GPIOOutputBank::GPIOOutputBank() 
    : OutputBank(PVOUT_BUILTIN_COUNT, 0)
{
    pinCount = PVOUT_COUNT;
    valvePin = (pin *) malloc(pinCount * sizeof(pin));
}

GPIOOutputBank::~GPIOOutputBank() {
    free(valvePin);
}

void GPIOOutputBank::setup(byte pinIndex, byte digitalPin) {
    valvePin[pinIndex].setup(digitalPin, OUTPUT);
}

void GPIOOutputBank::init(void) {
#if PVOUT_COUNT >= 1 && defined(VALVE1_PIN)
    setup(0, VALVE1_PIN);
#endif
#if PVOUT_COUNT >= 2 && defined(VALVE2_PIN)
    .setup(1, VALVE2_PIN);
#endif
#if PVOUT_COUNT >= 3 && defined(VALVE3_PIN)
    setup(2, VALVE3_PIN);
#endif
#if PVOUT_COUNT >= 4 && defined(VALVE4_PIN)
    setup(3, VALVE4_PIN);
#endif
#if PVOUT_COUNT >= 5 && defined(VALVE5_PIN)
    setup(4, VALVE5_PIN);
#endif
#if PVOUT_COUNT >= 6 && defined(VALVE6_PIN)
    setup(5, VALVE6_PIN);
#endif
#if PVOUT_COUNT >= 7 && defined(VALVE7_PIN)
    setup(6, VALVE7_PIN);
#endif
#if PVOUT_COUNT >= 8 && defined(VALVE8_PIN)
    setup(7, VALVE8_PIN);
#endif
#if PVOUT_COUNT >= 9 && defined(VALVE9_PIN)
    setup(8, VALVE9_PIN);
#endif
#if PVOUT_COUNT >= 10 && defined(VALVEA_PIN)
    setup(9, VALVEA_PIN);
#endif
#if PVOUT_COUNT >= 11 && defined(VALVEB_PIN)
    setup(10, VALVEB_PIN);
#endif
#if PVOUT_COUNT >= 12 && defined(VALVEC_PIN)
    setup(11, VALVEC_PIN);
#endif

    set(0);
}

void GPIOOutputBank::set(uint32_t outputBits) {
    outputBits = computeBits(outputBits);
    for (byte i = 0; i < pinCount; i++) {
        if (outputBits & (1<<i))
            valvePin[i].set(); 
        else 
            valvePin[i].clear();
    }
    m_outputBits = outputBits;
}

// ---------------------- MUXOutputBank ----------------------

MUXOutputBank::MUXOutputBank(byte latchPin, byte dataPin, byte clockPin, byte enablePin, boolean enableLogic)
    : OutputBank(PVOUT_BUILTIN_COUNT, 0)
{
    this->latchPin.setup(latchPin, OUTPUT);
    this->dataPin.setup(dataPin, OUTPUT);
    this->clockPin.setup(clockPin, OUTPUT);
    this->enablePin.setup(enablePin, OUTPUT);
    this->enableLogic = enableLogic;
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
    outputBits = computeBits(outputBits);

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
    this->m_outputBits = outputBits;
}

// ---------------------- MUXOutputBank ----------------------

#ifdef PVOUT_TYPE_MODBUS
MODBUSOutputBank::MODBUSOutputBank(byte bitPos, uint8_t addr, unsigned int coilStart, uint8_t coilCount, uint8_t offset)
    : OutputBank(coilCount, bitPos)
{
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
    outputBits = computeBits(outputBits);

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
    this->m_outputBits = outputBits;
}

uint32_t MODBUSOutputBank::offset() {
    return bitOffset;
}

byte MODBUSOutputBank::detect() {
    return slave.readCoils(coilReg, outputCount);
}
byte MODBUSOutputBank::setAddr(byte newAddr) {
    byte result = 0;
    result |= slave.writeSingleRegister(MODBUS_RELAY_REGSLAVEADDR, newAddr);
    if (!result) {
        slave.writeSingleRegister(MODBUS_RELAY_REGRESTART, 1);
        slaveAddr = newAddr;
    }
    return result;
}
byte MODBUSOutputBank::setIDMode(byte value) {
    return slave.writeSingleRegister(MODBUS_RELAY_REGIDMODE, value);
}

byte MODBUSOutputBank::getIDMode() {
    if (slave.readHoldingRegisters(MODBUS_RELAY_REGIDMODE, 1) == 0)
        return slave.getResponseBuffer(0);
    return 0;
}
#endif