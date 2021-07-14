#include "Arduino.h"
#include "WaterPump.h"

/**
 * Constructor
 * @param pin Pin digital 
 * @param pin2 Pin digital 
 */
WaterPump::WaterPump(int pin, int pin2) {
  _pin = pin;
  _pin2 = pin2;
  pinMode(_pin,OUTPUT);
  pinMode(_pin2,OUTPUT);
}

/**
 * enable Activa la bomba de agua
 */
void WaterPump::openPump() {
  _state = WATER_PUMP_STATE_OPEN;
  _openAt = millis();
  digitalWrite(_pin,HIGH);
  digitalWrite(_pin2,HIGH);
}

/**
 * close Cierra la bomba de agua
 */
void WaterPump::closePump() {
  _state = WATER_PUMP_STATE_CLOSED;
  digitalWrite(_pin,LOW);
  digitalWrite(_pin2,LOW);
  _closedAt = millis();
}

/**
 * @return Flujo de agua desde la ultima vez que se åctivó la bomba
 */
float WaterPump::getLastFlowMs() {

  // La mini bomba de agua tiene un caudal de 120L/hora (0.033L/segundo).
  float _waterFlowMs = 0.000033; // litros por milisegundo
  unsigned long elapsed; // tiempo transcurrido
  // Verificar si la bomba ya está cerrada o no
  if(_state == WATER_PUMP_STATE_CLOSED) {
    elapsed =  _closedAt - _openAt;
  }
  else {
    elapsed = millis() - _openAt;
  }
  return _waterFlowMs * elapsed;
}

int16_t WaterPump::getState() {
  return _state;
}
