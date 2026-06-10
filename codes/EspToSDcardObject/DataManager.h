#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H



#include <Arduino.h>
#include <ArduinoJson.h>

#include "SDManager.h"
#include "TimeManager.h"
#include "mqtt.h"


class DataManager {
  private:
    unsigned long _derniereSauvegardeMillis;
    unsigned long _dureePause;
    bool _premiereMesure;

    SDManager& _sd;
    TimeManager& _time;
    QameleoMQTT& _mqtt;

  public:
    DataManager(unsigned long pauseMinutes,
                SDManager& sd,
                TimeManager& tm,
                QameleoMQTT& mqtt)
      : _sd(sd), _time(tm), _mqtt(mqtt) {

      _dureePause = pauseMinutes * 60 * 1000;
      _derniereSauvegardeMillis = 0;
      _premiereMesure = true;
    }

    void processIncomingData(const String& rawLine) {

      // pause entre mesures
      if (!_premiereMesure &&
          (millis() - _derniereSauvegardeMillis < _dureePause)) {
        return;
      }

      Serial.print("Reçu et accepté : ");
      Serial.println(rawLine);

      // JSON parse
      StaticJsonDocument<512> doc;
      DeserializationError err = deserializeJson(doc, rawLine);

      if (err) {
        Serial.print("Erreur JSON : ");
        Serial.println(err.c_str());
        return;
      }

      String timestamp = _time.getFormattedTime();

      // CSV pour SD
      String csvRow = timestamp + ";" +
        String(doc["W1"]["raw"].as<float>()) + ";" + String(doc["W1"]["f"].as<float>()) + ";" +
        String(doc["W2"]["raw"].as<float>()) + ";" + String(doc["W2"]["f"].as<float>()) + ";" +
        String(doc["W3"]["raw"].as<float>()) + ";" + String(doc["W3"]["f"].as<float>()) + ";" +
        String(doc["W4"]["raw"].as<float>()) + ";" + String(doc["W4"]["f"].as<float>());

      // SD save
      if (_sd.appendRow(csvRow)) {
        Serial.print("Sauvegarde réussie : ");
        Serial.println(timestamp);

        _derniereSauvegardeMillis = millis();
        _premiereMesure = false;
      }

      // MQTT send
      int numEnvoi = millis() / 1000;

      bool ok = _mqtt.send(
        "waou/V5_3_1",
        numEnvoi,
        doc["W1"]["f"].as<float>(),
        doc["W2"]["f"].as<float>(),
        doc["W3"]["f"].as<float>(),
        doc["W4"]["f"].as<float>()
      );

      if (ok) {
        Serial.println("MQTT OK -> envoyé");
      } else {
        Serial.println("MQTT FAIL");
      }
    }
};

#endif