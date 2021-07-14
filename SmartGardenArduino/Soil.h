/**
 * Clase para sensor de humedad de suelo
 * 
 */
#include "Arduino.h"

 
class Soil {

  private:
    int _pin; // ANALOG PIN
    int _map_dry; // DRY
    int _map_wet; // WET
    
  public:
    Soil(int pin, int dry, int wet);
    int getMoisture();
    float getMoisturePercentage();
};
