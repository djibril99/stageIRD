#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("ESP32 UART2 pret");
}

void loop() {

  if (Serial2.available()) {

    String json = Serial2.readStringUntil('\n');

    json.trim();

    Serial.println("JSON recu:");
    Serial.println(json);
  }
}