#include "Arduino.h"
#include "Ambient.h"
#include <DallasTemperature.h>

Ambient::Ambient(int pin) {
  _pin = pin;
  _onewire.begin(_pin);
  _sensors.setOneWire(&_onewire); //
}

void Ambient::init() {
  _sensors.begin(); //
  Serial.print("Found ");
  Serial.print(_sensors.getDeviceCount(), DEC);
  Serial.println(" sensor devices.");
  Serial.print("Parasite power is: "); 
  if (_sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
}

float Ambient::getTemperature() {
  _sensors.requestTemperatures();
  return _sensors.getTempCByIndex(0);
}
