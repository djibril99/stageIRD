const int pinMeasure = A0;
const int pinVCC = 8;
const int pinGND = 7;

const float Rref = 100000.0;

const int NB_LECTURES = 20;

void setup() {
  Serial.begin(9600);

  pinMode(pinVCC, OUTPUT);
  pinMode(pinGND, OUTPUT);

  digitalWrite(pinVCC, HIGH);
  digitalWrite(pinGND, LOW);

  delay(200);
}

float measureResistance(bool reverse) {

  // activation polarité
  if (reverse) {
    digitalWrite(pinVCC, LOW);
    digitalWrite(pinGND, HIGH);
  } else {
    digitalWrite(pinVCC, HIGH);
    digitalWrite(pinGND, LOW);
  }

  delay(10); // stabilisation

  int adc = analogRead(pinMeasure);

  if (adc <= 1) adc = 1;
  if (adc >= 1022) adc = 1022;

  float V = adc * 5.0 / 1023.0;

  float R;

  if (!reverse) {
    // sens normal
    R = Rref * V / (5.0 - V);
  } else {
    // sens inversé
    R = Rref * (5.0 - V) / V;
  }

  return R;
}

void loop() {

  float somme = 0;

  Serial.println("---- MESURE ----");

  for (int i = 0; i < NB_LECTURES; i++) {

    bool reverse = (i % 2 == 1);

    float R = measureResistance(reverse);

    Serial.print("Lecture ");
    Serial.print(i);
    Serial.print(" | R = ");
    Serial.println(R);

    somme += R;

    delay(50);
  }

  float moyenne = somme / NB_LECTURES;

  Serial.print("=== MOYENNE FINALE = ");
  Serial.print(moyenne);
  Serial.println(" ohms ===");

  Serial.println();

  delay(2000);
}