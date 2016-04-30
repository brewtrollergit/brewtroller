#ifndef COM_RGBIO8_H
#define COM_RGBIO8_H

#include "BrewTroller.h"
#include "Config.h"


#ifdef RGBIO8_ENABLE

#define RGBIO8_MAX_OUTPUT_RECIPES 4
#define RGBIO8_INTERVAL 100

#define SOFTSWITCH_OFF 0
#define SOFTSWITCH_ON 1
#define SOFTSWITCH_AUTO 2

extern byte softSwitchHeat[HEAT_OUTPUTS_COUNT];
extern byte softSwitchPv[PVOUT_COUNT];

struct RGBIO8_output_assignment {
  byte type;
  byte index;
  byte recipe_id;
};

struct RGBIO8_input_assignment {
  byte type;
  byte index;
};

class RGBIO8 {
  public:
    RGBIO8();
    
    /**
     * Initializes the RGBIO8 board at the given endpoint. Specify either the rs485_address or the
     * i2c_address and specify 0 for the unused address. Once initialized the board is reset and
     * ready for use but it will not actually do anything until assign* functions are called.
     */
    void begin(int rs485_address, int i2c_address);
    
    /**
     * Creates an output recipe that can be assigned to an output. A recipe determines
     * which colors will be shown for a given output state. The returned recipe id can
     * then be used for multiple outputs.
     * Red, green and blue values should be 4 bits each, i.e. values from 0-15. A good way
     * to specify the color is using three characters of hex, where the characters are
     * r, g, b.
     * Examples:
     *   Red: 0xf00
     *   Purple: 0xf0f
     *   Yellow: 0x0ff
     *   White: 0xfff
     */
    static void setOutputRecipe(
      byte recipe_id, 
      uint16_t off_rgb,
      uint16_t auto_off_rgb,
      uint16_t auto_on_rgb,
      uint16_t on_rgb);
    
    /**
     * Attaches the given output to the specified vessel using the given recipe. Once this has been
     * called, future calls to update() will query the specified heat output for it's status and
     * then update the output with the recipe.
     */
    void assignHeatOutputRecipe(byte vessel, byte output, byte recipe_id);
    
    /**
     * Attaches the given output to the specified PV using the given recipe. Once this has been
     * called, future calls to update() will query the specified PV for it's status and
     * then update the output with the recipe.
     */
    void assignPvOutputRecipe(byte pv, byte output, byte recipe_id);
    
    /**
     * Creates a soft switch for the specified vessel attached to the specified input of the
     * RGBIO8 board. This causes the output of the specified vessel to be determined by the
     * logical AND() of the BrewTroller logic and the state of the given input.
     */
    void assignHeatInput(byte vessel, byte input);
    
    /**
     * Creates a soft switch for the specified PV attached to the specified input of the
     * RGBIO8 board. This causes the output of the specified PV to be determined by the
     * logical AND() of the BrewTroller logic and the state of the given input.
     */
    void assignPvInput(byte pv, byte input);
    
    /**
     * Called regularly by the main program loop. Updates the local state of the inputs, checks
     * the values of the outputs and sends any new output state that needs to be updated.
     */
    void update(void);

    void restart();
    void setIdMode(byte id_mode);
    void setAddress(byte a);
    
  private:
    static uint16_t output_recipes[RGBIO8_MAX_OUTPUT_RECIPES][4];
    int rs485_address, i2c_address;
    struct RGBIO8_output_assignment output_assignments[8];
    struct RGBIO8_input_assignment input_assignments[8];
    byte inputs_auto, inputs_manual;
    
    int getInputs(uint8_t *m, uint8_t *a);
    void setOutput(byte output, uint16_t rgb);
    uint8_t crc8(uint8_t inCrc, uint8_t inData );
};

void RGBIO8_Init();
void RGBIO8_Update();

#endif

#endif

