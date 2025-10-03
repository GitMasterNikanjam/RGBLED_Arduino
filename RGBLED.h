
#pragma once

/**
 * @file RGBLED.h
 * @brief Arduino library to control a simple RGB LED module (common-anode or common-cathode).
 * @details
 *  - Set specific colors (red, green, blue, yellow, cyan, purple, white).  
 *  - Turn ON/OFF, toggle, invert channels.  
 *  - Blink in blocking or non-blocking mode (non-blocking uses @ref blinkUpdate).  
 *  - Optional PWM path with 8-bit color and global brightness.  
 *
 * @note When PWM is disabled, digital writes (HIGH/LOW) are used (boolean colors).  
 * @warning When PWM is enabled, availability of @c analogWrite and PWM-capable pins depends
 *          on the board/core (e.g., AVR vs ESP32).
 * @version 1.0
 * @author Mohammad
 */

/**
 * @def RGBLED_ANALOG_WRITE
 * @brief Backend used for PWM writes when PWM mode is enabled.
 *
 * Override this macro *before* including this header if you want to use a different PWM API,
 * e.g. map to @c ledcWrite on ESP32.
 *
 * @code
 *   #define RGBLED_ANALOG_WRITE(pin, val) ledcWrite(myChannelFor(pin), (val))
 *   #include "RGBLED.h"
 * @endcode
 */
#ifndef RGBLED_ANALOG_WRITE
  #define RGBLED_ANALOG_WRITE(pin, val) analogWrite((pin), (val))
#endif

// ########################################################################################
// Include libraries:

#include <Arduino.h>

// ###########################################################################
// Enumerations & Structures
// ###########################################################################

/**
 * @enum RGBLED_ActiveMode
 * @brief Specifies the electrical active mode for driving the RGB LED.
 *
 * This enumeration defines how the microcontroller (MCU) output signal
 * controls the ON/OFF state of the LED, depending on whether the LED is
 * wired as active-low or active-high:
 *
 * - **RGBLED_ACTIVE_LOW**:  
 *   The LED turns ON when the MCU output is logic LOW (0).  
 *   Writing `LOW` to the pin lights the LED.  
 *   Writing `HIGH` to the pin turns it OFF.
 *
 * - **RGBLED_ACTIVE_HIGH**:  
 *   The LED turns ON when the MCU output is logic HIGH (1).  
 *   Writing `HIGH` to the pin lights the LED.  
 *   Writing `LOW` to the pin turns it OFF.
 */
enum RGBLED_ActiveMode : uint8_t 
{
  RGBLED_ACTIVE_LOW  = 0,  /**< LED is active when MCU output is LOW (0). */
  RGBLED_ACTIVE_HIGH = 1   /**< LED is active when MCU output is HIGH (1). */
};


/**
 * @struct RGBLED_Params
 * @brief Parameter bag for RGBLED configuration.
 * @details Set these fields before calling @ref RGBLED::init.
 * @note Default pin values are -1 (invalid). All three pins must be valid.
 */
struct RGBLED_Params 
{
  int8_t  RED_PIN   = -1;                   ///< Arduino pin for Red channel.
  int8_t  GREEN_PIN = -1;                   ///< Arduino pin for Green channel.
  int8_t  BLUE_PIN  = -1;                   ///< Arduino pin for Blue channel.
  uint8_t ACTIVE_MODE = RGBLED_ACTIVE_HIGH; ///< Active mode: 0:LOW, 1:HIGH
};

// ###########################################################################
// Class Definition
// ###########################################################################

/**
 * @class RGBLED
 * @brief Control an RGB LED on Arduino boards (CA/CC), with blocking & non-blocking blink.
 * @details Reports parameter validation errors via @ref lastError and @ref errorText.
 */
class RGBLED
{
  public:

    // -----------------------------------------------------------------------
    // Public members
    // -----------------------------------------------------------------------

    // ---------- Error handling: small, deterministic (no dynamic String) ----------

    /**
     * @enum RGBLED_Error
     * @brief Error codes reported by the library.
     */
    enum RGBLED_Error : uint8_t { RGBLED_OK = 0, RGBLED_ERR_PARAMS = 1 };

    ///< Last error code (updated by @ref init and validation).
    RGBLED_Error lastError = RGBLED_OK;

    /**
     * @brief Convert error code to PROGMEM string.
     * @param e Error code.
     * @return Pointer to flash-stored string describing the error.
     */
    static const __FlashStringHelper* errorText(RGBLED_Error e) 
    {
      switch (e) 
      {
        case RGBLED_OK:         return F("OK");
        case RGBLED_ERR_PARAMS: return F("Invalid parameters");
        default:                return F("Unknown");
      }
    }

    // --------------------------------------------------------------------------
    // Configuration
    // --------------------------------------------------------------------------

    RGBLED_Params parameters; ///< Configuration parameters (pins and wiring mode).

    // -----------------------------------------------------------------------
    // Constructor / Destructor
    // -----------------------------------------------------------------------

    /**
     * @brief Construct a new RGBLED object with default parameters.
     */
    RGBLED();

    /**
     * @brief Destructor. Turns off LED and releases pins (sets to INPUT).
     */
    ~RGBLED();

    // -----------------------------------------------------------------------
    // Initialization
    // -----------------------------------------------------------------------

    /**
     * @brief Initialize the RGBLED with user parameters.
     * @details Validates parameters, preloads OFF level to avoid glitches, configures pins,
     *          and ensures the LED starts OFF.
     * @pre  @ref parameters must contain valid pins and a valid COMMON_STATE.
     * @post Internal guard flag is set; subsequent control calls are accepted.
     * @retval true  Initialization succeeded.
     * @retval false Parameters are invalid; see @ref lastError and @ref errorText.
     */
    bool init(void);

    // -----------------------------------------------------------------------
    // Basic control methods
    // -----------------------------------------------------------------------

    /**
     * @brief Set LED to a custom color (boolean channels).
     * @pre   @ref init must have succeeded.
     * @param redState   true to enable red channel;   false to disable.
     * @param greenState true to enable green channel; false to disable.
     * @param blueState  true to enable blue channel;  false to disable.
     * @post  Cached color is updated and applied; @ref isOn becomes true if any channel is ON.
     * @sa    @ref setRGB
     */
    void set(bool redState, bool greenState, bool blueState);

    /**
     * @brief Turn LED completely OFF.
     * @pre   @ref init must have succeeded.
     */
    void off(void);

    /**
     * @brief Turn LED back ON using the last cached color.
     * @pre   @ref init must have succeeded.
     */
    void on(void);

    /**
     * @brief Toggle LED state between cached color and OFF.
     * @pre   @ref init must have succeeded.
     */
    void toggle(void);

    /**
     * @brief Invert all channels (R,G,B) regardless of cached color.
     * @pre   @ref init must have succeeded.
     */
    void inverse(void);

    /**
     * @brief Return the current ON/OFF state.
     * @retval true  Output corresponds to cached color and at least one channel is active.
     * @retval false Output is OFF.
     */
    bool isOn(void) const { return _isOn; }

    /**
     * @brief Get the cached desired color (independent of wiring).
     * @param[out] r Desired red channel state.
     * @param[out] g Desired green channel state.
     * @param[out] b Desired blue channel state.
     */
    void getColor(bool &r, bool &g, bool &b) const;

    bool getInitFlag(void) { return _initFlag;};
    
    // --------------------------------------------------------------------------
    // PWM / 8-bit color (optional)
    // --------------------------------------------------------------------------

    /**
     * @brief Set 8-bit color (0..255 per channel).
     * @pre   @ref init must have succeeded.
     * @param r Red   intensity (0..255)
     * @param g Green intensity (0..255)
     * @param b Blue  intensity (0..255)
     * @note  If PWM is disabled, non-zero values map to ON for the corresponding channel.
     * @sa    @ref enablePWM, @ref setBrightness
     */
    void setRGB(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Enable/disable PWM path. If disabled, uses digital writes (ON/OFF).
     * @param en true to enable PWM; false to disable.
     */
    void enablePWM(bool en) { _pwmEnabled = en; }
    
    /**
     * @brief Global brightness (0..255), only affects PWM path.
     * @param b Brightness scale (0 = off, 255 = full).
     */
    void setBrightness(uint8_t b) { _brightness = b; }

    // -----------------------------------------------------------------------
    // Blink utilities
    // -----------------------------------------------------------------------

    /**
     * @brief Blink the LED.
     *
     * @param duration      If @p number > 0: total sequence time in milliseconds.
     *                      If @p number == 0 (infinite mode): half-period in milliseconds
     *                      (i.e., time for each ON or OFF interval).
     * @param number        Number of ON/OFF cycles. Use 0 for infinite blinking.
     * @param blockingMode  If true and @p number > 0: block until finished.
     *                      Ignored when @p number == 0 (always non-blocking).
     *
     * @note In non-blocking mode, call @ref blinkUpdate() regularly in @c loop().
     * @note Use @ref stopBlink() to end infinite blinking.
     */
    void blink(uint16_t duration, uint8_t number, bool blockingMode = true);

    /**
     * @brief Stop non-blocking blink immediately.
     * @param turnOff If true, forces LED OFF; otherwise leaves current state as-is.
     */
    void stopBlink(bool turnOff = true);

    /**
     * @brief Get blinking status (only meaningful in non-blocking mode).
     * @return true if blinking is in progress; false if finished or idle.
     */
    bool isBlinking(void) {return _blinkActive;};

    /**
     * @brief Progress blinking sequence in non-blocking mode.
     * @details Should be called regularly in @c loop() or a periodic scheduler.
     */
    void blinkUpdate(void);

    // -----------------------------------------------------------------------
    // Color presets
    // -----------------------------------------------------------------------

    /** @brief Set LED to Red. */
    void red(void);

    /** @brief Set LED to Green. */
    void green(void);

    /** @brief Set LED to Blue. */
    void blue(void);

    /** @brief Set LED to Yellow (Red+Green). */
    void yellow(void);

    /** @brief Set LED to Purple (Red+Blue). */
    void purple(void);

    /** @brief Set LED to Cyan (Green+Blue). */
    void cyan(void);

    /** @brief Set LED to White (Red+Green+Blue). */
    void white(void);

  private:

    // -----------------------------------------------------------------------
    // Internal state
    // -----------------------------------------------------------------------

    uint8_t _onLevel = 1; ///< Logical ON level for pins (1 for CC, 0 for CA).

    bool _redDesired   = false; ///< Desired state of red channel.
    bool _greenDesired = false; ///< Desired state of green channel.
    bool _blueDesired  = false; ///< Desired state of blue channel.

    bool _isOn = false;       ///< True if LED is currently ON with cached color.
    bool _initFlag = false;   ///< True after successful init().

    // Non-blocking blink state
    bool     _blinkActive   = false; ///< True if non-blocking blink in progress.
    uint8_t  _blinkNumber   = 0;     ///< Number of requested cycles.
    uint8_t  _blinkEdgeCnt  = 0;     ///< Counts ON/OFF edges.
    uint32_t _blinkDelayMs  = 0;     ///< Base half-period in ms.
    uint16_t _blinkRemainder = 0;    ///< Edges that get +1 ms to hit exact total duration.
    uint32_t _tRef          = 0;     ///< Last toggle timestamp.

    uint32_t _currentDelay  = 0;     ///< Half-period used for the *next* edge.

    // ---------- Optional PWM / brightness ----------
    bool    _pwmEnabled   = false;
    uint8_t _brightness   = 255;     ///< 0..255 scales PWM output.
    uint8_t _r8 = 0, _g8 = 0, _b8 = 0; ///< cached 8-bit desired color

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Validate configuration parameters.
     * @return true if all parameters are valid, false otherwise.
     */
    bool _checkParameters(void);

    /**
     * @brief Apply current cached color to hardware according to wiring mode and PWM flag.
     */
    void _applyOutputs();

};

