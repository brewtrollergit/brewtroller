#ifndef COM_BTPD_H
#define COM_BTPD_H

#ifdef BTPD_SUPPORT

enum PDType
{
  PDT_NONE = 1,     // Nothing
  PDT_TEMP,         // Temperature
  PDT_TEMPSP,       // Temperature SetPoint
  PDT_VOLUME,       // Volume Level
  PDT_VOLUMESP,     // Average Volume Level
  PDT_FIXED,        // Fixed User Defined Value
#ifdef BTPD_TIMERS
  PDT_TIMER = 13,   // Timer value
#endif

};

enum PDPos
{
  PDL_TOP    = 0xfd, // Top Line
  PDL_BOTTOM = 0xfe  // Bottom Line
};

// Called by updateCom() in Com.h to update the displays
void updateBTPD();

// Can be called to change what is currently being displayed
//
// @display   Base 1 display number (1-BTPD_COUNT)
// @type      Type of indormation to displayu
// @position  Which line Top/Bottom to change
// @ref       Index needed by temp, volume, and fixed types
void changePIDDisplay(byte display, PDType type, PDPos position, int ref);

#endif // BTPD_SUPPORT

#endif // COM_BTPD_H
