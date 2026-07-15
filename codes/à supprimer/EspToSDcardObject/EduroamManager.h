#ifndef EDUROAM_MANAGER_H
#define EDUROAM_MANAGER_H

#include <WiFi.h>
#include "esp_wpa2.h"

class EduroamManager {
  private:
    const char* _ssid;
    const char* _identity;
    const char* _password;

  public:
    EduroamManager(const char* ssid, const char* identity, const char* password) {
      _ssid = ssid;
      _identity = identity;
      _password = password;
    }

    bool connect(unsigned long timeoutMs = 20000) {
      Serial.print("Connexion Eduroam");

      WiFi.mode(WIFI_STA);
      WiFi.begin(_ssid, "");

      esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)_identity, strlen(_identity));
      esp_wifi_sta_wpa2_ent_set_username((uint8_t*)_identity, strlen(_identity));
      esp_wifi_sta_wpa2_ent_set_password((uint8_t*)_password, strlen(_password));
      esp_wifi_sta_wpa2_ent_enable();

      unsigned long start = millis();

      while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        delay(500);
        Serial.print(".");
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi OK");
        return true;
      }

      Serial.println("\nÉchec connexion WiFi");
      return false;
    }

    void disconnect() {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      Serial.println("WiFi coupé (Mode autonome actif)");
    }

    bool isConnected() {
      return WiFi.status() == WL_CONNECTED;
    }
};

#endif