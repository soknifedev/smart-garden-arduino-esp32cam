#include "Arduino.h"
#include "SGWiFi.h"
#include <WiFi.h>


SGWiFi::SGWiFi() {
  WiFi.onEvent(SGWiFi::OnWiFiConnected, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(SGWiFi::OnWiFiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}

SGWiFi::SGWiFi(const char* wifi_ssid, const char* wifi_passwd) {
  _ssid = wifi_ssid;
  _passwd = wifi_passwd;
  WiFi.onEvent(SGWiFi::OnWiFiConnected, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(SGWiFi::OnWiFiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}

void SGWiFi::connect() {
  WiFi.disconnect(true);
  delay(1000);
  WiFi.begin(_ssid, _passwd);
  Serial.print("Connecting to WiFi");
  while (!this->isConnected()) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

uint8_t SGWiFi::getState() {
  return WiFi.status();
}

bool SGWiFi::isConnected() {
  return (getState() == WL_CONNECTED);
}

/**
 * keepAlive verifica la conexión WiFi e intenta reconectar
 * @param interval cantidad de tiempo entre cada chequeo
 * Se recomiendan 30 segundos
 */
void SGWiFi::keepAlive(unsigned long interval) {
  unsigned long currentMs = millis();

  if (!isConnected() && (currentMs - _keepAlivePreviousMs >= interval)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    _keepAlivePreviousMs = currentMs;
  }
}
