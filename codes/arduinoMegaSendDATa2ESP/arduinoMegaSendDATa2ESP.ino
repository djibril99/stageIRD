#include <ArduinoJson.h>

#define TAILLE_FENETRE 200
#define VCC 5
const float alpha = 0.02;//0.0512;

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

      bool reverse = false;
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
        float voltage = (adc * VCC) / 1023.0;
        
        if(!reverse){
          samples[i] = 1000000.0 * ((VCC / voltage) - 1.0);
        } else {
          samples[i] = 1000000.0 * voltage / (VCC - voltage);
        }
        delay(9);
        reverse = !reverse;
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
      //faire une moyenne sur au tour de la mediane pour lisser les valeurs
      int TailleMoyenne = 15; // nombre de valeurs à prendre de chaque côté de la médiane
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

void loop() {

  SensorData data[4];
  
  data[0].raw = capA0.readRaw();
  data[0].filt = capA0.updateFiltered();
  
  data[1].raw = capA1.readRaw();
  data[1].filt = capA1.updateFiltered();
  
  data[2].raw = capA2.readRaw();
  data[2].filt = capA2.updateFiltered();

  data[3].raw = capA3.readRaw();
  data[3].filt = capA3.updateFiltered();

  const char* names[4] = { "W1", "W2", "W3", "W4"};

  StaticJsonDocument<300> doc;

  for (int i = 0; i < 4; i++) {
    JsonObject obj = doc[names[i]].to<JsonObject>();
    obj["raw"] = data[i].raw;
    obj["f"]   = data[i].filt;
  }

  serializeJson(doc, Serial1);
  Serial1.println();

  plotCapteurs(data, names, 4);

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