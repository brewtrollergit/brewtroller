/*
BrewTroller Phoenix Single Vessel Hardware Configuration
*/

#ifndef __BREWTROLLER_HWPROFILE__
    #error "Include HardwareProfile.h instead of this file"
#endif

#ifndef BT_HWPROFILE
#define BT_HWPROFILE

  #define ENCODER_I2C
  #define ENCODER_I2CADDR 0x01

  #define ALARM_PIN 15 //OUT4
  
  #define PVOUT
  #define PVOUT_TYPE_MUX
  #define PVOUT_TYPE_MODBUS
  #define PVOUT_BUILTIN_COUNT 16 // # of onboard outputs

  const uint8_t PVOUT_COUNT = PVOUT_BUILTIN_COUNT + (NUM_MODBUS_RELAY_BOARDS*MODBUS_RELAY_DEFCOILCOUNT);  // Total # of outputs - Outputs used for heat + Modbus relay outputs

  #define MUX_LATCH_PIN 3
  #define MUX_CLOCK_PIN 4
  #define MUX_DATA_PIN 1
  #define MUX_ENABLE_PIN 2
  #define MUX_ENABLE_LOGIC 0
  
  #define HLTHEAT_PIN 12  //Output Terminal 1

  #define DIGITAL_INPUTS
  #define DIGIN_COUNT 9
  #define DIGIN1_PIN 31
  #define DIGIN2_PIN 30
  #define DIGIN3_PIN 29
  #define DIGIN4_PIN 28
  #define DIGIN5_PIN 18
  #define DIGIN6_PIN 19
  #define DIGIN7_PIN 20
  #define DIGIN8_PIN 21
  #define DIGIN9_PIN 22 //estop

  #define RS485_SERIAL_PORT 1
  #define RS485_RTS_PIN    23
  
  #define HLTVOL_APIN 7
  #define MASHVOL_APIN 6
  #define KETTLEVOL_APIN 5
  #define STEAMPRESS_APIN 4
  
  #define UI_LCD_I2C
  #define UI_LCD_I2CADDR 0x01
  #define UI_DISPLAY_SETUP
  #define LCD_DEFAULT_CONTRAST 100
  #define LCD_DEFAULT_BRIGHTNESS 255
  
  // BTPD_SUPPORT: Enables use of BrewTroller PID Display devices on I2C bus
  #define BTPD_SUPPORT
  
  // BTNIC_EMBEDDED: Enables use of on-board I2C Ethernet module
  #define BTNIC_EMBEDDED
  
  #define HEARTBEAT
  #define HEARTBEAT_PIN 0

  //**********************************************************************************
  // OneWire Temperature Sensor Options
  //**********************************************************************************
  // TS_ONEWIRE: Enables use of OneWire Temperature Sensors
  // TS_ONEWIRE_I2C: Enables use of DS2482 I2C 1-Wire Bus Master
  #define TS_ONEWIRE
  #define TS_ONEWIRE_I2C
  
  // TS_ONEWIRE_PPWR: Specifies whether parasite power is used for OneWire temperature
  // sensors. Parasite power allows sensors to obtain their power from the data line
  // but significantly increases the time required to read the temperature (94-750ms
  // based on resolution versus 10ms with dedicated power).
  #define TS_ONEWIRE_PPWR 0
  
  // TS_ONEWIRE_RES: OneWire Temperature Sensor Resolution (9-bit - 12-bit). Valid
  // options are: 9, 10, 11, 12). Unless parasite power is being used the recommended
  // setting is 12-bit (for DS18B20 sensors). DS18S20 sensors can only operate at a max
  // of 9 bit. When using parasite power decreasing the resolution reduces the 
  // temperature conversion time: 
  //   12-bit (0.0625C / 0.1125F) = 750ms 
  //   11-bit (0.125C  / 0.225F ) = 375ms 
  //   10-bit (0.25C   / 0.45F  ) = 188ms 
  //    9-bit (0.5C    / 0.9F   ) =  94ms   
  #define TS_ONEWIRE_RES 12
  
  // TS_ONEWIRE_FASTREAD: Enables faster reads of temperatures by reading only the first
  // 2 bytes of temperature data and ignoring CRC check.
  #define TS_ONEWIRE_FASTREAD
  
  // DS2482_ADDR: I2C Address of DS2482 OneWire Master (used for TS_OneWire_I2C)
  // Should be 0x18, 0x19, 0x1A, 0x1B
  #define DS2482_ADDR 0x1B
  //**********************************************************************************

  //  Enables lookup of the board temperature
  #define TC74_BOARD_TEMP
  #define TC74_I2C_ADDR 0x48
  // Enable to track and monitor board temperature
  #define MONITOR_BOARD_TEMP

  //**********************************************************************************
  // Serial0 Communication Options
  //**********************************************************************************
  // COM_SERIAL0: Specifies the communication type being used (Pick One):
  //  ASCII  = Original BrewTroller serial command protocol used with BTRemote and BTLog
  //  BTNIC  = BTnic (Lighterweight implementation of ASCII protocol using single-byte
  //           commands. This protocol is used with BTnic Modules and software for
  //           network connectivity.
  //  BINARY = Binary Messages
  //**********************************************************************************
  
  //#define COM_SERIAL0  ASCII
  #define COM_SERIAL0  BTNIC
  //#define COM_SERIAL0 BINARY
  
  // BAUD_RATE: The baud rate for the Serial0 connection. Previous to BrewTroller 2.0
  // Build 419 this was hard coded to 9600. Starting with Build 419 the default rate
  // was increased to 115200 but can be manually set using this compile option.
  #define SERIAL0_BAUDRATE 115200

#endif


