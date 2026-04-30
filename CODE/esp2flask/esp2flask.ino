#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_wpa2.h" // Indispensable pour eduroam

#define RXD2 16
#define TXD2 17

// --- Config Eduroam (Entreprise) ---
const char* ssid_uni = "eduroam";
#define EAP_IDENTITY "bwd2978a@univ-tlse3.fr" 
#define EAP_PASSWORD "motDEpasse1999@"

// --- Config iPhone (Personnel) ---
const char* ssid_perso = "iPhone de Djibril";
const char* pass_perso = "passer123";

const char* serverURL = "https://stageird.onrender.com/api/data";

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("\nTentative de connexion WiFi...");

  // --- TENTATIVE 1 : EDUROAM ---
  Serial.print("Essai sur : "); Serial.println(ssid_uni);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  
  // Configuration spécifique Entreprise
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  
  WiFi.begin(ssid_uni);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start < 10000)) { // Timeout 10s pour eduroam (plus lent)
    delay(500);
    Serial.print(".");
  }

  // --- TENTATIVE 2 : IPHONE (si la 1 a échoué) ---
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nEchec Eduroam. Essai sur : iPhone...");
    esp_wifi_sta_wpa2_ent_disable(); // Désactive le mode entreprise
    WiFi.disconnect();
    
    WiFi.begin(ssid_perso, pass_perso);
    
    start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start < 8000)) { // Timeout 8s
      delay(500);
      Serial.print(".");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connecte !");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nAucun réseau disponible.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  connectWiFi();
}

void loop() {
  // Vérification connexion toutes les 30 secondes si perdu
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (Serial2.available()) {
    String json = Serial2.readStringUntil('\n');
    json.trim();

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverURL);
      http.addHeader("Content-Type", "application/json");
      int code = http.POST(json);
      Serial.printf("Envoi -> HTTP: %d\n", code);
      http.end();
      if (code == 400){
        Serial.println("Donnée envoyé");
      }
    } else {
      Serial.println("Erreur: Données reçues mais WiFi non connecté.");
    }
  }
  delay(10);
}