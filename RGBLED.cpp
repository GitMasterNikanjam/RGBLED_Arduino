#include "RGBLED.h"


RGBLED::RGBLED()
{
  parameters.RED_PIN = -1;
  parameters.GREEN_PIN = -1;
  parameters.BLUE_PIN = -1;

  parameters.COMMON_STATE = 0;
}

RGBLED::~RGBLED()
{
  set(0,0,0);

  if(parameters.RED_PIN != -1)
  {
    pinMode(parameters.RED_PIN, INPUT);
  }

  if(parameters.GREEN_PIN != -1)
  {
    pinMode(parameters.GREEN_PIN, INPUT);
  }

  if(parameters.BLUE_PIN != -1)
  {
    pinMode(parameters.BLUE_PIN, INPUT);
  }

}

bool RGBLED::init(void)
{

  if(CheckParameters() == false)
  {
    return false;
  }

  if(parameters.COMMON_STATE == 0)
  {
    _onState = 1;
  }
  else
  {
    _onState = 0;
  }

  pinMode(parameters.RED_PIN, OUTPUT);
  pinMode(parameters.GREEN_PIN, OUTPUT);
  pinMode(parameters.BLUE_PIN, OUTPUT);

  set(0,0,0);

  return true;
}

void RGBLED::set(bool redState, bool greenState, bool blueState)
{
  if(_onState == RGB_COMMON_CATHODE)
  {
    digitalWrite(parameters.RED_PIN, redState);
    digitalWrite(parameters.GREEN_PIN, greenState);
    digitalWrite(parameters.BLUE_PIN, blueState);
  }
  else
  {
    digitalWrite(parameters.RED_PIN, !redState);
    digitalWrite(parameters.GREEN_PIN, !greenState);
    digitalWrite(parameters.BLUE_PIN, !blueState);
  }
}

void RGBLED::red(bool state)
{
  set(state,0,0);
}

void RGBLED::green(bool state)
{
  set(0,state,0);
}

void RGBLED::blue(bool state)
{
  set(0,0,state);
}

void RGBLED::yellow(bool state)
{
  set(state,state,0);
}

void RGBLED::purple(bool state)
{
  set(state,0,state);
}

void RGBLED::cyan(bool state)
{
  set(0,state,state);
}

void RGBLED::white(bool state)
{
  set(state,state,state);
}

bool RGBLED::CheckParameters(void)
{
  bool state = ( (parameters.COMMON_STATE <= 1) && (parameters.BLUE_PIN >= 0) &&
               (parameters.GREEN_PIN >= 0) && (parameters.RED_PIN >= 0) ); 

  if(state == false)
  {
    errorMessage = "Error RGBLED: One or Some parameters are not correct.";
    return false;
  }

  return true;
}