#ifndef SGStructures_h
#define SGStructures_h
typedef struct sensor_data {
  int16_t action;
  int16_t soilMoisture;
  int16_t waterPumpState;
  // int16_t _break; // "truquito" no necesario de este lado!
  float temperature;
  float waterPumpLastFlow;
};

#endif
