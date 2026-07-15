#include <Wire.h>
#include <RTClib.h>


class ClockManager {

private:
    RTC_DS3231 rtc;

public:

    bool begin() {

        Wire.begin();

        if (!rtc.begin()) {
            Serial.println("RTC non detecte");
            return false;
        }

        if (rtc.lostPower()) {
            Serial.println("RTC reset -> reinitialisation");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        return true;
    }

    DateTime now() {
        return rtc.now();
    }

    String date() {
        DateTime t = rtc.now();

        char b[11];
        sprintf(b, "%04d-%02d-%02d", t.year(), t.month(), t.day());
        return String(b);
    }

    String time() {
        DateTime t = rtc.now();

        char b[9];
        sprintf(b, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
        return String(b);
    }
};