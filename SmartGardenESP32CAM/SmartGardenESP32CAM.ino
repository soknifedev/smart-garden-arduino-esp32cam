#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include "SerialTransfer.h"
#include <millisDelay.h>

millisDelay rebootDelay; // necesario para evitar errores con el paso del tiempo
millisDelay captureImage;
unsigned long REBOOT_DELAY_MS = 24 * 60 * 60 * 1000; // 1 dia
unsigned long CAPTURE_IMAGE_DELAY_MS = 30 * 60 * 1000; // 30 minutos

// Built-in led
#define ESP32CAM_LED_PIN 13

// Smart Garden Libraries
#include "sg_structures.h"
sensor_data sensorData;

#include "SGWiFi.h"
#include "SGClient.h"

#define SG_ACTION_IDLE  1
#define SG_ACTION_TAKE_PICTURE 2
#define SG_ACTION_RECORD_DATA  3

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

const char* ssid = "NOMBRE_DE_TU_RED_WIFI";
const char* password = "PASSWORD_DE_TU_RED_WIFI";
SGWiFi wifi(ssid, password);
WiFiClient wifiClient;

const char* mqttHost = "DIRECCION.IP.RASPBERRY.PI";
const int   mqttPort = 1883;
const char* mqttUser = "smartgarden";
const char* mqttPass = "sg2021sg";

SGClient sgClient(mqttHost, mqttPort, mqttUser, mqttPass, wifiClient);

// Serial para comunicación con arduino
SerialTransfer myTransfer;

void setup() {
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  // digitalWrite(ESP32CAM_LED_PIN, HIGH); //Turn on

  Serial.begin(115200);
  // Serial para comunicación a arduino
  Serial2.begin(115200, SERIAL_8N1, 15, 14); // TX, RX
  myTransfer.begin(Serial2);

  Serial.setDebugOutput(true);
  Serial.println();

  rebootDelay.start(REBOOT_DELAY_MS); // start reboot timer
  captureImage.start(CAPTURE_IMAGE_DELAY_MS); // start image capture timer

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_VGA or FRAMESIZE_UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Conexión WiFi
  wifi.connect();
  sgClient.initialize();
  //test

}

void loop() {
  if(wifi.isConnected()) 
  {
    sgClient.loop();
    if(myTransfer.available())  
    {

      uint16_t recvSize = 0;
      recvSize = myTransfer.rxObj(sensorData, recvSize);

      //Serial.print("Received: action=" + String(sensorData.action) + "; temp=");
      //Serial.println(sensorData.temperature);
      // sgClient.sendLogEvent("Received " + String(recvSize) + " bytes from serial channel");
      
      if(sensorData.action == SG_ACTION_IDLE) {
         // sgClient.sendLogEvent("Arduino said that I should stay idle.");
         Serial.print("Received ");
         Serial.print(recvSize);
         Serial.println(" bytes from arduino.");
      }
      else if(sensorData.action == SG_ACTION_TAKE_PICTURE) {
          camera_fb_t* image = getImageCapture();
          if(image) {
            sgClient.sendImageCapture(image, sensorData);
          } else {
            sgClient.sendLogEvent("Error al capturar la imagen");
          }
      }
      else if(sensorData.action == SG_ACTION_RECORD_DATA) {
          sgClient.sendSensorData(sensorData);
      }
      else {
          sgClient.sendLogEvent("Error, accion desconocida recibida: " + sensorData.action);
      }

    } else if(myTransfer.status < 0) {
      Serial.print("ERROR: ");
      if(myTransfer.status == -1) {
        sgClient.sendLogEvent("Error 'CRC_ERROR' en el serial channel!");
        Serial.print(F("CRC_ERROR"));
      }
      else if(myTransfer.status == -2) {
        sgClient.sendLogEvent("Error 'PAYLOAD_ERROR' en el serial channel!");
        Serial.print(F("PAYLOAD_ERROR"));
      }
      else if(myTransfer.status == -3) {
        sgClient.sendLogEvent("Error 'STOP_BYTE_ERROR' en el serial channel!");
        Serial.print(F("STOP_BYTE_ERROR"));
      }
    }/* else {
      sgClient.sendLogEvent("awaiting for data...");
      delay(1000);
    }*/

    if (captureImage.justFinished()) {
      camera_fb_t* image = getImageCapture();
      if(image) {
        sgClient.sendImageCapture(image, sensorData);
      } else {
        sgClient.sendLogEvent("Error al capturar la imagen");
      }
      captureImage.start(CAPTURE_IMAGE_DELAY_MS); // restart timer
    }
  }
  if (rebootDelay.justFinished()) {
    // reboot!!
    Serial.println("Reboot should happen now!");
  }
  // wifi.keepAlive(30000);
}

camera_fb_t* getImageCapture() {
  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  return fb;
}

void SGWiFi::OnWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  wifi.keepAlive(0);
}


void SGWiFi::OnWiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("WiFi IP Address: ");
  Serial.println(WiFi.localIP());
}

// El modo suscriptor en el modulo ESP32CAM sirve para enviar los datos y/o imagenes
// a internet cuando se publique en ellos cualquier cosa, a través del servidor MQTT. 
// El Arduino actualiza periódicamente estos datos a través del canal serial.
void SGClient::onMqttMessage(String topic, byte* message, unsigned int length) {
  if (topic == "garden/request/image") {
    Serial.println("> onMqttMessage: garden/request/image");
    camera_fb_t* image = getImageCapture();
    if(image) {
      sgClient.sendImageCapture(image, sensorData);
    } else {
      sgClient.sendLogEvent("Error al capturar la imagen");
    }
  }
  else if (topic == "garden/request/sensors") {
    Serial.println("> onMqttMessage: garden/request/sensors");
    sgClient.sendSensorData(sensorData);
  } else {
    Serial.print("> onMqttMessage: not handled topic received: ");
    Serial.println(topic); 
  }
}
