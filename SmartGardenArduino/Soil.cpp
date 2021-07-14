#include "Arduino.h"
#include "Soil.h"

/**
 * Constructor
 * @param pin Pin analogico del sensor
 * @param dry Valor de la tierra en estado de minima humedad (seca)
 * @param wet Valor de la tierra en estado de máxima humedad
 */
Soil::Soil(int pin, int dry, int wet) {
  _pin = pin;
  _map_dry = dry;
  _map_wet = wet;
}

/**
 * getMoisture realiza una lectura analógica del sensor
 * para devolver su valor actual.
 * @return int
 */
int Soil::getMoisture() {
  return analogRead(_pin);
}

/**
 * getMoisturePercentage Obtiene el porcentaje de humedad
 * en base a las variables _map_dry y _map_wet. 
 * @return float
 */
float Soil::getMoisturePercentage() {
  return map(getMoisture(), _map_dry, _map_wet, 0, 100);
}
