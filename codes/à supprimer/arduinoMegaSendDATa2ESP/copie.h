#include <ArduinoJson.h>

#include <SoftwareSerial.h>




#include "ClockManager.h"
#include "CSVLogger.h"

#include "sensorData.h"


#define TAILLE_FENETRE 50//220
#define TAILLE_FENETRE_MOYENNE 5
#define VCC 5



SoftwareSerial Logger(46, 47); // RX, TX

// =======================
// CLASSE CAPTEUR
// =======================

class Capteur {

  private:
    int pinMeasure;
    int pinVCC;
    int pinGND;

    const float Rref = 100000.0;
    const int NB_LECTURES = 20;

    float rawValue = 0;
    float filtered = 0;
    bool init = false;

    float alpha = 0.2;

  public:

    Capteur(int measurePin, int gndPin, int vccPin) {
      pinMeasure = measurePin;
      pinVCC = vccPin;
      pinGND = gndPin;
    }

    void begin() {
      pinMode(pinVCC, OUTPUT);
      pinMode(pinGND, OUTPUT);

      digitalWrite(pinVCC, HIGH);
      digitalWrite(pinGND, LOW);

      delay(200);
    }

    // =====================================================
    // MESURE EXACTE IDENTIQUE À TON CODE
    // =====================================================
    float measureResistance(bool reverse) {

      if (reverse) {
        digitalWrite(pinVCC, LOW);
        digitalWrite(pinGND, HIGH);
      } else {
        digitalWrite(pinVCC, HIGH);
        digitalWrite(pinGND, LOW);
      }

      delay(10);

      int adc = analogRead(pinMeasure);

      if (adc <= 1) adc = 1;
      if (adc >= 1022) adc = 1022;

      float V = adc * 5.0 / 1023.0;

      float R;

      if (!reverse) {
        R = Rref * V / (5.0 - V);
      } else {
        R = Rref * (5.0 - V) / V;
      }

      return R;
    }

    // =====================================================
    // UPDATE (identique loop de ton test)
    // =====================================================
    void update() {

      float somme = 0;

      for (int i = 0; i < NB_LECTURES; i++) {

        bool reverse = (i % 2 == 1);

        float R = measureResistance(reverse);

        somme += R;

        delay(50);
      }

      rawValue = somme / NB_LECTURES;

      // filtre exponentiel
      //verifir s il n y a pas une tres grande difference entre rawValue et filtered, si oui, ne pas appliquer le filtre pour eviter les grosses erreurs de debut


      if (!init or abs(rawValue - filtered) > 2000) {
        filtered = rawValue;
        init = true;
      } else {
        filtered = alpha * rawValue + (1 - alpha) * filtered;
      }
    }

    // =====================================================
    // GETTERS
    // =====================================================
    float getRawValue() {
      return rawValue;
    }

    float getFiltered() {
      return filtered;
    }
};





// CAPTEURS 
// =======================
Capteur capA0(A0, 7, 8);
Capteur capA1(A1, 9, 10);
Capteur capA2(A2, 11, 12);
Capteur capA3(A3, 13, 14);
Capteur capA4(A4, 15, 16);
Capteur capA5(A5, 17, 18);
Capteur capA6(A6, 19, 28);
Capteur capA7(A7, 24, 26);
//CLOCK
//ClockManager clockAdmin;
//CSVLogger logger(clockAdmin, "dataLOGGG.csv");



// =======================
// SETUP
// =======================
void setup1() {
  Serial.begin(115200);    // debug PC
  //Serial1.begin(9600);     // vers ESP32
  //clockAdmin.begin();
  //logger.begin();
  Logger.begin(9600);


  capA0.begin();
  capA1.begin();
  capA2.begin();
  capA3.begin();
  capA4.begin();
  capA5.begin();
  capA6.begin();
  capA7.begin();




}

// =======================
// LOOP

void loop() {

  SensorData data[4];
  Serial.print("lecture ......");
  //update capteurs
  capA0.update();
  capA1.update();
  capA2.update();
  capA3.update();
  capA4.update();
  capA5.update();
  capA6.update();
  capA7.update(); 


  data[0].raw = capA0.getRawValue();
  data[0].filt = capA0.getFiltered();
  
  data[1].raw = capA1.getRawValue();
  data[1].filt = capA1.getFiltered();
  
  data[2].raw = capA2.getRawValue();
  data[2].filt = capA2.getFiltered();

  data[3].raw = capA3.getRawValue();
  data[3].filt = capA3.getFiltered();

  data[4].raw = capA4.getRawValue();
  data[4].filt = capA4.getFiltered();

  data[5].raw = capA5.getRawValue();
  data[5].filt = capA5.getFiltered();
  
  data[6].raw = capA6.getRawValue();
  data[6].filt = capA6.getFiltered();
  
  data[7].raw = capA7.getRawValue();
  data[7].filt = capA7.getFiltered();

  Serial.println("FIN !!!");

  const char* names[8] = { "W1", "W2", "W3", "W4" ,"W5", "W6", "W7", "W8" };

  StaticJsonDocument<1000> doc;

  for (int i = 0; i < 8; i++) {
    JsonObject obj = doc[names[i]].to<JsonObject>();
    obj["raw"] = data[i].raw;
    obj["f"]   = data[i].filt;
    //ecriture forma json
   
  }

  serializeJson(doc, Serial);
  Serial.println();

  serializeJson(doc, Logger);
  Logger.println();

  //plotCapteurs(data, names, 8);

  //logger.save(data);

  delay(1000);
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