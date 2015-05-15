/*
  Test TCLite on Arduino Due

  Mario Ban, 05.2015

*/

InputDebounce::InputDebounce()
  : _pinIn(0)
  , _stateOn(false)
  , _timeStamp(0)
  , _debDelay(0)
  , _stateOnCount(0)
  , _enabled(false)
{
}

void InputDebounce::setup(uint8_t pinIn, unsigned long debDelay)
{
  if(pinIn > 0) {
    _pinIn = pinIn;
    _debDelay = debDelay;
    pinMode(_pinIn, INPUT);
    _enabled = true;
  }
  else {
    _enabled = false;
  }
}

unsigned long InputDebounce::process(unsigned long now)   // return pressed time if on
{
  if(!_enabled) {
    return 0;
  }
  if(now - _timeStamp > _debDelay) {
    int value = digitalRead(_pinIn); // LOW when button pressed (on)
    if(_stateOn != !value) {
      _stateOn = !value;
      _timeStamp = now;
      if(_stateOn) {
        _stateOnCount++;
      }
      return _stateOn ? 1 : 0;
    }
  }
  return _stateOn ? now - _timeStamp : 0;
}

unsigned long InputDebounce::getStateOnCount()
{
  return _stateOnCount;
}

