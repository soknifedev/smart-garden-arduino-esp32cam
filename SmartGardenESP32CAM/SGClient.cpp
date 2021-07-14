#include "Arduino.h"
#include "esp_camera.h"
#include "sg_structures.h"
#include "SGClient.h"
#include <PubSubClient.h>
#include <WiFi.h>

const char* SGClient::MQTT_TOPIC_LOG_MESSAGE   = "log/esp32cam";
const char* SGClient::MQTT_TOPIC_AMBIENT_IMAGE   = "sensors/ambient/image";
const char* SGClient::MQTT_TOPIC_SENSORS_TEMPERATURE = "sensors/ambient/temperature";
const char* SGClient::MQTT_TOPIC_SENSORS_SOILMOISTURE = "sensors/soil/moisture";
const char* SGClient::MQTT_TOPIC_ACTUATORS_WATERPUMPSTATE = "actuators/waterPump/state";
const char* SGClient::MQTT_TOPIC_ACTUATORS_WATERPUMPFLOW = "actuators/waterPump/flow";

SGClient::SGClient(const char* &host, const int &port, const char* &user, const char* &password, WiFiClient &wifi) {
  _mqttHost = host;
  _mqttPort = port;
  _mqttUser = user;
  _mqttPasswd = password;
  mqtt_client.setClient(wifi);
  mqtt_client.setServer(_mqttHost, _mqttPort);
  mqtt_client.setCallback(SGClient::onMqttMessage);
}


void SGClient::loop() {
  mqtt_client.loop();
}

void SGClient::initialize() {
  Serial.println("SGClient initializing...");
  Serial.print("Buffer before resize: ");
  Serial.println(mqtt_client.getBufferSize()); 
  boolean res = mqtt_client.setBufferSize(38000); // 38kb
  if (res) {
    Serial.print("Buffer resized: ");
    Serial.println(mqtt_client.getBufferSize()); 
  }
  else { 
    Serial.print("Buffer could not be resized: ");
    Serial.println(mqtt_client.getBufferSize()); 
  }  
  Serial.println("SGClient setup ok"); 
  while (!mqtt_client.connected()) {
    String clientId = "ESP32CAM-";
    clientId += WiFi.localIP();
    Serial.println("Connecting to MQTT broker: ");
    Serial.print(_mqttHost);
    Serial.print(_mqttPort);
    Serial.print(" with ");
    Serial.print(_mqttUser);
    Serial.print(_mqttPasswd);
    Serial.print(" clientId=");
    Serial.print(clientId);
    Serial.println("...");
    if (mqtt_client.connect(clientId.c_str(), _mqttUser, _mqttPasswd )) {
      Serial.println("MQTT Connection successed.");
    } else {
      Serial.print("MQTT Connection Failed: ");
      Serial.print(mqtt_client.state());
      delay(2000);
      Serial.println("Retrying connection...");
    }
  }
  // El modo suscriptor no se utilizó en el circuito de la investigación
  // Sin embargo, fue agregado para investigadores futuros
  mqtt_client.subscribe("garden/settings/data/interval");
  mqtt_client.subscribe("garden/request/image");
  mqtt_client.subscribe("garden/request/sensors");
}

void SGClient::sendLogEvent(const String &message) {
  Serial.print("sendLogMessage: ");
  Serial.println(message);

  mqtt_client.publish(MQTT_TOPIC_LOG_MESSAGE, message.c_str());
}

void SGClient::sendSensorData(sensor_data &sensorData) {
  //sendLogEvent("Executing sendSensorData...");

  String temperature = "", soilMoisture = "", waterPumpState = "", waterPumpFlow = "";
  temperature += sensorData.temperature;
  soilMoisture += sensorData.soilMoisture;
  waterPumpState += sensorData.waterPumpState;
  waterPumpFlow += sensorData.waterPumpLastFlow;
  mqtt_client.publish(MQTT_TOPIC_SENSORS_TEMPERATURE, temperature.c_str());
  mqtt_client.publish(MQTT_TOPIC_SENSORS_SOILMOISTURE, soilMoisture.c_str());
  mqtt_client.publish(MQTT_TOPIC_ACTUATORS_WATERPUMPSTATE, waterPumpState.c_str());
  mqtt_client.publish(MQTT_TOPIC_ACTUATORS_WATERPUMPFLOW, waterPumpFlow.c_str());
}

void SGClient::sendImageCapture(camera_fb_t* image, sensor_data &sensorData) {
  sendSensorData(sensorData); // Record sensor data
  // sendLogEvent("sendImageCapture needs to be coded");
  if (image->len) {
    Serial.println("> uploading image....");
    boolean published = mqtt_client.publish_P(MQTT_TOPIC_AMBIENT_IMAGE, image->buf, image->len, false);
    if (!published)
    {
      // error!
      Serial.println(">> Error sending picture.");
      sendLogEvent("Could not publish image on mqtt topic.");
    } else {
      Serial.print("Image sent, was ");
      Serial.print(image->len);
      Serial.println(" bytes!");
      sendLogEvent("Image sent, bytes -> " + String(image->len));
    }

  } else {
    Serial.print("Image could not be sent because image-len is wrong! -> ");
    Serial.println(image->len);
    sendLogEvent("Image could not be sent because image-len is wrong! -> " + String(image->len));
  }
  esp_camera_fb_return(image);
}
