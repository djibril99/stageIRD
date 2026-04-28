#include <ArduinoJson.h>
#define TAILLE_FENETRE 100
#define VCC 5

// =======================
// CONFIG
// =======================

//const int TAILLE_FENETRE = 100;
//const float VCC = 5.0;
const float alpha = 0.0512; // ce parametre permet de faire du lissage ( plus elle est basse plus la courbe est lisse , mais elle ralentie la progression de la courbe )


// =======================
// CLASSE CAPTEUR
// =======================

class Capteur {
  private:
    int pinRead;
    float samples[100];   // fenetre coulissant pour le filtre médiant 
    float filtered = 0;  // la donnée filtrée 
    bool init = false; 

  public:
    Capteur(int pin) {
      pinRead = pin;
    }

    float readRaw() {

      for (int i = 0; i < TAILLE_FENETRE; i++) {

        int adc = analogRead(pinRead);
        float voltage = (adc * VCC) / 1023.0;

        if (voltage <= 0.01 || voltage >= VCC - 0.01) {
          samples[i] = 0;
        } else {
          samples[i] = 1000000.0 * ((VCC / voltage) - 1.0);
        }

        delay(2);
      }

      // médiane
      for (int i = 0; i < TAILLE_FENETRE - 1; i++) {
        for (int j = i + 1; j < TAILLE_FENETRE; j++) {
          if (samples[j] < samples[i]) {
            float t = samples[i];
            samples[i] = samples[j];
            samples[j] = t;
          }
        }
      }

      return samples[TAILLE_FENETRE / 2];
    }

    float readFiltered() {

      float raw = readRaw();

      if (!init) {
        filtered = raw;
        init = true;
      } else {
        filtered = alpha * raw + (1 - alpha) * filtered;
      }
      return filtered;
    }
};

// =======================
// CAPTEURS
// =======================

Capteur capA6(A6);
Capteur capA0(A0);
Capteur capA1(A1);
Capteur capA2(A2);
// =======================
// SETUP
// =======================

void setup() {
  Serial.begin(115200);
}

// =======================
// LOOP
// =======================

void loop() {

  float rawA6 = capA6.readRaw();
  float filtA6 = capA6.readFiltered();

  float rawA0 = capA0.readRaw();
  float filtA0 = capA0.readFiltered();

  // JSON
  StaticJsonDocument<200> doc;

  JsonObject a6 = doc["a6"].to<JsonObject>();
  a6["raw"] = rawA6;
  a6["f"] = filtA6;

  JsonObject a0 = doc["a0"].to<JsonObject>();
  a0["raw"] = rawA0;
  a0["f"] = filtA0;

  serializeJson(doc, Serial);
  Serial.println();
  delay(50);
}

void resister(JsonObject obj , String name , float value ){
}