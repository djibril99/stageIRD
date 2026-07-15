#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H


#include "time.h"

class TimeManager {
  private:
    const char* _tzInfo;

  public:
    TimeManager(const char* tzInfo) {
      _tzInfo = tzInfo;
    }

    bool syncTime() {
      if (WiFi.status() != WL_CONNECTED) return false;

      configTzTime(_tzInfo, "fr.pool.ntp.org", "pool.ntp.org");

      struct tm timeinfo;
      int tentative = 0;

      while (!getLocalTime(&timeinfo) && tentative < 20) {
        delay(500);
        Serial.print(".");
        tentative++;
      }

      if (tentative >= 20) {
        Serial.println("\nÉchec de synchronisation NTP");
        return false;
      }

      Serial.println("\nHorloge synchronisée");
      return true;
    }

    String getFormattedTime() {
      struct tm timeinfo;
      char dateStr[25];

      if (getLocalTime(&timeinfo)) {
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(dateStr);
      }

      return String("NO_TIME");
    }
};

#endif