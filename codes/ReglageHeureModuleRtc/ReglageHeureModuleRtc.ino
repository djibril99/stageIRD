#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("RTC non detecte !");
    while (1);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("Heure du RTC reglee !");
}

void loop() {
}