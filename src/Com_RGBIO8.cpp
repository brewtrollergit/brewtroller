#include "Com_RGBIO8.h"
#include "Outputs.h"

#ifdef RGBIO8_ENABLE

byte softSwitchPv[PVOUT_COUNT];
byte softSwitchHeat[HEAT_OUTPUTS_COUNT];

RGBIO8 rgbio8s[RGBIO8_NUM_BOARDS];
unsigned long lastRGBIO8 = 0;

// Initializes the RGBIO8 system. If you want to provide custom IO mappings
// this is the place to do it. See the CUSTOM CONFIGURATION section below for
// further instructions.
void RGBIO8_Init() {
  // Initialize and address each RGB board that is attached
  for (int i = 0; i < RGBIO8_NUM_BOARDS; i++) {
    rgbio8s[i].begin(0, RGBIO8_START_ADDR + i);
  }
  
  // Set the default values of Softswitches to AUTO so that outputs that are not assigned to softswitches are unaffected by this logic
  for (byte i = 0; i < PVOUT_COUNT; i++)
    softSwitchPv[i] = SOFTSWITCH_AUTO;

  for (byte i = 0; i < HEAT_OUTPUTS_COUNT; i++)
    softSwitchHeat[i] = SOFTSWITCH_AUTO;

  
  ////////////////////////////////////////////////////////////////////////
  // CUSTOM CONFIGURATION
  ////////////////////////////////////////////////////////////////////////
  // To provide your own custom IO mappings you will have to add code to
  // this section. The code is very simple and the mappings are very
  // powerful.
  //
  // The system is configured by providing input and output mappings
  // for heat outputs and pump/valve outputs. Each of these outputs
  // can be in one of four states:
  // Off:       The output is forced off, no matter what other systems attempt.
  // Auto Off:  The output is under auto control of BrewTroller, and is
  //            currently set to off. It may turn on at any time.
  // Auto On:   The output is under auto control of BrewTroller, and is
  //            currently set to on. It may turn off at any time.
  // On:        The output is forced on and is not under control of 
  //            BrewTroller.
  // 
  // The first thing that is configured are output "recipes". These recipes
  // define the color that will be shown for each of the states above.
  // 
  // Often times you will see colors on a web page expressed in RGB
  // hexidecimal, such as #FF0000 meaning bright red or #FFFF00 meaning
  // bright yellow. The RGBIO8 board uses a similar system for color,
  // except it uses 3 digits instead of 6. In most cases, if you find
  // a color you like that is in the #ABCDEF format, you can convert it
  // to the right code for RGBIO8 by removing the second, fourth and
  // last digit. So, for instance, #ABCDEF would become #ACE.
  // 
  // The system has room for four recipes, so you can create 4 different
  // color schemes that map to your outputs.
  // 
  // By default we use two recipes. One for heat outputs and another for
  // pump/valve outputs. They are listed below. If you like, you can just
  // change the colors in a recipe, or you can create entirely new recipes.
  
  // Recipe 0, used for Heat Outputs
  // Off:       0xF00 (Red)
  // Auto Off:  0xFFF (White)
  // Auto On:   0xF40 (Orange)
  // On:        0x0F0 (Green)
  RGBIO8::setOutputRecipe(0, 0xF00, 0xFFF, 0xF40, 0x0F0);
  
  // Recipe 1, used for Pump/Valve Outputs
  // Off:       0xF00 (Red)
  // Auto Off:  0xFFF (White)
  // Auto On:   0x00F (Blue)
  // On:        0x0F0 (Green)
  RGBIO8::setOutputRecipe(1, 0xF00, 0xFFF, 0x00F, 0x0F0);

  //
  // Now we move on to mappings. A mapping ties a given input or output to
  // either a heat output or a pump/valve output. 
  // 
  // To create a mapping between a heat output you use one of the following
  // two functions:
  // assignHeatInput(vesselNumber, inputNumber);
  // assignHeatOutputRecipe(vesselNumber, outputNumber, recipeNumber);
  //
  // To create a mapping between a pump/valve output you use one of the
  // following two functions.
  // assignPvInput(pvOutputNumber, inputNumber);
  // assignPvOutputRecipe(pvOutputNumber, outputNumber, recipeNumber);
  //
  // When creating a mapping, you have to specify which RGB board the mapping
  // belongs to. That is done by using rgbio8s[boardNumber]. before the
  // function calls above. Some example mappings are shown below:
  // 
  // Map board 0, heat output 0 (HLT) to input/output 0 using recipe 0.
  // rgbio8s[0].assignHeatInput(0, 0);
  // rgbio8s[0].assignHeatOutputRecipe(0, 0, 0);
  //
  // 
  // Map board 1, pump/valve output 2 to input/output 3 using recipe 1.
  // rgbio8s[1].assignPvInput(2, 3);
  // rgbio8s[1].assignPvOutputRecipe(2, 3, 1);
  //
  // Add your custom mappings below this line
}

void RGBIO8_Update() {
  if (millis() > (lastRGBIO8 + RGBIO8_INTERVAL)) {
    for (int i = 0; i < RGBIO8_NUM_BOARDS; i++) {
      rgbio8s[i].update();
    }
    lastRGBIO8 = millis();
  }
}

uint16_t RGBIO8::output_recipes[RGBIO8_MAX_OUTPUT_RECIPES][4];

RGBIO8::RGBIO8() {
  this->rs485_address = 0;
  this->i2c_address = 0;
  for (int i = 0; i < 8; i++) {
    output_assignments[i].type = 0;
    input_assignments[i].type = 0;
  }
}

void RGBIO8::begin(int rs485_address, int i2c_address) {
  this->rs485_address = rs485_address;
  this->i2c_address = i2c_address;
}

void RGBIO8::setOutputRecipe(
  byte recipe_id, 
  uint16_t off_rgb,
  uint16_t auto_off_rgb,
  uint16_t auto_on_rgb,
  uint16_t on_rgb) {
    output_recipes[recipe_id][0] = off_rgb;
    output_recipes[recipe_id][1] = auto_off_rgb;
    output_recipes[recipe_id][2] = auto_on_rgb;
    output_recipes[recipe_id][3] = on_rgb;
}
    
void RGBIO8::assignHeatOutputRecipe(byte vessel, byte output, byte recipe_id) {
  output_assignments[output].type = 1;
  output_assignments[output].index = vessel;
  output_assignments[output].recipe_id = recipe_id;
}
    
void RGBIO8::assignPvOutputRecipe(byte pv, byte output, byte recipe_id) {
  output_assignments[output].type = 2;
  output_assignments[output].index = pv;
  output_assignments[output].recipe_id = recipe_id;
}
    
void RGBIO8::assignHeatInput(byte vessel, byte input) {
  input_assignments[input].type = 1;
  input_assignments[input].index = vessel;
}
    
void RGBIO8::assignPvInput(byte pv, byte input) {
  input_assignments[input].type = 2;
  input_assignments[input].index = pv;
}
    
void RGBIO8::update(void) {
  // Get the state of the 8 inputs first
  getInputs(&inputs_manual, &inputs_auto);
  
  // Update any assigned inputs
  for (int i = 0; i < 8; i++) {
    RGBIO8_input_assignment *a = &input_assignments[i];
    if (a->type) {
      if (a->type == 1) {
        // this is a heat input
        if (inputs_manual & (1 << i)) {
          softSwitchHeat[a->index] = SOFTSWITCH_ON;
        }
        else if (inputs_auto & (1 << i)) {
          softSwitchHeat[a->index] = SOFTSWITCH_AUTO;
        }
        else {
          softSwitchHeat[a->index] = SOFTSWITCH_OFF;
        }
      }
      else if (a->type == 2) {
        // this is a PV input
        if (inputs_manual & (1 << i)) {
          softSwitchPv[a->index] = SOFTSWITCH_ON;
        }
        else if (inputs_auto & (1 << i)) {
          softSwitchPv[a->index] = SOFTSWITCH_AUTO;
        }
        else {
          softSwitchPv[a->index] = SOFTSWITCH_OFF;
        }
      }
    }
  }
  
  // Update any assigned outputs
  #ifdef PVOUT
  uint32_t vlvBits = getValveBits();
  #endif
  for (int i = 0; i < 8; i++) {
    RGBIO8_output_assignment *a = &output_assignments[i];
    if (a->type) {
      if (a->type == 1) {
        // this is a heat output
        // If PIDEnabled[a->index] is set and the PID is heating, heatStatus
        // will always be set. It does not reflect the state of the pin.
        // If we want to reflect the actual state of the pin we'd also
        // need to check against heatPin[a->index].get().
        if (heatStatus[a->index]) {
          if (softSwitchHeat[a->index] == SOFTSWITCH_AUTO) {
            setOutput(i, output_recipes[a->recipe_id][2]);
          }
          else {
            setOutput(i, output_recipes[a->recipe_id][3]);
          }
        }
        else {
          if (softSwitchHeat[a->index] == SOFTSWITCH_AUTO) {
            setOutput(i, output_recipes[a->recipe_id][1]);
          }
          else {
            setOutput(i, output_recipes[a->recipe_id][0]);
          }
        }
      }
      else if (a->type == 2) {
        // this is a PV output
        #ifdef PVOUT
        if (vlvBits & (1UL << (uint32_t)a->index)) {
          if (softSwitchPv[a->index] == SOFTSWITCH_AUTO) {
            setOutput(i, output_recipes[a->recipe_id][2]);
          }
          else {
            setOutput(i, output_recipes[a->recipe_id][3]);
          }
        }
        else {
          if (softSwitchPv[a->index] == SOFTSWITCH_AUTO) {
            setOutput(i, output_recipes[a->recipe_id][1]);
          }
          else {
            setOutput(i, output_recipes[a->recipe_id][0]);
          }
        }
        #endif
      }
    }
  }
}

void RGBIO8::restart() {
  Wire.beginTransmission(i2c_address);
  Wire.write(0xfd);
  Wire.endTransmission();
}

void RGBIO8::setIdMode(byte id_mode) {
  Wire.beginTransmission(i2c_address);
  Wire.write(0xfe);
  Wire.write(id_mode);
  Wire.endTransmission();
}

void RGBIO8::setAddress(byte a) {
  Wire.beginTransmission(i2c_address);
  Wire.write(0xff);
  Wire.write(a);
  Wire.endTransmission();
}

int RGBIO8::getInputs(uint8_t *m, uint8_t *a) {
  Wire.requestFrom(i2c_address, 3);
  uint8_t inputs_m = Wire.read();
  uint8_t inputs_a = Wire.read();
  uint8_t crc = Wire.read();
  
  uint8_t crc_comp = '*';
  crc_comp = crc8(crc_comp, inputs_m);
  crc_comp = crc8(crc_comp, inputs_a);
  
  if (crc == crc_comp) {
    *m = inputs_m;
    *a = inputs_a;
    return 1;
  }
  else {
    return 0;
  }
}

void RGBIO8::setOutput(byte output, uint16_t rgb) {
  Wire.beginTransmission(i2c_address);
  Wire.write(0x01);
  Wire.write(output);
  Wire.write((uint8_t*) &rgb, 2);
  Wire.endTransmission();
}

uint8_t RGBIO8::crc8(uint8_t inCrc, uint8_t inData ) {
  uint8_t i;
  uint8_t data;

  data = inCrc ^ inData;
  
  for (i = 0; i < 8; i++) {
    if ((data & 0x80) != 0) {
      data <<= 1;
      data ^= 0x07;
    }
    else {
      data <<= 1;
    }
  }

  return data;
}


#endif

