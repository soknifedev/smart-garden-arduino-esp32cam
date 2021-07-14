/**
 * Clase para manejar la conectividad WiFi
 * 
 */
#ifndef SGWiFi_h
#define SGWiFi_h
#include "Arduino.h"
#include <WiFi.h>

class SGWiFi {

  private:
    const char* _ssid;
    const char* _passwd;
    unsigned long _keepAlivePreviousMs = millis();
    
  public:
    SGWiFi();
    SGWiFi(const char* wifi_ssid, const char* wifi_passwd);
    static void OnWiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info);
    static void OnWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    void connect();
    bool isConnected();
    uint8_t getState();
    void keepAlive(unsigned long interval);
};
#endif
