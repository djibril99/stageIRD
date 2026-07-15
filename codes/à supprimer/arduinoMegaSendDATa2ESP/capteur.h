#ifndef CAPTEUR_H
#define CAPTEUR_H

class Capteur {

  private:
    int pinMeasure;
    int pinVCC;
    int pinGND;

    const float Rref = 99600.0;
    const int NB_LECTURES = 100;

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

#endif