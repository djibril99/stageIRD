#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  rtc.begin();
}

void loop() {

  DateTime now = rtc.now();

  Serial.print(now.day());
  Serial.print("/");

  Serial.print(now.month());
  Serial.print("/");

  Serial.print(now.year());

  Serial.print("  ");

  Serial.print(now.hour());
  Serial.print(":");

  Serial.print(now.minute());
  Serial.print(":");

  Serial.println(now.second());

  delay(1000);
}