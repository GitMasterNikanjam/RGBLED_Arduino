#include "WString.h"
#ifndef RGBLED_H
#define RGBLED_H

#include <Arduino.h>

#define RGB_COMMON_CATHODE      0         // Common pin connected to GND
#define RGB_COMMON_ANODE        1         // Common pin connected to VCC

class RGBLED
{
  public:

    String errorMessage;

    struct ParametersStruct
    {
      // 
      uint8_t COMMON_STATE;

      int8_t RED_PIN;
      int8_t GREEN_PIN;
      int8_t BLUE_PIN;
    }parameters;

    RGBLED();

    ~RGBLED();

    bool init(void);

    void set(bool redState, bool greenState, bool blueState);

    void red(bool state);

    void green(bool state);

    void blue(bool state);

    void yellow(bool state);

    void purple(bool state);

    void cyan(bool state);

    void white(bool state);

  private:

    bool _onState;

    bool CheckParameters(void);

};


















#endif