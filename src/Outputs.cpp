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
#include <pin.h>
#include "HardwareProfile.h"
#include "Enum.h"
#include "BrewTroller.h"
#include "EEPROM.h"
#include "Outputs.h"
#include "Events.h"
#include "Com_RGBIO8.h"

extern const int HEAT_OUTPUTS_COUNT;

#ifdef PID_FLOW_CONTROL 
  #define LAST_HEAT_OUTPUT VS_PUMP // not this is mostly done for code readability as VS_PUMP = VS_STEAM
#else
  #ifdef USESTEAM
    #define LAST_HEAT_OUTPUT VS_STEAM
  #elif defined DIRECT_FIRED_RIMS
    #define LAST_HEAT_OUTPUT VS_STEAM
  #else
    #define LAST_HEAT_OUTPUT VS_KETTLE
  #endif
#endif


// set what the PID cycle time should be based on how fast the temp sensors will respond
#if TS_ONEWIRE_RES == 12
  #define PID_CYCLE_TIME 750
#elif TS_ONEWIRE_RES == 11
  #define PID_CYCLE_TIME 375
#elif TS_ONEWIRE_RES == 10
  #define PID_CYCLE_TIME 188
#elif TS_ONEWIRE_RES == 9
  #define PID_CYCLE_TIME 94
#else
  // should not be this value, fail the compile
  #ERROR
#endif

#ifdef PWM_BY_TIMER
// note there are some assumptions here, we assume that the COM1A1, COM1B1, COM1A0, and COM1B0 
// bits are all 0 (as they should be on power up)
void pwmInit( void )
{
    // set timer 1 prescale factor to 0
    sbi(TCCR1B, CS10);
    cbi(TCCR1B, CS12);
    cbi(TCCR1B, CS11);

    //clear timer 1 out of 8 bit phase correct PWM mode from sanguino init
    cbi(TCCR1A, WGM10);
    //set timer 1 into 16 bit phase and frequency correct PWM mode with ICR1 as TOP
    sbi(TCCR1B, WGM13);
    //set TOP as 1000, which makes the overflow on return to bottom for this mode happen ever 
    // 125uS given a 16mhz input clock, aka 8khz PWM frequency, the overflow ISR will handle 
    // the PWM outputs that are slower than 8khz, and the OCR1A/B ISR will handle the 8khz PWM outputs
    ICR1 = 1000; 

    //enable timer 1 overflow interrupt (in this mode overflow happens when the timer counds down to BOTTOM
    // after counting UP from BOTTOM to TOP. 
    sbi(TIMSK1, TOIE1);

}

//note that the code in any SIGNAL function is an ISR, and the code needs to kept short and fast
// it is important to avoid divides by non power of 2 numbers, remainder (mod) calculations, wait loops,
// or calls to functions that have wait loops. It's also not a good idea to write into any global that may be 
// used else where in the code inside here without interrupt protecting all accesses to that variable in 
// non ISR code, or making sure that if we do write to it in the ISR, we dont write/read to it in non ISR code
// (for example, below the heatPin objects are not written to if PIDEnable[i] = 1;
//
// Also the below ISR is set to nonblock so that interrupts are enabled as we enter the function
// this is done to make sure that we can run low counts in the compare registers, for example, 
// a count of 1 could cause an interrupts 1 processor clock cycle after this interrupt is called 
// sense it's called at bottom, and sense this has a fair amount of code in it, it's good to let the 
// compare interrupts interrupt this interrupt (same with the UART and timer0 interrupts)
ISR(TIMER1_OVF_vect, ISR_NOBLOCK )
{
    //count the number of times this has been called 
    timer1_overflow_count++;
    for(byte i = 0; i <= LAST_HEAT_OUTPUT; i++)
    {
        // if PID is enabled, and NOT one of the 8khz PWM outputs then we can use this
        if(PIDEnabled[i])
        {
            //init the cyclestart counter if needed
            if(cycleStart[i] == 0 ) cycleStart[i] = timer1_overflow_count; 
            //if our period just ended, update to when the next period ends
            if((timer1_overflow_count - cycleStart[i]) > PIDOutputCountEquivalent[i][0]) 
                cycleStart[i] += PIDOutputCountEquivalent[i][0];
            //check to see if the pin should be high or low (note when our 16 bit integer wraps we will have 1 period where 
            // the PWM % if cut short, because from the time of wrap until the next period 
            if ((!estop) && PIDOutputCountEquivalent[i][1] >= timer1_overflow_count - cycleStart[i] 
                  && timer1_overflow_count != cycleStart[i]) 
                heatPin[i].set(HIGH); else heatPin[i].set(LOW);
        }
    }
}

#endif //PWM_BY_TIMER


void pinInit() {
  #ifdef ALARM_PIN
    alarmPin.setup(ALARM_PIN, OUTPUT);
  #endif
  //Setup HLT Pin
  #ifdef HLTHEAT_PIN
    heatPin[VS_HLT].setup(HLTHEAT_PIN, OUTPUT);
  #endif

  #ifdef SINGLE_VESSEL_SUPPORT
    //Single Vessel: Also setup HLT pin for Mash/Kettle
    #ifdef HLTHEAT_PIN
      heatPin[VS_MASH].setup(HLTHEAT_PIN, OUTPUT);
      heatPin[VS_KETTLE].setup(HLTHEAT_PIN, OUTPUT);
    #endif
  #else
    //Setup Mash/Kettle Vessel Heat Pins for non-Single Vessel
    #ifdef KETTLE_AS_MASH
      //Kettle as Mash option
      #ifdef KETTLEHEAT_PIN
        heatPin[VS_MASH].setup(KETTLEHEAT_PIN, OUTPUT);
      #endif
    #else
      //Normal Mash Vessel
      #ifdef MASHHEAT_PIN
        heatPin[VS_MASH].setup(MASHHEAT_PIN, OUTPUT);
      #endif
    #endif
    #ifdef HLT_AS_KETTLE
      //HLT as Kettle Option
      #ifdef HLTHEAT_PIN
        heatPin[VS_KETTLE].setup(HLTHEAT_PIN, OUTPUT);
      #endif      
    #else
      //Normal Kettle
      #ifdef KETTLEHEAT_PIN
        heatPin[VS_KETTLE].setup(KETTLEHEAT_PIN, OUTPUT);
      #endif
    #endif
  #endif
  
  #ifdef DIRECT_FIRED_RIMS
    //MASHHEAT_PIN should be defined, so setup above.
    heatPin[VS_STEAM].setup(STEAMHEAT_PIN, OUTPUT);
  #endif

  #ifdef USESTEAM
    #ifdef STEAMHEAT_PIN
      heatPin[VS_STEAM].setup(STEAMHEAT_PIN, OUTPUT);
    #endif
  #endif
  #ifdef PID_FLOW_CONTROL
    #ifdef PWMPUMP_PIN
      heatPin[VS_PUMP].setup(PWMPUMP_PIN, OUTPUT);
    #endif
  #endif

  #ifdef HEARTBEAT
    hbPin.setup(HEARTBEAT_PIN, OUTPUT);
  #endif
  
  #ifdef DIGITAL_INPUTS
    #if DIGIN_COUNT > 0
      digInPin[0].setup(DIGIN1_PIN, INPUT);
    #endif
    #if DIGIN_COUNT > 1
      digInPin[1].setup(DIGIN2_PIN, INPUT);
    #endif
    #if DIGIN_COUNT > 2
      digInPin[2].setup(DIGIN3_PIN, INPUT);
    #endif
    #if DIGIN_COUNT > 3
      digInPin[3].setup(DIGIN4_PIN, INPUT);
    #endif
    #if DIGIN_COUNT > 4
      digInPin[4].setup(DIGIN5_PIN, INPUT);
    #endif
    #if DIGIN_COUNT > 5
      digInPin[5].setup(DIGIN6_PIN, INPUT);
    #endif
  #endif
}

void pidInit() {
  //note that the PIDCycle for the 8khz outputs is set to 10 because the TOP of the counter/timer is set to 1000
  // this means that after it is multiplied by the PIDLIMIT it will be the proper value to give you the desired % output
  // it also makes the % calculations work properly in the log, UI, and other area's. 
  #ifdef PID_FLOW_CONTROL
    PIDCycle[VS_PUMP] = 1; // for PID pump flow the STEAM heat output is set to a fixed 10hz signal with 100 step outputs. 
  #endif
  
  for (byte vessel = VS_HLT; vessel <= VS_KETTLE; vessel++) {
    pid[vessel].SetInputLimits(0, 25500);
    pid[vessel].SetOutputLimits(0, PIDCycle[vessel] * pidLimits[vessel]);
    pid[vessel].SetTunings(getPIDp(vessel), getPIDi(vessel), getPIDd(vessel));
    pid[vessel].SetMode(PID::AUTO_MODE);
    pid[vessel].SetSampleTime(PID_CYCLE_TIME);
  }
  pid[VS_KETTLE].SetMode(PID::MANUAL_MODE);


  #ifdef PID_FLOW_CONTROL
    #ifdef USEMETRIC
      pid[VS_PUMP].SetInputLimits(0, 255000); // equivalent of 25.5 LPM (255 * 100)
    #else
      pid[VS_PUMP].SetInputLimits(0, 6375); // equivalent of 6.375 GPM (255 * 25)
    #endif
    pid[VS_PUMP].SetOutputLimits(PID_FLOW_MIN, PIDCycle[VS_PUMP] * PIDLIMIT_STEAM);
    pid[VS_PUMP].SetTunings(getPIDp(VS_PUMP), getPIDi(VS_PUMP), getPIDd(VS_PUMP));
    #ifdef PID_CONTROL_MANUAL
      pid[VS_PUMP].SetMode(MANUAL);
    #else
      pid[VS_PUMP].SetMode(AUTO);
    #endif
    pid[VS_PUMP].SetSampleTime(FLOWRATE_READ_INTERVAL);
    #ifdef PID_CONTROL_MANUAL
      nextcompute = millis() + FLOWRATE_READ_INTERVAL;
    #endif
  #else
    #ifdef USEMETRIC
      pid[VS_STEAM].SetInputLimits(0, 50000000 / steamPSens);
    #else
      pid[VS_STEAM].SetInputLimits(0, 7250000 / steamPSens);
    #endif
    pid[VS_STEAM].SetOutputLimits(0, PIDCycle[VS_STEAM] * PIDLIMIT_STEAM);
    pid[VS_STEAM].SetTunings(getPIDp(VS_STEAM), getPIDi(VS_STEAM), getPIDd(VS_STEAM));
    pid[VS_STEAM].SetMode(PID::AUTO_MODE);
    pid[VS_STEAM].SetSampleTime(PID_CYCLE_TIME);
  #endif

  #ifdef DEBUG_PID_GAIN
    for (byte vessel = VS_HLT; vessel <= VS_STEAM; vessel++) logDebugPIDGain(vessel);
  #endif
}

void resetOutputs() {
  actProfiles = 0;
  updateValves();
  for (byte i = VS_HLT; i <= LAST_HEAT_OUTPUT; i++)
    resetHeatOutput(i);
}

void resetHeatOutput(byte vessel) {
  #ifdef PWM_BY_TIMER
    uint8_t oldSREG;
  #endif
  setSetpoint(vessel, 0);
  PIDOutput[vessel] = 0;
  #ifdef PID_FEED_FORWARD
    if(vessel == VS_MASH)
      FFBias = 0;
  #endif
  #ifdef PWM_BY_TIMER
    // need to disable interrupts so a write into here can finish before an interrupt can come in and read it
    oldSREG = SREG;
    cli();
    //if we are not a 8K output then we can set it to 0, but if we are we need to set it to 1000 to make the duty cycle 0
    PIDOutputCountEquivalent[vessel][1] = 0;
  #endif
  heatPin[vessel].set(LOW);
  #ifdef PWM_BY_TIMER
    SREG = oldSREG; // restore interrupts
  #endif
}  

#if defined PID_FLOW_CONTROL && defined PID_CONTROL_MANUAL
void processPID_FLOW_CONTROL(byte vessel) {
  if(vessel == VS_PUMP){ //manual control if PID isnt working due to long sample times or other reasons
    millistemp = millis();
    if(millistemp >= nextcompute){
      nextcompute += FLOWRATE_READ_INTERVAL;
      if(setpoint[vessel] == 0) PIDOutput[vessel] = 0;
      else{
        if((long)setpoint[vessel] - flowRate[VS_KETTLE] > 100){
          additioncount[0]++;
          additioncount[1] = 0;
          if(additioncount[0] > 5){    // this is here to break a case where adding 10 causes a change of 100 but lowering 10 causes a change of 100 off the setpoint and we just oscilate. 
            additioncount[0] = 0;
            PIDOutput[vessel] += 5;
          }
          else PIDOutput[vessel] += 10;
        }
        else if((long)setpoint[vessel] - flowRate[VS_KETTLE] < -100){
          additioncount[0]++;
          additioncount[1] = 0;
          if(additioncount[0] > 5){    // this is here to break a case where adding 10 causes a change of 100 but lowering 10 causes a change of 100 off the setpoint and we just oscilate. 
           additioncount[0] = 0;
           PIDOutput[vessel] -= 5;
         }
         else PIDOutput[vessel] -= 10;
        }
        else if((long)setpoint[vessel] - flowRate[VS_KETTLE] > 50){ 
          additioncount[0] = 0;
          additioncount[1]++;
          if(additioncount[0] > 5){    // this is here to break a case where adding 5 causes a change of 50 but lowering 5 causes a change of 50 off the setpoint and we just oscilate. 
            additioncount[1] = 0;
            PIDOutput[vessel] += 1;
          }
          else PIDOutput[vessel] += 5;
        }
        else if((long)setpoint[vessel] - flowRate[VS_KETTLE] < -50){ 
          additioncount[0] = 0;
          additioncount[1]++;
          if(additioncount[0] > 5){    // this is here to break a case where adding 5 causes a change of 50 but lowering 5 causes a change of 50 off the setpoint and we just oscilate. 
            additioncount[1] = 0;
            PIDOutput[vessel] -= 1;
          }
          else PIDOutput[vessel] -= 5;
        }
        else if((long)setpoint[vessel] - flowRate[VS_KETTLE] > 10) PIDOutput[vessel] += 1;
        else if((long)setpoint[vessel] - flowRate[VS_KETTLE] < -10) PIDOutput[vessel] -= 1;
        
        if(PIDOutput[vessel] > pid[vessel].GetOUTMax()) PIDOutput[vessel] = pid[vessel].GetOUTMax();
        else if(PIDOutput[vessel] < pid[vessel].GetOUTMin()) PIDOutput[vessel] = pid[vessel].GetOUTMin();
      }
    }
  }
}
#endif // defined PID_FLOW_CONTROL && defined PID_CONTROL_MANUAL

/**
 * Called by processHeatOutputs to process a PID-enabled heat output.
 * Main things that happen here are updates to the PIDOutput and PIDInput arrays
 *   before calling compute() for the PID.
 * Once the PID has been computed we check if it results in heat being needed
 *   and we call either heatPin[vessel[VS]].set(HIGH); or heatPin[vessel[VS]].set(LOW);
 * NOTE: If PID is being processed by timer that happens elsewhere.
 */
void processHeatOutputsPIDEnabled(const byte vessel[]) {
  #ifdef PWM_BY_TIMER
    uint8_t oldSREG;
  #endif
  if (vessel[VS] != VS_STEAM && vessel[VS] != VS_KETTLE && temp[vessel[TS]] == BAD_TEMP) {
    PIDOutput[vessel[VS]] = 0;
  } else {
    if (pid[vessel[VS]].GetMode() == PID::AUTO_MODE) {
      #ifdef PID_FLOW_CONTROL
        if(vessel[VS] == VS_PUMP) PIDInput[vessel[VS]] = flowRate[VS_KETTLE];
      #else
        if (vessel[VS] == VS_STEAM) PIDInput[vessel[VS]] = steamPressure; 
      #endif
      else { 
        PIDInput[vessel[VS]] = temp[vessel[TS]];
      #ifdef PID_FEED_FORWARD
        if(vessel[VS] == VS_MASH ) FFBias = temp[FEED_FORWARD_SENSOR];
      #endif
      }
      pid[vessel[VS]].Compute();
      #ifdef PID_FLOW_CONTROL
        if(vessel[VS] == VS_PUMP && setpoint[vessel[VS]] == 0) PIDOutput[vessel[VS]] = 0; // if the setpoint is 0 then make sure we output 0, as dont want the min output always on. 
      #endif
      #ifdef PID_FEED_FORWARD
        if(vessel[VS] == VS_MASH && setpoint[vessel[VS]] == 0) PIDOutput[vessel[VS]] = 0; // found a bug where the mash output could be turned on if setpoint was 0 but FFBias was not 0. 
                                                             // this fixes the bug but still lets the integral gain learn to compensate for the FFBias while 
                                                             // the setpoint is 0. 
      #endif
    }
    #if defined PID_FLOW_CONTROL && defined PID_CONTROL_MANUAL
      processPID_FLOW_CONTROL(vessel[VS]);
    #endif // defined PID_FLOW_CONTROL && defined PID_CONTROL_MANUAL
  }
  
  #ifdef HLT_MIN_HEAT_VOL
    if(vessel[VS] == VS_HLT && volAvg[vessel[VS]] < HLT_MIN_HEAT_VOL) PIDOutput[vessel[VS]] = 0;
  #endif
  #ifdef MASH_MIN_HEAT_VOL
    if(vessel[VS] == VS_MASH && volAvg[vessel[VS]] < MASH_MIN_HEAT_VOL) PIDOutput[vessel[VS]] = 0;
  #endif
  #ifdef KETTLE_MIN_HEAT_VOL
    if(vessel[VS] == VS_KETTLE && volAvg[vessel[VS]] < KETTLE_MIN_HEAT_VOL) PIDOutput[vessel[VS]] = 0;
  #endif
    
  //Trigger based element save
  if (vesselMinTrigger(vessel[VS]) != NULL) if(!vesselMinTrigger(vessel[VS])->get()) PIDOutput[vessel[VS]] = 0;
  
  #ifndef PWM_BY_TIMER
    //only 1 call to millis needed here, and if we get hit with an interrupt we still want to calculate based on the first read value of it
    unsigned long timestamp = millis();
    if ((!estop) && PIDOutput[vessel[VS]] >= timestamp - cycleStart[vessel[VS]] && timestamp != cycleStart[vessel[VS]]) heatPin[vessel[VS]].set(HIGH); else heatPin[vessel[VS]].set(LOW);
  #else
    //here we do as much math as we can OUT SIDE the ISR, we calculate the PWM cycle time in counter/timer counts
    // and place it in the [vessel][0] value, then calculate the timer counts to get the desired PWM % and place it in [vessel][1]
    // need to disable interrupts so a write into here can finish before an interrupt can come in and read it
    oldSREG = SREG;
    cli();
    PIDOutputCountEquivalent[vessel[VS]][0] = PIDCycle[vessel[VS]] * 800;
    PIDOutputCountEquivalent[vessel[VS]][1] = PIDOutput[vessel[VS]] * 8;
    SREG = oldSREG; // restore interrupts
  #endif
  if (PIDOutput[vessel[VS]] == 0)  heatStatus[vessel[VS]] = 0; else heatStatus[vessel[VS]] = 1;
}


/**
 * Called by processHeatOutputsNonPIDEnabled to process a heat output when heatStatus[vessel] == true.
 */
void processHeatOutputsNonPIDEnabledWithHeatOn(const byte vessel[]) {
  // determine if setpoint has ben reached, or there is a bad temp reading.
  // If it either condition, set the pin low (turn it off).
  // we do not want the RIMS (in DIRECT_FIRED_RIMS) processed here either; it is taken care of in the MASH loop

  //Indicates if the minimum volume has been reached (defaults to true in case trigger is not used)
  boolean vesselMinTrig = 1;
  if (vesselMinTrigger(vessel[VS]) != NULL) vesselMinTrig = (vesselMinTrigger(vessel[VS])->get());

  if (
    estop 
    || (!vesselMinTrig) 
    || (vessel[VS] != VS_STEAM && (temp[vessel[TS]] == BAD_TEMP || temp[vessel[TS]] >= setpoint[vessel[VS]]))
    #ifndef DIRECT_FIRE_RIMS
      || (vessel[VS] == VS_STEAM && steamPressure >= setpoint[vessel[VS]])
    #endif
  ) { 
    // For DIRECT_FIRED_RIMS, the setpoint for both VS_MASH & VS_STEAM should be the same, 
    // so nothing to do here.
    heatPin[vessel[VS]].set(LOW);
    heatStatus[vessel[VS]] = 0;
  } else { 
    // setpoint has not been reached, and temp reading is valid.
    // Insure that the correct heat pin is enabled, and heatStatus updated.
    #ifdef DIRECT_FIRED_RIMS
      // When temp[VS_MASH] is less than setpoint[VS_MASH] - RIMS_TEMP_OFFSET, then
      // the VS_MASH pint should be set high, and VS_STEAM set low.  If the different
      // is within RIMS_TEMP_OFFSET, then the opposite.
      if (vessel[VS] == VS_MASH) {
        if (temp[TS_MASH] >= setpoint[VS_MASH] - (RIMS_TEMP_OFFSET * 100)) {
          heatPin[VS_MASH].set(LOW);
          heatStatus[VS_MASH] = 0;
          if ((temp[TS_MASH] < setpoint[VS_MASH]) && (temp[RIMS_TEMP_SENSOR] < (RIMS_MAX_TEMP * 100))) {
            heatPin[VS_STEAM].set(HIGH);
            heatStatus[VS_STEAM] = 1;
          } else {
            heatPin[VS_STEAM].set(LOW);
            heatStatus[VS_STEAM] = 0;
          }
        } else {
          heatPin[VS_MASH].set(HIGH);
          heatStatus[VS_MASH] = 1;
          heatPin[VS_STEAM].set(LOW);
          heatStatus[VS_STEAM] = 0;
        }
      } else {
        heatPin[vessel[VS]].set(HIGH);
        heatStatus[vessel[VS]] = 1;
      }
    #else
      heatPin[vessel[VS]].set(HIGH);
      heatStatus[vessel[VS]] = 1;
    #endif
  }
}

/**
 * Called by processHeatOutputsNonPIDEnabled to process a heat output when heatStatus[vessel] == false.
 */
void processHeatOutputsNonPIDEnabledWithHeatOff(const byte vessel[]) {
  // Determine is the vessel temperature is below the setpoint, accounting for hysteresis.
  // we do not want the RIMS (in DIRECT_FIRED_RIMS) processed here either; it is taken care of in the MASH loop

  //Indicates if the minimum volume has been reached (defaults to true in case trigger is not used)
  boolean vesselMinTrig = 1;
  if (vesselMinTrigger(vessel[VS]) != NULL) vesselMinTrig = (vesselMinTrigger(vessel[VS])->get());

  if (
    !estop 
    && vesselMinTrig 
    && (
      (vessel[VS] != VS_STEAM && temp[vessel[TS]] != BAD_TEMP && (setpoint[vessel[VS]] - temp[vessel[TS]]) >= hysteresis[vessel[VS]] * 10) 
    #ifndef DIRECT_FIRE_RIMS
      || (vessel[VS] == VS_STEAM && (setpoint[vessel[VS]] - steamPressure) >= hysteresis[vessel[VS]] * 100)
    #endif
       )
  ) {
      // The temperature of the vessel is below what we want, so insure the correct pin is tunred on,
      // and the heatStatus is updated.
    #ifdef DIRECT_FIRED_RIMS
      // When temp[VS_MASH] is less than setpoint[VS_MASH] - RIMS_TEMP_OFFSET, then
      // the VS_MASH pint should be set high, and VS_STEAM set low.  If the difference
      // is within RIMS_TEMP_OFFSET, then the opposite.
      if (vessel[VS] == VS_MASH) {
        if (temp[TS_MASH] >= setpoint[VS_MASH] - (RIMS_TEMP_OFFSET * 100)) {
          heatPin[VS_MASH].set(LOW);
          heatStatus[VS_MASH] = 0;
          if ((temp[TS_MASH] < setpoint[VS_MASH]) && (temp[RIMS_TEMP_SENSOR] < (RIMS_MAX_TEMP * 100))) {
            heatPin[VS_STEAM].set(HIGH);
            heatStatus[VS_STEAM] = 1;
          } else {
            heatPin[VS_STEAM].set(LOW);
            heatStatus[VS_STEAM] = 0;
          }
        } else {
          heatPin[VS_MASH].set(HIGH);
          heatStatus[VS_MASH] = 1;
          heatPin[VS_STEAM].set(LOW);
          heatStatus[VS_STEAM] = 0;
        }
      } else {
        heatPin[vessel[VS]].set(HIGH);
        heatStatus[vessel[VS]] = 1;
      }
    #else
      heatPin[vessel[VS]].set(HIGH);
      heatStatus[vessel[VS]] = 1;
    #endif
  } else {
    // The heat is maintaining currently desired value, so insure heat source is (still) off.
    // For DIRECT_FIRED_RIMS, the setpoint for both VS_MASH & VS_STEAM should be the same, 
    // so nothing to do here.
    heatPin[vessel[VS]].set(LOW);
    heatStatus[vessel[VS]] = 0;
    #ifdef DIRECT_FIRED_RIMS
      if (vessel[VS] == VS_MASH) {
        if ((!estop) && (temp[TS_MASH] < setpoint[VS_MASH]) && (temp[RIMS_TEMP_SENSOR] < (RIMS_MAX_TEMP * 100))) {
          heatPin[VS_STEAM].set(HIGH);
          heatStatus[VS_STEAM] = 1;
        } else {
          heatPin[VS_STEAM].set(LOW);
          heatStatus[VS_STEAM] = 0;
        }
      }
    #endif
  }
}

/**
 * Called by processHeatOutputs to process a nonPID-enabled heat output.
 */
void processHeatOutputsNonPIDEnabled(const byte vessel[]) {
  if (heatStatus[vessel[VS]]) {
    processHeatOutputsNonPIDEnabledWithHeatOn(vessel);
  } else {
    processHeatOutputsNonPIDEnabledWithHeatOff(vessel);
  }
  
#ifdef DIRECT_FIRED_RIMS
  // Check to insure RIMS is below safe level
  if (temp[RIMS_TEMP_SENSOR] >= (RIMS_ALARM_TEMP * 100)  ) {
  #ifdef ALARM_PIN
    alarmPin.set(1); //Sound the alarm.
  #endif
  } else {
  #ifdef ALARM_PIN
    alarmPin.set(0);
  #endif
  }
#endif  
}

/*
 * Things get a little complex here, so let's break it down.
 * processHeatOutputs() is called by the main brewCore() loop.
 *    Processes the RIMS_MLT_SETPOINT_DELAY, which waits a number of
 *      milliseconds before enabling the setpoint for RIMS.
 *    Calls boilController() which manages the auto boil process. If
 *      auto boil is active it may set PIDOutput[VS_KETTLE]
 *    Then it loops through all of the vessels (using HEAT_OUTPUTS_COUNT
 *      as a maximum) and:
 *      Skips various vessels depending on certain conditions.
 *      If the PID for the vessel is enabled it calls processHeatOutputsPIDEnabled
 *        otherwise it calls processHeatOutputsNonPIDEnabled.
 */       
void processHeatOutputs() {
  //Process Heat Outputs
  #ifdef RIMS_MLT_SETPOINT_DELAY
    if(timetoset <= millis() && timetoset != 0){
      RIMStimeExpired = 1;
      timetoset = 0;
      setSetpoint(TS_MASH, getProgMashTemp(stepProgram[steptoset], steptoset - 5));
    }
  #endif
  
  //Process Auto Boil Control Logic
  boilController();
  
  for (int vesselIndex = 0; vesselIndex < HEAT_OUTPUTS_COUNT; vesselIndex++) {
    #ifndef PWM_BY_TIMER
      unsigned long timestamp = millis();
      if (cycleStart[vesselIndex] == 0) cycleStart[vesselIndex] = timestamp;
      if (timestamp - cycleStart[vesselIndex] > PIDCycle[vesselIndex] * 100) cycleStart[vesselIndex] += PIDCycle[vesselIndex] * 100;
    #endif
    
    #ifdef HLT_AS_KETTLE
      if (
        (vesselIndex == VS_KETTLE && setpoint[VS_HLT]) //Skip kettle heat if HLT setpoint is active
        || (vesselIndex == VS_HLT && !setpoint[VS_HLT] && setpoint[VS_KETTLE]) //Skip HLT if HLT setpoint is inactive and Kettle setpoint is active
      ) continue;
    #elif defined KETTLE_AS_MASH
      if (
        (vesselIndex == VS_KETTLE && setpoint[VS_MASH]) //Skip kettle heat if Mash setpoint is active
        || (vesselIndex == VS_MASH && !setpoint[VS_MASH] && setpoint[VS_KETTLE]) //Skip Mash if Mash setpoint is inactive and Kettle setpoint is active
      ) continue;
    #elif defined SINGLE_VESSEL_SUPPORT
      if (
        (!setpoint[vesselIndex] && (setpoint[VS_HLT] || setpoint[VS_MASH] || setpoint[VS_KETTLE]))
        || (setpoint[VS_MASH] && vesselIndex != VS_MASH)
        || (setpoint[VS_KETTLE] && vesselIndex == VS_HLT)
      ) continue;
    #endif
    
    #ifdef RGBIO8_ENABLE
    if (softSwitchHeat[vesselIndex] == SOFTSWITCH_AUTO) {
      // Auto
    #endif
      if (PIDEnabled[HEAT_OUTPUTS[vesselIndex][VS]]) {
        processHeatOutputsPIDEnabled(HEAT_OUTPUTS[vesselIndex]);
      } else {
        processHeatOutputsNonPIDEnabled(HEAT_OUTPUTS[vesselIndex]);
      }
    #ifdef RGBIO8_ENABLE
    }
    else if (!estop && softSwitchHeat[vesselIndex] == SOFTSWITCH_ON) {
      // On
      heatPin[vesselIndex].set(HIGH);
      heatStatus[vesselIndex] = 1;
    }
    else {
      // Off, or invalid, which is as good as Off
      heatPin[vesselIndex].set(LOW);
      heatStatus[vesselIndex] = 0;
    }
    #endif
  }
}

#ifdef PVOUT
  void updateValves() {
    setValves(computeValveBits());
  }
  
  void setValves(unsigned long vlvBits) {
    if (vlvBits != Valves.get()) {
      Valves.set(vlvBits);
      //Mirror outputs to Modbus
      #ifdef PVOUT_TYPE_MODBUS
        if (ValvesMB[0])
          ValvesMB[0]->set(vlvBits >> (ValvesMB[0]->offset()));
        
        if (ValvesMB[1])
          ValvesMB[1]->set(vlvBits >> (ValvesMB[1]->offset()));
      #endif
    }
  }

  void processAutoValve() {
    #ifdef HLT_MIN_REFILL
      unsigned long HLTStopVol;
    #endif
    //Do Valves
    if (autoValve[AV_FILL]) {
      if (volAvg[VS_HLT] < tgtVol[VS_HLT]) bitSet(actProfiles, VLV_FILLHLT);
        else bitClear(actProfiles, VLV_FILLHLT);
        
      if (volAvg[VS_MASH] < tgtVol[VS_MASH]) bitSet(actProfiles, VLV_FILLMASH);
        else bitClear(actProfiles, VLV_FILLMASH);
    }
    
    //HLT/MASH/KETTLE AV Logic
    for (byte i = VS_HLT; i <= VS_KETTLE; i++) {
      byte vlvHeat = vesselVLVHeat(i);
      byte vlvIdle = vesselVLVIdle(i);
      if (autoValve[vesselAV(i)]) {
        if (heatStatus[i]) {
          if (vlvConfigIsActive(vlvIdle)) bitClear(actProfiles, vlvIdle);
          if (!vlvConfigIsActive(vlvHeat)) bitSet(actProfiles, vlvHeat);
        } else {
          if (vlvConfigIsActive(vlvHeat)) bitClear(actProfiles, vlvHeat);
          if (!vlvConfigIsActive(vlvIdle)) bitSet(actProfiles, vlvIdle); 
        }
      }
    }
    
    if (autoValve[AV_SPARGEIN]) {
      if (volAvg[VS_HLT] > tgtVol[VS_HLT]) bitSet(actProfiles, VLV_SPARGEIN);
        else bitClear(actProfiles, VLV_SPARGEIN);
    }
    if (autoValve[AV_SPARGEOUT]) {
      if (volAvg[VS_KETTLE] < tgtVol[VS_KETTLE]) bitSet(actProfiles, VLV_SPARGEOUT);
      else bitClear(actProfiles, VLV_SPARGEOUT);
    }
    if (autoValve[AV_FLYSPARGE]) {
      if (volAvg[VS_KETTLE] < tgtVol[VS_KETTLE]) {
        #ifdef SPARGE_IN_PUMP_CONTROL
          if((long)volAvg[VS_KETTLE] - (long)prevSpargeVol[0] >= SPARGE_IN_HYSTERESIS)
          {
            #ifdef HLT_MIN_REFILL
               HLTStopVol = (SpargeVol > HLT_MIN_REFILL_VOL ? getVolLoss(VS_HLT) : (HLT_MIN_REFILL_VOL - SpargeVol));
               if(volAvg[VS_HLT] > HLTStopVol + 20)
            #else
               if(volAvg[VS_HLT] > getVolLoss(VS_HLT) + 20)
            #endif
                 bitSet(actProfiles, VLV_SPARGEIN);
             prevSpargeVol[0] = volAvg[VS_KETTLE];
          }
          #ifdef HLT_FLY_SPARGE_STOP
          else if((long)prevSpargeVol[1] - (long)volAvg[VS_HLT] >= SPARGE_IN_HYSTERESIS || volAvg[VS_HLT] < HLT_FLY_SPARGE_STOP_VOLUME + 20)
          #else
          else if((long)prevSpargeVol[1] - (long)volAvg[VS_HLT] >= SPARGE_IN_HYSTERESIS || volAvg[VS_HLT] < getVolLoss(VS_HLT) + 20)
          #endif
          {
             bitClear(actProfiles, VLV_SPARGEIN);
             prevSpargeVol[1] = volAvg[VS_HLT];
          }
        #else
          bitSet(actProfiles, VLV_SPARGEIN);
        #endif
        bitSet(actProfiles, VLV_SPARGEOUT);
      } else {
        bitClear(actProfiles, VLV_SPARGEIN);
        bitClear(actProfiles, VLV_SPARGEOUT);
      }
    }
    if (autoValve[AV_CHILL]) {
      //Needs work
      /*
      //If Pumping beer
      if (vlvConfigIsActive(VLV_CHILLBEER)) {
        //Cut beer if exceeds pitch + 1
        if (temp[TS_BEEROUT] > pitchTemp + 1.0) bitClear(actProfiles, VLV_CHILLBEER);
      } else {
        //Enable beer if chiller H2O output is below pitch
        //ADD MIN DELAY!
        if (temp[TS_H2OOUT] < pitchTemp - 1.0) bitSet(actProfiles, VLV_CHILLBEER);
      }
      
      //If chiller water is running
      if (vlvConfigIsActive(VLV_CHILLH2O)) {
        //Cut H2O if beer below pitch - 1
        if (temp[TS_BEEROUT] < pitchTemp - 1.0) bitClear(actProfiles, VLV_CHILLH2O);
      } else {
        //Enable H2O if chiller H2O output is at pitch
        //ADD MIN DELAY!
        if (temp[TS_H2OOUT] >= pitchTemp) bitSet(actProfiles, VLV_CHILLH2O);
      }
      */
    }
  }
#endif //#ifdef PVOUT
  
unsigned long computeValveBits() {
  if (estop) return 0;
  unsigned long vlvBits = 0;
  
  //Force certain valve profiles off based on trigger pins if enabled
  if (TriggerPin[TRIGGER_SPARGEMAX] != NULL) if(TriggerPin[TRIGGER_SPARGEMAX]->get()) bitClear(actProfiles, VLV_SPARGEIN);
  if (TriggerPin[TRIGGER_HLTMIN] != NULL) if(!TriggerPin[TRIGGER_HLTMIN]->get()) bitClear(actProfiles, VLV_HLTHEAT);
  if (TriggerPin[TRIGGER_MASHMIN] != NULL) if(!TriggerPin[TRIGGER_MASHMIN]->get()) bitClear(actProfiles, VLV_MASHHEAT);
  if (TriggerPin[TRIGGER_KETTLEMIN] != NULL) if(!TriggerPin[TRIGGER_KETTLEMIN]->get()) bitClear(actProfiles, VLV_KETTLEHEAT);
  
  for (byte i = 0; i < NUM_VLVCFGS; i++) {
    if (bitRead(actProfiles, i)) {
      vlvBits |= vlvConfig[i];
    }
  }
  #ifdef RGBIO8_ENABLE
  // Build the softswitch masks
  // Any bits set to 1 on offMask will force the corresponding valve off.
  unsigned long offMask = 0;
  // Any bits set to 1 on onMask will force the corresponding valve on.
  unsigned long onMask = 0;
  for (int i = 0; i < PVOUT_COUNT; i++) {
    if (softSwitchPv[i] == SOFTSWITCH_OFF) {
      offMask |= (1 << i);
    }
    else if (softSwitchPv[i] == SOFTSWITCH_ON) {
      onMask |= (1 << i);
    }
  }
  // Apply the masks to the pre-computed valve bits.
  offMask = ~offMask;
  vlvBits &= offMask;
  vlvBits |= onMask;
  #endif
  return vlvBits;
}

boolean vlvConfigIsActive(byte profile) {
  //An empty valve profile cannot be active
  if (!vlvConfig[profile]) return 0;
  return bitRead(actProfiles, profile);
}

void boilController () {
  if (boilControlState == CONTROLSTATE_AUTO) {
    if(temp[TS_KETTLE] < setpoint[TS_KETTLE]) PIDOutput[VS_KETTLE] = PIDCycle[VS_KETTLE] * PIDLIMIT_KETTLE;
    else PIDOutput[VS_KETTLE] = PIDCycle[VS_KETTLE] * min(boilPwr, PIDLIMIT_KETTLE);
  }
}

//Map AutoValve Profiles to Vessels
byte vesselAV(byte vessel) {
  if (vessel == VS_HLT) return AV_HLT;
  else if (vessel == VS_MASH) return AV_MASH;
  else if (vessel == VS_KETTLE) return AV_KETTLE;
}

byte vesselVLVHeat(byte vessel) {
  if (vessel == VS_HLT) return VLV_HLTHEAT;
  else if (vessel == VS_MASH) return VLV_MASHHEAT;
  else if (vessel == VS_KETTLE) return VLV_KETTLEHEAT;
}

byte vesselVLVIdle(byte vessel) {
  if (vessel == VS_HLT) return VLV_HLTIDLE;
  else if (vessel == VS_MASH) return VLV_MASHIDLE;
  else if (vessel == VS_KETTLE) return VLV_KETTLEIDLE;
}

pin * vesselMinTrigger(byte vessel) {
  if (vessel == VS_HLT) return TriggerPin[TRIGGER_HLTMIN];
  else if (vessel == VS_MASH) return TriggerPin[TRIGGER_MASHMIN];
  else if (vessel == VS_KETTLE) return TriggerPin[TRIGGER_KETTLEMIN];
  else return NULL;
}

byte autoValveBitmask(void) {
  byte modeMask = 0;
  for (byte i = AV_FILL; i < NUM_AV; i++)
    if (autoValve[i]) modeMask |= 1<<i;
  return modeMask;
}

byte getHeatPower (byte vessel) {
  return (PIDEnabled[vessel] ? (PIDOutput[vessel] / PIDCycle[vessel]) : (heatStatus[vessel] ? 100 : 0));
}

