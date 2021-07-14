/**
 * Clase para enviar datos al "Smart Garden Server"
 * 
 */
#ifndef SGClient_h
#define SGClient_h
#include "Arduino.h"
#include "esp_camera.h"
#include <PubSubClient.h>
#include "sg_structures.h"
#include <WiFi.h>

class SGClient {

  private:
    PubSubClient mqtt_client;
    const char* _mqttHost;
    const char* _mqttUser;
    const char* _mqttPasswd;
    int _mqttPort;
    const char* TOPIC_CAMERA = "camera/shot";
    const char* TOPIC_SENSORS = "sensors/fetch";

  public:
    static const char* MQTT_TOPIC_LOG_MESSAGE;
    static const char* MQTT_TOPIC_AMBIENT_IMAGE;
    static const char* MQTT_TOPIC_SENSORS_TEMPERATURE;
    static const char* MQTT_TOPIC_SENSORS_SOILMOISTURE;
    static const char* MQTT_TOPIC_ACTUATORS_WATERPUMPSTATE;
    static const char* MQTT_TOPIC_ACTUATORS_WATERPUMPFLOW;
    SGClient(const char* &host, const int &port, const char* &user, const char* &password, WiFiClient &wifi);
    void initialize();
    static void onMqttMessage(String topic, byte* message, unsigned int length);
    void sendLogEvent(const String &message);
    void sendSensorData(sensor_data &sensorData);
    void sendImageCapture(camera_fb_t* image, sensor_data &sensorData);
    void loop();
};
#endif
