#include <SD.h>
#include "sensorData.h"
class CSVLogger {

private:
    ClockManager* clock;
    const char* fileName;

public:

    CSVLogger(ClockManager& cm, const char* filename) {
        clock = &cm;
        fileName = filename;
    }

    bool begin() {

        if (!SD.begin(53)) {
            Serial.println("Erreur SD");
            return false;
        }

        if (!SD.exists(fileName)) {

            File f = SD.open(fileName, FILE_WRITE);

            if (f) {
                f.println(
                    "Date,Heure,"
                    "W1_raw,W1_f,"
                    "W2_raw,W2_f,"
                    "W3_raw,W3_f,"
                    "W4_raw,W4_f,"
                    "W5_raw,W5_f,"
                    "W6_raw,W6_f,"
                    "W7_raw,W7_f,"
                    "W8_raw,W8_f"
                );

                f.close();
            }
        }

        return true;
    }

    void save(SensorData data[8]) {

        File f = SD.open(fileName, FILE_WRITE);

        if (!f) {
            Serial.println("Erreur fichier");
            return;
        }

        DateTime now = clock->now();

        char dateBuf[11];
        sprintf(dateBuf, "%04d-%02d-%02d",
                now.year(), now.month(), now.day());

        char timeBuf[9];
        sprintf(timeBuf, "%02d:%02d:%02d",
                now.hour(), now.minute(), now.second());

        f.print(dateBuf);
        f.print(",");
        f.print(timeBuf);

        for (int i = 0; i < 8; i++) {
            f.print(",");
            f.print(data[i].raw, 2);
            f.print(",");
            f.print(data[i].filt, 2);
        }

        f.println();
        f.close();
    }
};