#include <ArduinoJson.h>

#define TAILLE_FENETRE 300
#define VCC 5.0
const float alpha = 0.025;

int delaiMn = 0.25 * 60000 ; 

// =====================================
// CLASSE CAPTEUR OPTIMISÉE
// =====================================
class Capteur {
  private:
    int pinRead, pinVCC, pinGND;
    float samples[TAILLE_FENETRE];
    float lastRaw = 0;
    float filtered = 0;
    bool init = false;

  public:
    Capteur(int readPin, int vccPin, int gndPin) 
      : pinRead(readPin), pinVCC(vccPin), pinGND(gndPin) {}

    void begin() {
      pinMode(pinVCC, OUTPUT);
      pinMode(pinGND, OUTPUT);
      digitalWrite(pinVCC, LOW);
      digitalWrite(pinGND, LOW);
    }

    // Effectue une lecture complète (Médiane + Moyenne + Filtrage)
    void update() {
      digitalWrite(pinVCC, HIGH);
      delay(10); 
      // 1. Acquisition
      for (int i = 0; i < TAILLE_FENETRE; i++) {
        int adc ; 
        do{
             adc = analogRead(pinRead);
        }while(adc<0 || adc>1023);
        
        float voltage = (adc * VCC) / 1023.0;
        //samples[i] = 1000000.0 * ((VCC / voltage) - 1.0);
        samples[i] = adc ;
        delay(50); 
      }
      digitalWrite(pinVCC, LOW);

      // 2. Tri à bulles pour la médiane pour isoler les grosses variation vers les bornes
      for (int i = 0; i < TAILLE_FENETRE - 1; i++) {
        for (int j = i + 1; j < TAILLE_FENETRE; j++) {
          if (samples[j] < samples[i]) {
            float t = samples[i];
            samples[i] = samples[j];
            samples[j] = t;
          }
        }
      }

      // 3. Moyenne robuste autour de la médiane (dim=5 -> 11 valeurs)
      int milieu = TAILLE_FENETRE / 2;
      float somme = 0;
      int count = 0;
      int tailleMoyenne = int(TAILLE_FENETRE/8) ;
      for (int i = milieu - tailleMoyenne ; i <= milieu + tailleMoyenne ; i++) {
        if (i >= 0 && i < TAILLE_FENETRE) {
          somme += samples[i];
          count++;
        }
      }
      lastRaw = (count > 0) ? (somme / count) : samples[milieu];

      if 

      // 4. Application du Filtre Passe-Bas (EMA)
      if (!init) {
        filtered = lastRaw;
        init = true;
      } else {
        filtered = alpha * lastRaw + (1.0 - alpha) * filtered;
      }
    }

    float getRaw() { return lastRaw; }
    float getFiltered() { return filtered; }
};

// --- Déclaration des capteurs ---
Capteur capA0(A0, 8, 7);
Capteur capA1(A1, 10, 9);
Capteur capA2(A2, 12, 11);
Capteur capA3(A3, 14, 13);
Capteur capA6(A6, 28, 19);

void setup() {
  Serial.begin(115200); // Debug PC
  Serial1.begin(9600);  // Liaison vers ESP32 (Pins 16/17 en général)
  
  capA0.begin();
  capA1.begin();
  capA2.begin();
  capA3.begin();
  capA6.begin();
  Serial.println("System Ready");
}

void loop() {
  // Mise à jour de tous les capteurs
  capA0.update();
  capA1.update();
  capA2.update();
  capA3.update();
  capA6.update();

  // Préparation du JSON
  StaticJsonDocument<1000> doc;
  
  String ids[] = {"a0", "a1", "a2", "a3", "a6"};
  Capteur* caps[] = {&capA0, &capA1, &capA2, &capA3, &capA6};

  for (int i = 0; i < 5; i++) {
    JsonObject obj = doc.createNestedObject(ids[i]);
    obj["raw"] = caps[i]->getRaw();
    obj["f"] = caps[i]->getFiltered();
  }

  // Envoi vers l'ESP32
  serializeJson(doc, Serial1);
  Serial1.println(); 

  // Debug Serial Plotter
  serializeJson(doc, Serial);
  Serial.println(); 
  //delay(delaiMn);// Fréquence d'envoi globale
}