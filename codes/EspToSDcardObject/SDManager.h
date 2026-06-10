#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SD.h>


class SDManager {
  private:
    int _csPin;
    const char* _filePath;

  public:
    SDManager(int csPin, const char* filePath)
      : _csPin(csPin), _filePath(filePath) {}

    bool begin() {
      if (!SD.begin(_csPin)) {
        Serial.println("Erreur carte SD");
        return false;
      }
      Serial.println("Carte SD OK");
      return true;
    }

    void createHeader() {
      if (!SD.exists(_filePath)) {
        File file = SD.open(_filePath, FILE_WRITE);
        if (file) {
          file.println("Date_Heure;W1_raw;W1_f;W2_raw;W2_f;W3_raw;W3_f;W4_raw;W4_f");
          file.close();
        }
      }
    }

    bool appendRow(const String& rowData) {
      File file = SD.open(_filePath, FILE_APPEND);
      if (!file) {
        Serial.println("Erreur ouverture fichier SD");
        return false;
      }

      file.println(rowData);
      file.close();
      return true;
    }
};

#endif