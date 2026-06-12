#include <ArduinoJson.h>

#define TAILLE_FENETRE 50//220
#define TAILLE_FENETRE_MOYENNE 5
#define VCC 5
const float alpha = 0.015;//0.0512;

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
    float moyenneMediane ; 
    bool init = false;

  public:
    Capteur(int readPin, int gndPin, int vccPin) {
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

    bool reverse = false;
    const float resistanceInterne = 100000.0;  //99600.0f; // 1 MΩ

    for (int i = 0; i < TAILLE_FENETRE; i++) {

        if (reverse) {
            digitalWrite(pinVCC, LOW);
            digitalWrite(pinGND, HIGH);
        } else {
            digitalWrite(pinVCC, HIGH);
            digitalWrite(pinGND, LOW);
        }

        delay(10);

        int adc = analogRead(pinRead);

        // éviter division par zéro
        if (adc <= 0) adc = 1;
        if (adc >= 1023) adc = 1022;

        float voltage = (adc * VCC) / 1023.0f;

        if (!reverse) {
            // Résistance connue côté VCC
            samples[i] = resistanceInterne * voltage / (VCC - voltage);
        } else {
            // Polarité inversée
            samples[i] = resistanceInterne * ((VCC / voltage) - 1.0f);
        }

        reverse = !reverse;

        delay(9);
    }

    // couper complètement l'alimentation du capteur
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
      //faire une moyenne sur au tour de la mediane pour lisser les valeurs
      int TailleMoyenne = TAILLE_FENETRE_MOYENNE; // nombre de valeurs à prendre de chaque côté de la médiane
      float somme = 0;
      int count = 0;
      for (int i = milieu - TailleMoyenne; i <= milieu + TailleMoyenne; i++) {
        if (i >= 0 && i < TAILLE_FENETRE) {
          somme += samples[i];
          count++;
        }
      }

      if (count == 0){
        moyenneMediane = samples[milieu] ;
      } 
      else{
        moyenneMediane =  somme / count;
      }

      return samples[milieu];

//////////////////////////////////////////
    //return samples[milieu];
    }

    float __readFiltered__() { //deprecier , trop de calculs pour une simple lecture
      float raw = readRaw();

      if (!init) {
        filtered = raw;
        init = true;
      } else {
        filtered = alpha * raw + (1 - alpha) * filtered;
      }
      return filtered;
    }

    float updateFiltered() {
      float raw =  moyenneMediane ;
      if (!init) {
        filtered = raw;
        init = true;
      } else {
        filtered = alpha * raw + (1 - alpha) * filtered;
      }
      return filtered;
    }
};

struct SensorData {
  float raw;
  float filt;
};



// CAPTEURS 
// =======================

Capteur capA0(A0 ,8,7);
Capteur capA1(A1 ,10,9 );
Capteur capA2(A2,12,11);
Capteur capA3(A3,14,13);

Capteur capA4(A4,16,15);
Capteur capA5(A5,18,17);

Capteur capA6(A6,28,19);

Capteur capA7(A7,26,24);


// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);    // debug PC
  //Serial1.begin(9600);     // vers ESP32
}

// =======================
// LOOP

void loop() {

  SensorData data[4];
  Serial.print("lecture ......");
  data[0].raw = capA0.readRaw();
  data[0].filt = capA0.updateFiltered();
  
  data[1].raw = capA1.readRaw();
  data[1].filt = capA1.updateFiltered();
  
  data[2].raw = capA2.readRaw();
  data[2].filt = capA2.updateFiltered();

  data[3].raw = capA3.readRaw();
  data[3].filt = capA3.updateFiltered();
  Serial.print("   Moitié ......");
  data[4].raw = capA4.readRaw();
  data[4].filt = capA4.updateFiltered();

  data[5].raw = capA5.readRaw();
  data[5].filt = capA5.updateFiltered();
  
  data[6].raw = capA6.readRaw();
  data[6].filt = capA6.updateFiltered();
  
  data[7].raw = capA7.readRaw();
  data[7].filt = capA7.updateFiltered();

  Serial.print("FIN !!!");

  const char* names[8] = { "W1", "W2", "W3", "W4" ,"W5", "W6", "W7", "W8" };

  StaticJsonDocument<1000> doc;

  for (int i = 0; i < 8; i++) {
    JsonObject obj = doc[names[i]].to<JsonObject>();
    obj["raw"] = data[i].raw;
    obj["f"]   = data[i].filt;
    //ecriture forma json
    Serial.print("{");
    Serial.print(names[i]);
    Serial.print(": {raw: ");
    Serial.print(data[i].raw);
    Serial.print(", f: ");
    Serial.print(data[i].filt);
    Serial.println("}}");
  }

  //serializeJson(doc, Serial);
  Serial.println();

  //plotCapteurs(data, names, 8);

  delay(5);
}

void plotCapteurs(SensorData data[], const char* names[], int count) {

  for (int i = 0; i < count; i++) {
    Serial.print(names[i]);
    Serial.print("_raw:");
    Serial.print(data[i].raw);
    Serial.print(" ");

    Serial.print(names[i]);
    Serial.print("_f:");
    Serial.print(data[i].filt);
    Serial.print(" ");
  }

  Serial.println();
}