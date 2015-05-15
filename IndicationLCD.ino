/*
  Test TCLite on Arduino Due

  Mario Ban, 05.2015

*/

IndicationLCD::IndicationLCD()
  : _lcd(0)
  , _col(0)
  , _row(0)
  , _indChar('I')
  , _indDelay(300)
  , _timeStamp(0)
  , _on(false)
  , _doShow(false)
  , _enabled(false)
{
}

void IndicationLCD::setup(LiquidCrystal_I2C* lcd, unsigned char col, unsigned char row, char indChar, unsigned long indDelay)
{
  if(indDelay >= 100) {
    _lcd = lcd;
    _col = col;
    _row = row;
    _indChar = indChar;
    _indDelay = indDelay;
    _enabled = true;
  }
  else {
    _enabled = false;
  }
}

void IndicationLCD::show(unsigned long now)
{
  if(!_enabled) {
    return;
  }
  _timeStamp = now;
  if(!_on) {
    _on = true;
    _lcd->setCursor(_col, _row);
    _lcd->print(_indChar);
  }
}

void IndicationLCD::show()
{
  if(!_enabled) {
    return;
  }
  _doShow = true; // wait till next process call
}

void IndicationLCD::hide()
{
  if(!_enabled) {
    return;
  }
  _doShow = false;
  _timeStamp -= _indDelay; // wait till next process call
}

void IndicationLCD::process(unsigned long now)
{
  if(!_enabled) {
    return;
  }
  if(_doShow) {
    _doShow = false;
    show(now);
  }
  if(_on && now - _timeStamp > _indDelay) {
    _on = false;
    _lcd->setCursor(_col, _row);
    _lcd->print(' ');
  }
}

