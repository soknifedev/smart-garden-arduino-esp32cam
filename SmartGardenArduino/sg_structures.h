#ifndef SGStructures_h
#define SGStructures_h
typedef struct sensor_data {
  int16_t action;
  int16_t soilMoisture;
  int16_t waterPumpState;
  int16_t _break; // "truquito" para SerialTRansfer, sin el la struct no se recibe correctamente del otro lado!
  float temperature;
  float waterPumpLastFlow;
};

#endif
