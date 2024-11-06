
#ifndef RGBLED_H
#define RGBLED_H

// ########################################################################################
// Include libraries:

#include <Arduino.h>

// ########################################################################################
// Define public macros:

#define RGBLED_COMMON_CATHODE      0         // Common pin connected to GND
#define RGBLED_COMMON_ANODE        1         // Common pin connected to VCC

// ########################################################################################
// Define Public classes:

/**
  @class RGBLED
  @brief Class for manage RGB LED.
*/
class RGBLED
{
  public:

    /// @brief Last error accured for object.
    String errorMessage;

    /**
      @struct ParametersStructure
      @brief Parametrers structure.
    */
    struct ParametersStructure
    {
      /**
        @brief RGB LED common mode anode/kathod. 
      */
      uint8_t COMMON_STATE;

      /**
        @brief 
      */
      int8_t RED_PIN;

      /**
        @brief 
      */
      int8_t GREEN_PIN;

      /**
        @brief 
      */
      int8_t BLUE_PIN;
    }parameters;

    /**
      @brief Default constructor. Init some variables.
    */
    RGBLED();

    /**
      @brief Detructor
    */
    ~RGBLED();

    /**
      @brief Init object. Check parameters.
      @return true if succeeded.
    */
    bool init(void);

    /**
      @brief Set RGB LED light to custom color.
    */
    void set(bool redState, bool greenState, bool blueState);

    /**
      @brief Set RGB LED light to off.
    */
    void off(void);

    /**
      @brief Set RGB LED light to red.
    */
    void red(void);

    /**
      @brief Set RGB LED light to green.
    */
    void green(void);

    /**
      @brief Set RGB LED light to blue.
    */
    void blue(void);

    /**
      @brief Set RGB LED light to yellow.
    */
    void yellow(void);

    /**
      @brief Set RGB LED light to purple.
    */
    void purple(void);

    /**
      @brief Set RGB LED light to cyan.
    */
    void cyan(void);

    /**
      @brief Set RGB LED light to white.
    */
    void white(void);

  private:

    /**
      @brief This indicates digital outputs mode for on led. 
      @note 0: LED on when output is 0.    
      @note 1: LED on when output is 1.
    */
    bool _onState;

    /**
      @brief Check parameters validation.
      @return true if succeeded.
    */
    bool _checkParameters(void);

};


















#endif