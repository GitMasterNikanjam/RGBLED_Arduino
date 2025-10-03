
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
  parameters.ACTIVE_MODE = RGBLED_ACTIVE_HIGH;
}

RGBLED::~RGBLED()
{
  off();

  // Optional: first put pins Hi-Z
  if (parameters.RED_PIN   != -1) pinMode(parameters.RED_PIN,   INPUT);
  if (parameters.GREEN_PIN != -1) pinMode(parameters.GREEN_PIN, INPUT);
  if (parameters.BLUE_PIN  != -1) pinMode(parameters.BLUE_PIN,  INPUT);

}

bool RGBLED::init(void)
{
  if (!_checkParameters()) return false;

  lastError = RGBLED_OK;

  // Compute logical "on" level for the output pins.
  // For active high: writing HIGH -> LED ON.
  // For active low:   writing LOW  -> LED ON.
  _onLevel = (parameters.ACTIVE_MODE == RGBLED_ACTIVE_HIGH) ? 1 : 0;

  // --------- Glitch-free init: preload OFF level before switching to OUTPUT
  const uint8_t offLevel = (_onLevel ? LOW : HIGH);
  digitalWrite(parameters.RED_PIN,   offLevel);
  digitalWrite(parameters.GREEN_PIN, offLevel);
  digitalWrite(parameters.BLUE_PIN,  offLevel);

  pinMode(parameters.RED_PIN, OUTPUT);
  pinMode(parameters.GREEN_PIN, OUTPUT);
  pinMode(parameters.BLUE_PIN, OUTPUT);

  _redDesired = _greenDesired = _blueDesired = false;
  _isOn = false;
  _blinkActive = false;
  _r8 = _g8 = _b8 = 0;

  _initFlag = true;      // allow blink() etc.
  off();                 // now effective (or delete this line entirely)

  return true;
}

void RGBLED::set(bool redState, bool greenState, bool blueState)
{
  if (!_initFlag) return;  // Guard (1)

  _redDesired   = redState;
  _greenDesired = greenState;
  _blueDesired  = blueState;

  // Map boolean -> 8-bit desired cache for unified path
  _r8 = _redDesired   ? 255 : 0;
  _g8 = _greenDesired ? 255 : 0;
  _b8 = _blueDesired  ? 255 : 0;
  _applyOutputs();

  _isOn = true;  // reflect that the cached color is now being shown
}


void RGBLED::off(void)
{
  if (!_initFlag) return;  // Guard (1)

  _isOn = false;
  // OFF means "not lit" regardless of wiring:
  // For CC: write LOW; For CA: write HIGH.
  const uint8_t offLevel = (_onLevel ? LOW : HIGH);
  
  if (_pwmEnabled) 
  {
    analogWrite(parameters.RED_PIN,   (_onLevel ? 0   : 255)); // 0 duty = off for CC; 255 for CA
    analogWrite(parameters.GREEN_PIN, (_onLevel ? 0   : 255));
    analogWrite(parameters.BLUE_PIN,  (_onLevel ? 0   : 255));
  } 
  else 
  {
    digitalWrite(parameters.RED_PIN,   offLevel);
    digitalWrite(parameters.GREEN_PIN, offLevel);
    digitalWrite(parameters.BLUE_PIN,  offLevel);
  }
}

void RGBLED::on() 
{
  if (!_initFlag) return;  // Guard (1)

  _applyOutputs();
  _isOn = ((_r8 | _g8 | _b8) != 0);
}

void RGBLED::toggle() 
{
  if (!_initFlag) return;  // Guard (1)

  if (_isOn) off();
  else       on();
}

void RGBLED::inverse(void)
{   
  if (!_initFlag) return;  // Guard (1)

  // Invert each desired channel and then show it.
  set(!_redDesired, !_greenDesired, !_blueDesired);
}

void RGBLED::setRGB(uint8_t r, uint8_t g, uint8_t b)
{
  if (!_initFlag) return;  // Guard (1)
  _r8 = r; _g8 = g; _b8 = b;
  // Keep logical booleans in sync for callers that read them
  _redDesired   = (r != 0);
  _greenDesired = (g != 0);
  _blueDesired  = (b != 0);
  _applyOutputs();
  _isOn = (_r8 | _g8 | _b8) != 0;
}

void RGBLED::_applyOutputs()
{
  // Brightness scaling (0..255)
  uint16_t r = (uint16_t)_r8 * _brightness / 255;
  uint16_t g = (uint16_t)_g8 * _brightness / 255;
  uint16_t b = (uint16_t)_b8 * _brightness / 255;

  if (_pwmEnabled) {
    // Apply wiring mode (CA inverts)
    uint8_t pr = _onLevel ? r : (255 - r);
    uint8_t pg = _onLevel ? g : (255 - g);
    uint8_t pb = _onLevel ? b : (255 - b);
    analogWrite(parameters.RED_PIN,   pr);
    analogWrite(parameters.GREEN_PIN, pg);
    analogWrite(parameters.BLUE_PIN,  pb);
  } else {
    // Digital writes: non-zero -> ON
    const bool rOn = (_onLevel ? (r != 0) : (r == 0));
    const bool gOn = (_onLevel ? (g != 0) : (g == 0));
    const bool bOn = (_onLevel ? (b != 0) : (b == 0));
    digitalWrite(parameters.RED_PIN,   rOn ? HIGH : LOW);
    digitalWrite(parameters.GREEN_PIN, gOn ? HIGH : LOW);
    digitalWrite(parameters.BLUE_PIN,  bOn ? HIGH : LOW);
  }
}

void RGBLED::getColor(bool &r, bool &g, bool &b) const
{
  r = _redDesired;
  g = _greenDesired;
  b = _blueDesired;
}

void RGBLED::stopBlink(bool turnOff)
{
  _blinkActive = false;
  _blinkEdgeCnt = 0;
  _blinkNumber = 0;
  _blinkRemainder = 0;
  if (turnOff) off();
  else _applyOutputs();
}

void RGBLED::blink(uint16_t duration_ms, uint8_t number, bool blocking) 
{
    if (!_initFlag) return;

    // duration_ms must be > 0 in all modes
    if (duration_ms == 0) 
    {
        _blinkActive = false;
        off();
        return;
    }

    // -------- Infinite mode: number == 0 --------
    if (number == 0) 
    {
        // Interpret duration_ms as HALF-PERIOD
        _blinkNumber    = 0;            // sentinel: infinite
        _blinkEdgeCnt   = 0;
        _blinkActive    = true;
        _blinkDelayMs   = duration_ms;  // fixed half-period
        _blinkRemainder = 0;            // not used in infinite mode
        _currentDelay   = _blinkDelayMs;
        _tRef           = millis();

        // Start from ON (show cached color)
        on();
        return;
    }

    // -------- Finite mode (existing behavior) --------
    const uint32_t edges = 2UL * number;              // ON + OFF edges
    _blinkDelayMs   = (uint32_t)(duration_ms / edges);
    _blinkRemainder = (uint16_t)(duration_ms % edges); // spread +1ms over first remainder edges
    if (_blinkDelayMs == 0 && edges) { _blinkDelayMs = 1; } // avoid zero delay
    _currentDelay = _blinkDelayMs + (_blinkRemainder ? 1 : 0);

    if (blocking) 
    {
        const bool r = _redDesired, g = _greenDesired, b = _blueDesired;

        uint16_t rem = _blinkRemainder;
        for (uint8_t i = 0; i < number; ++i) {
            set(r, g, b);
            delay(_blinkDelayMs + (rem ? 1 : 0)); if (rem) --rem;
            off();
            delay(_blinkDelayMs + (rem ? 1 : 0)); if (rem) --rem;
        }
    } 
    else 
    {
        _blinkNumber  = number;
        _blinkEdgeCnt = 0;
        _blinkActive  = true;
        _tRef         = millis();
        on();
    }
}

void RGBLED::blinkUpdate() 
{
  if (!_blinkActive || !_initFlag) return; // Guard (1)

  const uint32_t now = millis();
  if ((uint32_t)(now - _tRef) < _currentDelay) return;

  // time for next edge
  toggle();
  _tRef = now;
  _blinkEdgeCnt++;
  if (_blinkRemainder) { _blinkRemainder--; }
  _currentDelay = _blinkDelayMs + (_blinkRemainder ? 1 : 0);

  // Stop only in FINITE mode (when _blinkNumber > 0)
  if (_blinkNumber > 0 && _blinkEdgeCnt >= (uint8_t)(2 * _blinkNumber)) {
      _blinkActive = false;
      off();
  }
}

void RGBLED::red()    { set(true,  false, false); }
void RGBLED::green()  { set(false, true,  false); }
void RGBLED::blue()   { set(false, false, true ); }
void RGBLED::yellow() { set(true,  true,  false); }
void RGBLED::purple() { set(true,  false, true ); }
void RGBLED::cyan()   { set(false, true,  true ); }
void RGBLED::white()  { set(true,  true,  true ); }

bool RGBLED::_checkParameters(void)
{
  const bool ok =
        (parameters.RED_PIN   >= 0) &&
        (parameters.GREEN_PIN >= 0) &&
        (parameters.BLUE_PIN  >= 0) &&
        (parameters.ACTIVE_MODE == RGBLED_ACTIVE_HIGH ||
         parameters.ACTIVE_MODE == RGBLED_ACTIVE_LOW);

  if (!ok) { lastError = RGBLED_ERR_PARAMS; }

  return ok;
}