#ifndef OUTPUT_BANK_H
#include <pin.h>
#include "Config.h"
#include "HardwareProfile.h"
#include <ModbusMaster.h>
#include <HardwareSerial.h>
  
class OutputBank
{
public:
    OutputBank();
    virtual ~OutputBank() = default;

    virtual void     init(void) = 0;
    virtual void     set(uint32_t outBits) = 0;
    virtual uint32_t get() = 0;

protected:
    uint32_t outputBits;
};

class GPIOOutputBank : public OutputBank
{
public:
    PVOutGPIO(byte count);
    ~PVOutGPIO();

    void     setup(byte pinIndex, byte digitalPin);
    void     init(void);
    void     set(uint32_t outputBits);
    uint32_t get();

private:
    pin*     valvePin;
    byte     pinCount;
};
  
class MUXOutputBank : public OutputBank
{
public:
    PVOutMUX(byte latchPin, byte dataPin, byte clockPin, byte enablePin, boolean enableLogic);

    void     init(void);
    void     set(uint32_t outputBits);
    uint32_t get();

private:
    pin      latchPin;
    pin      dataPin;
    pin      clockPin;
    pin      enablePin;
    boolean  enableLogic;
};

//#ifdef PVOUT_TYPE_MODBUS
class MODBUSOutputBank : public OutputBank
{
public:
    PVOutMODBUS(uint8_t addr, unsigned int coilStart, uint8_t coilCount, uint8_t offset);

    void         init(void);
    void         set(uint32_t outputBits);
    uint32_t     get();

    byte         count();
    uint32_t     offset();
    byte         detect();
    byte         setAddr(byte newAddr);
    byte         setIDMode(byte value);
    byte         getIDMode();

private:
    ModbusMaster slave;
    byte         slaveAddr;
    byte         outputCount;
    byte         bitOffset;
    uint32_t     coilReg;
};
//#endif

#endif //ifndef PVOUT_H
