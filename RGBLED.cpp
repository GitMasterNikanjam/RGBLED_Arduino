
// #################################################################################
// Include libraries:

#include "RGBLED.h"

// ##################################################################################
// RGBLED Class:

RGBLED::RGBLED()
{
  parameters.RED_PIN = -1;
  parameters.GREEN_PIN = -1;
  parameters.BLUE_PIN = -1;

  parameters.COMMON_STATE = 0;
}

RGBLED::~RGBLED()
{
  off();

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

  if(_checkParameters() == false)
  {
    return false;
  }

  if(parameters.COMMON_STATE == RGBLED_COMMON_CATHODE)
  {
    _onState = 1;
  }
  else if(parameters.COMMON_STATE == RGBLED_COMMON_ANODE)
  {
    _onState = 0;
  }
  else 
  {
    errorMessage = "Error RGBLED: Common mode value is not correct.";
    return false;;
  }

  pinMode(parameters.RED_PIN, OUTPUT);
  pinMode(parameters.GREEN_PIN, OUTPUT);
  pinMode(parameters.BLUE_PIN, OUTPUT);

  off();

  return true;
}

void RGBLED::set(bool redState, bool greenState, bool blueState)
{
  if(_onState == RGBLED_COMMON_CATHODE)
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


void RGBLED::off(void)
{
  set(0,0,0);
}

void RGBLED::red(void)
{
  set(1,0,0);
}

void RGBLED::green(void)
{
  set(0,1,0);
}

void RGBLED::blue(void)
{
  set(0,0,1);
}

void RGBLED::yellow(void)
{
  set(1,1,0);
}

void RGBLED::purple(void)
{
  set(1,0,1);
}

void RGBLED::cyan(void)
{
  set(0,1,1);
}

void RGBLED::white(void)
{
  set(1,1,1);
}

bool RGBLED::_checkParameters(void)
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