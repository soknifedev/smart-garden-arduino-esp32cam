#include "Soil.h"
#include "Ambient.h"
#include "WaterPump.h"

// Sender
#include "SoftwareSerial.h"         
#include "SerialTransfer.h"
#include "sg_structures.h"

SoftwareSerial mySerial(6, 7); // RX-14, TX-15
SerialTransfer myTransfer;

sensor_data sensorData;

#define SG_ACTION_IDLE  1
#define SG_ACTION_TAKE_PICTURE 2
#define SG_ACTION_RECORD_DATA  3


#define PIN_SENSOR_SOIL_MOISTURE A0 // ANALOGICO
#define PIN_SENSOR_TEMPERATURE 2 // DIGITAL

#define PIN_SENSOR_WATER_PUMP_1 3 // DIGITAL
#define PIN_SENSOR_WATER_PUMP_2 4 // DIGITAL


#define VALUE_SENSOR_SOIL_DRY 860
#define VALUE_SENSOR_SOIL_WET 580

Soil soil(PIN_SENSOR_SOIL_MOISTURE, VALUE_SENSOR_SOIL_DRY, VALUE_SENSOR_SOIL_WET);
Ambient ambient(PIN_SENSOR_TEMPERATURE);
WaterPump waterPump(PIN_SENSOR_WATER_PUMP_1, PIN_SENSOR_WATER_PUMP_2);

class SmartGarden
{

  public:
    bool needsWater() {
      return soil.getMoisturePercentage() <= 50;
    }
};

SmartGarden garden;


void setup() {
  Serial.begin(115200);
  Serial.println("Starting SmartGardenArduino...");
  mySerial.begin(115200); // bauds, rx, tx
  myTransfer.begin(mySerial);
  ambient.init();

}
 
void loop() {

  if(garden.needsWater() && waterPump.getState() != WATER_PUMP_STATE_OPEN) {
    Serial.println("Garden needs water, opening pump...");
    waterPump.openPump();
  } else {
    //Serial.println("Garden does not need water, closing pump...");
    waterPump.closePump();
  }

  // SG_ACTION_RECORD_DATA, SG_ACTION_IDLE, SG_ACTION_TAKE_PICTURE
  // SG_ACTION_IDLE hará que se seteen los datos de los sensores en el ESP32CAM
  // pero no se envie ningun dato al servidor MQTT. 
  // El modulo ESP32CAM entra en modo suscriptor a dos topics para dicho fin.
  sensorData.action  = SG_ACTION_IDLE; // no hacer nada en esp32cam
 
  sensorData.temperature = ambient.getTemperature();
  sensorData.soilMoisture  = soil.getMoisture();
  sensorData.waterPumpState = waterPump.getState();
  sensorData.waterPumpLastFlow = waterPump.getLastFlowMs();

  // Serial.print("Water Pump state:");
  // Serial.println(sensorData.waterPumpState);
  uint16_t sendSize = 0;
  sendSize = myTransfer.txObj(sensorData, sendSize);
  myTransfer.sendData(sendSize);
  Serial.print("Sent ");
  Serial.print(sendSize);
  Serial.println(" bytes through serial channel");
  delay(1000);

}
