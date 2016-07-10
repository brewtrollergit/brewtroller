#ifndef OUTPUT_BANK_H
#define OUTPUT_BANK_H

#include <pin.h>
#include "Config.h"
#include "HardwareProfile.h"
#include <ModbusMaster.h>
#include <HardwareSerial.h>
  
class OutputBank
{
public:
    OutputBank(byte size, byte bitPos);
    virtual ~OutputBank() = default;

    virtual void     init(void) = 0;
    virtual void     set(uint32_t outBits) = 0;
    virtual uint32_t get();
    
    byte             size() const;

    uint32_t         combineBits(uint32_t bits);
 
protected:
    uint32_t         computeBits(uint32_t);

    uint32_t         m_outputBits;
    byte             m_size;
    byte             m_bitPos;  
    uint32_t         m_mask;
};

class GPIOOutputBank : public OutputBank
{
public:
    GPIOOutputBank();
    ~GPIOOutputBank();

    void     setup(byte pinIndex, byte digitalPin);
    void     init(void);
    void     set(uint32_t outputBits);

private:
    pin*     valvePin;
    byte     pinCount;
};
  
class MUXOutputBank : public OutputBank
{
public:
    MUXOutputBank(byte latchPin, byte dataPin, byte clockPin, byte enablePin, boolean enableLogic);

    void     init(void);
    void     set(uint32_t outputBits);

private:
    pin      latchPin;
    pin      dataPin;
    pin      clockPin;
    pin      enablePin;
    boolean  enableLogic;
};

#ifdef PVOUT_TYPE_MODBUS
class MODBUSOutputBank : public OutputBank
{
public:
    MODBUSOutputBank(byte bitPos, uint8_t addr, unsigned int coilStart, uint8_t coilCount, uint8_t offset);

    void         init(void);
    void         set(uint32_t outputBits);
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
#endif

#endif //ifndef OUTPUT_BANK_H
