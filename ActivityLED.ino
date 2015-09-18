/*
  Test TCLite on Arduino Due

  Mario Ban, 05.2015

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

ActivityLED::ActivityLED()
  : _pin(LED_BUILTIN)
  , _interval(1000)
  , _timeStamp(0)
  , _toggle(false)
  , _enabled(false)
{
}

void ActivityLED::setup(unsigned char pin, unsigned long interval)
{
  if(interval >= 100) {
    _pin = pin;
    _interval = interval;
    pinMode(_pin, OUTPUT);
    _enabled = true;
  }
  else {
    _enabled = false;
  }
}

void ActivityLED::process(unsigned long now)
{
  if(!_enabled) {
    return;
  }
  if(now - _timeStamp > _interval) {
    _timeStamp = now;
    if(_toggle) {
      digitalWrite(_pin, HIGH);
      //TCL_LogInfo("ActivityLED HIGH");
    }
    else {
      digitalWrite(_pin, LOW);
      //TCL_LogInfo("ActivityLED LOW");
    }
    _toggle = !_toggle;
  }
}

