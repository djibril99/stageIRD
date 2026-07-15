const int pinHigh = 28;
const int pinLow  = 19;
const int pinRead = A6;

// Utilisation d'une constante globale pour la maintenance
const int TAILLE_FENETRE = 100; 
const float VCC = 5.0;

// ===== FILTRE EMA =====
const float alpha = 0.12;
float filtered = 0.0;
bool filterInit = false;

float samples[TAILLE_FENETRE];

// ===== MEDIANE GENERIQUE =====
float calculerMediane(float *x) {
  float a[TAILLE_FENETRE];

  // Copie des données
  for (int i = 0; i < TAILLE_FENETRE; i++) a[i] = x[i];

  // Tri à bulles (Bubble Sort)
  for (int i = 0; i < TAILLE_FENETRE - 1; i++) {
    for (int j = i + 1; j < TAILLE_FENETRE; j++) {
      if (a[j] < a[i]) {
        float t = a[i];
        a[i] = a[j];
        a[j] = t;
      }
    }
  }

  // Retourne la valeur centrale
  return a[TAILLE_FENETRE / 2];
}

void setup() {
  Serial.begin(115200);
  pinMode(pinHigh, OUTPUT);
  pinMode(pinLow, OUTPUT);
  Serial.println("Raw,Filtered");
}

void loop() {
  digitalWrite(pinHigh, HIGH);
  digitalWrite(pinLow, LOW);
  delay(10);

  // ===== Capture des mesures =====
  for (int i = 0; i < TAILLE_FENETRE; i++) {
    int adc = analogRead(pinRead);
    float voltage = (adc * VCC) / 1023.0;

    if (voltage <= 0.01 || voltage >= VCC - 0.01) {
      samples[i] = 0;
    } else {
      samples[i] = 1000000.0 * ((VCC / voltage) - 1.0);
    }
    delay(5);
  }

  // Calcul de la médiane dynamique
  float rawValue = calculerMediane(samples);

  // ===== Filtrage EMA =====
  if (!filterInit) {
    filtered = rawValue;
    filterInit = true;
  } else {
    filtered = alpha * rawValue + (1.0 - alpha) * filtered;
  }

  // ===== Affichage =====
  Serial.print(rawValue, 2);
  Serial.print(",");
  Serial.println(filtered, 2);

  delay(200);
}