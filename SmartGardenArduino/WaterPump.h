/**
 * Clase para bomba de agua
 * 
 */
#ifndef WaterPump_h
#define WaterPump_h
#include "Arduino.h"

// WATER PUMP STATES
#define WATER_PUMP_STATE_OPEN 1
#define WATER_PUMP_STATE_CLOSED 2

class WaterPump {

  private:
    int _pin; // Water Pump PIN
    int _pin2; // Water Pump PIN
    int16_t _state = WATER_PUMP_STATE_CLOSED; // Default STATE
    unsigned long _openAt; // for flow measure
    unsigned long _closedAt; // for flow measure
  public:
    WaterPump(int pin, int pin2);
    void openPump();
    void closePump();
    int getState();
    float getLastFlowMs();

};

#endif
