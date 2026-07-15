#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_wpa2.h"
#include <ArduinoJson.h>

// =========================
// Delai
// =========================
int compteur  = 0 ;
int compteurMax = 2 ; 
// =========================
// UART
// =========================
#define RXD2 15
#define TXD2 2

// =========================
// LED
// =========================
#define LED_BUILTIN 2

// =========================
// WIFI EDUROAM
// =========================
const char* ssid_uni = "eduroam";
#define EAP_IDENTITY "bwd2978a@univ-tlse3.fr"
#define EAP_PASSWORD "motDEpasse1999@"

// =========================
// WIFI PERSO
// =========================
const char* ssid_perso = "iPhone de Djibril";
const char* pass_perso = "passer123";

// =========================
// SERVER
// =========================
const char* serverURL = "https://stageird.onrender.com/api/data";

// =========================
// DATA STATE
// =========================
bool dataPending = false;
String lastJson = "";


float delaiMN = 2 ;
// =========================
// LED FUNCTION
// =========================
void blinkLed(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMs);
  }
}

// =========================
// WIFI CONNECTION
// =========================
void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("\nConnexion WiFi...");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  // ===== EDUROAM =====
  Serial.println("Essai Eduroam...");
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();

  WiFi.begin(ssid_uni);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  // ===== FALLBACK IPHONE =====
  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("\nEchec Eduroam → iPhone");

    esp_wifi_sta_wpa2_ent_disable();
    WiFi.disconnect();

    WiFi.begin(ssid_perso, pass_perso);

    start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
      delay(500);
      Serial.print(".");
    }
  }

  // ===== RESULT =====
  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("\nWiFi OK");
    Serial.println(WiFi.localIP());

    blinkLed(3, 200);
    digitalWrite(LED_BUILTIN, HIGH);

  } else {

    Serial.println("\nWiFi FAIL");

    blinkLed(10, 100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

// =========================
// SETUP
// =========================
void setup() {

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  connectWiFi();
}

// =========================
// LOOP
// =========================
void loop() {

  // ===== WIFI CHECK =====
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // ===== UART READ =====
  if (!dataPending && Serial2.available()) {

    lastJson = Serial2.readStringUntil('\n');
    lastJson.trim();

    if (lastJson.length() > 0) { //nouvelle donnée  
      // Allocate a JSON document in memory
      StaticJsonDocument<1500> doc;
      DeserializationError error = deserializeJson(doc, lastJson);

      // Check for parsing errors
      if (error) {
      Serial.print(F("Deserialization failed: "));
      Serial.println(error.f_str());
      dataPending = false;
      }
      else{
        dataPending = true;
      }
    }
    

  }

  // ===== SEND DATA =====
  if (dataPending && WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(lastJson);

    Serial.printf("HTTP: %d\n", code);

    http.end();

    // ===== SUCCESS =====
    if (code == 200) {

      Serial.println("Envoi OK");

      dataPending = false;
      lastJson = "";

      blinkLed(1, 300);
      //delay(delaiMN * 60000); 
      unsigned long start = millis();
      unsigned long duree = delaiMN * 60000;
      unsigned long now = millis();
      while (now - start < duree) {
        Serial.println(duree - now - start ) ; 
        
        now = millis();
      }

    } else {
      compteur ++ ; 
      Serial.println("Erreur → retry dans 30s");
      delay(30000);
      if(compteur >= compteurMax ){
        compteur  = 0  ;
        dataPending = false;
        lastJson = "";
        Serial.println("Max de tentative atteintes") ; 
      }

    }
  }

  delay(10);
}