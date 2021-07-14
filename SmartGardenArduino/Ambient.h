/**
 * Clase para sensor de humedad de suelo
 * 
 */
#include "Arduino.h"
#include <DallasTemperature.h>
 
class Ambient {

  private:
    int _pin; // DIGITAL PIN
    OneWire _onewire; // 
    DallasTemperature _sensors; //
    
  public:
    Ambient(int pin);
    void init();
    float getTemperature();
};
