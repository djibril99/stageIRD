void setup() {
  Serial.begin(115200);   // debug PC
  Serial1.begin(9600);    // vers ESP32 (pins 18/19)
}

void loop() {

  String json = "{\"a0\":123,\"a1\":456}";

  Serial1.println(json);

  Serial.println("Envoye: " + json);

  delay(1000);
}