# <RGBLED.h> Library For Arduino

- This is simple library for contorl RGB LED light.   
- Just certain color can controled. Includes: OFF, RED, GREEN, BLUE, YELLOW, PURPLE, CYNE, WHITE.

-----------------------------------------------------

## Public Member Functions

```c++
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
    
```

## Public Member Variables

```c++
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

```