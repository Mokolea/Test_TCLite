/*
  Test TCLite on Arduino Due

  Mario Ban, 05.2015

  GNU General Public License v2.0
  Copyright (C) 2015 Mario Ban
*/

ActivityLCD::ActivityLCD()
  : _lcd(0)
  , _col(0)
  , _row(0)
  , _interval(1000)
  , _timeStamp(0)
  , _count(0)
  , _enabled(false)
{
}

void ActivityLCD::setup(LiquidCrystal_I2C* lcd, unsigned char col, unsigned char row, unsigned long interval)
{
  if(interval >= 100) {
    _lcd = lcd;
    _col = col;
    _row = row;
    _interval = interval;
    _enabled = true;
  }
  else {
    _enabled = false;
  }
}

void ActivityLCD::process(unsigned long now)
{
  if(!_enabled) {
    return;
  }
  if(now - _timeStamp > _interval) {
    _timeStamp = now;
    _lcd->setCursor(_col, _row);
    if((_count + 3) % 4 == 0) {
      _lcd->print(" ");
    }
    else if((_count + 2) % 4 == 0) {
      _lcd->print("x");
    }
    else if((_count + 1) % 4 == 0) {
      _lcd->print(" ");
    }
    else if(_count % 4 == 0) {
      _lcd->print("o");
    }
    else {
      _lcd->print("?"); /* never */
    }
    _count++;
  }
}

