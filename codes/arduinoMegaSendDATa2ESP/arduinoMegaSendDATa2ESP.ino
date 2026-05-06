#include <ArduinoJson.h>

#define TAILLE_FENETRE 50
#define VCC 5
const float alpha = 0.0512;

// =======================
// CLASSE CAPTEUR
// =======================
class Capteur {
  private:
    int pinRead;
    int pinVCC;
    int pinGND;

    float samples[TAILLE_FENETRE];
    float filtered = 0;
    bool init = false;

  public:
    Capteur(int readPin, int vccPin, int gndPin) {
      pinRead = readPin;
      pinVCC  = vccPin;
      pinGND  = gndPin;
    }

    void begin() {
      pinMode(pinVCC, OUTPUT);
      pinMode(pinGND, OUTPUT);

      digitalWrite(pinVCC, LOW);
      digitalWrite(pinGND, LOW);
    }

    float readRaw() {

      // alimentation capteur
      digitalWrite(pinVCC, HIGH);
      digitalWrite(pinGND, LOW);

      delay(10); // stabilisation capteur

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

      // couper alimentation après lecture
      digitalWrite(pinVCC, LOW);
      digitalWrite(pinGND, LOW);

      // tri
      for (int i = 0; i < TAILLE_FENETRE - 1; i++) {
        for (int j = i + 1; j < TAILLE_FENETRE; j++) {
          if (samples[j] < samples[i]) {
            float t = samples[i];
            samples[i] = samples[j];
            samples[j] = t;
          }
        }
      }

      int milieu = TAILLE_FENETRE / 2;
/////////////////////////////////

//faire la moyenne de sur un tableau de taille TAILLEMoyyennARRAYE au tour du int milieu = TAILLE_FENETRE / 2; int dim = 5; float moy = 0; int count = 0; for (int i = milieu - dim; i <= milieu + dim; i++) { if (i >= 0 && i < TAILLE_FENETRE) { moy += samples[i]; count++; } } if (count == 0) return samples[milieu]; moy = moy / count;

//////////////////////////////////////////



      return samples[milieu];
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
// CAPTEURS
// =======================

Capteur capA0(A0 ,8,7);
Capteur capA1(A1 ,10,9 );
Capteur capA2(A2,12,11);
Capteur capA3(A3,14,13);

Capteur capA6(A6 ,28,19);


// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);    // debug PC
  Serial1.begin(9600);     // vers ESP32
}

// =======================
// LOOP
// =======================
void loop() {

  // lecture capteurs
  float rawA6 = capA6.readRaw();
  float filtA6 = capA6.readFiltered();

  float rawA0 = capA0.readRaw();
  float filtA0 = capA0.readFiltered();

  float rawA1 = capA1.readRaw();
  float filtA1 = capA1.readFiltered();

  float rawA2 = capA2.readRaw();
  float filtA2 = capA2.readFiltered();

  float rawA3 = capA3.readRaw();
  float filtA3 = capA3.readFiltered();

  // JSON
  StaticJsonDocument<300> doc;

  JsonObject a6 = doc["a6"].to<JsonObject>();
  a6["raw"] = rawA6;
  a6["f"] = filtA6;

  JsonObject a0 = doc["a0"].to<JsonObject>();
  a0["raw"] = rawA0;
  a0["f"] = filtA0;

  JsonObject a1 = doc["a1"].to<JsonObject>();
  a1["raw"] = rawA1;
  a1["f"] = filtA1;

  JsonObject a2 = doc["a2"].to<JsonObject>();
  a2["raw"] = rawA2;
  a2["f"] = filtA2;

  JsonObject a3 = doc["a3"].to<JsonObject>();
  a3["raw"] = rawA3;
  a3["f"] = filtA3;

  // envoi ESP32
  serializeJson(doc, Serial1);
  Serial1.println();   // fin de trame

  // debug PC
  //serializeJson(a0, Serial);
  //Serial.println();
   plotCapteurs(
   rawA6,  filtA6,
   rawA0,  filtA0,
   rawA1,  filtA1,
   rawA2,  filtA2
  ) ;

  delay(5);
}


void plotCapteurs(
  float rawA6, float filtA6,
  float rawA0, float filtA0,
  float rawA1, float filtA1,
  float rawA2, float filtA2
) {

  Serial.print("A6_raw:");
  Serial.print(rawA6);
  Serial.print(" ");

  Serial.print("A6_f:");
  Serial.print(filtA6);
  Serial.print(" ");

  Serial.print("A0_raw:");
  Serial.print(rawA0);
  Serial.print(" ");

  Serial.print("A0_f:");
  Serial.print(filtA0);
  Serial.print(" ");

  Serial.print("A1_raw:");
  Serial.print(rawA1);
  Serial.print(" ");

  Serial.print("A1_f:");
  Serial.print(filtA1);
  Serial.print(" ");

  Serial.print("A2_raw:");
  Serial.print(rawA2);
  Serial.print(" ");

  Serial.print("A2_f:");
  Serial.print(filtA2);

  Serial.println();
}