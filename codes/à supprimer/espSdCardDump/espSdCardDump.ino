#include <WiFi.h>
#include "esp_wpa2.h"
#include "time.h"
#include <SPI.h>
#include <SD.h> 
#include <ArduinoJson.h>

// =====================================================
// WIFI EDUROAM
// =====================================================
const char* ssid_uni = "eduroam";
#define EAP_IDENTITY "bwd2978a@univ-tlse3.fr"
#define EAP_PASSWORD "motDEpasse1999@"
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// =====================================================
// SD
// =====================================================
const int chipSelect = 5;

// =====================================================
// LIAISON MEGA -> ESP32
// RX = GPIO16 | TX = GPIO17
// =====================================================
HardwareSerial MegaSerial(1);

// =====================================================
// GESTION DE LA PAUSE DE 20 MINUTES
// =====================================================
unsigned long derniereSauvegardeMillis = 0; 
// 20 minutes = 20 * 60 * 1000 millisecondes
const unsigned long DUREE_PAUSE =   5 * 60 * 1000; 
bool premiereMesure = true; // Permet d'enregistrer tout de suite au démarrage

// =====================================================

void setup() {
  Serial.begin(115200);

  MegaSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println();
  Serial.println("Demarrage ESP32");

  // INITIALISATION SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Erreur carte SD");
    while (true) {
      delay(1000);
    }
  }
  Serial.println("Carte SD OK");

  // CREATION ENTETE CSV
  if (!SD.exists("/mesuresDECHARGE.csv")) {
    File file = SD.open("/mesuresDECHARGE.csv", FILE_WRITE);
    if (file) {
      file.println("Date_Heure;W1_raw;W1_f;W2_raw;W2_f;W3_raw;W3_f;W4_raw;W4_f");
      file.close();
    }
  }

  // CONNEXION EDUROAM
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_uni, "");
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();

  Serial.print("Connexion Eduroam");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  // SYNCHRO NTP
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK");
    configTzTime(TZ_INFO, "fr.pool.ntp.org", "pool.ntp.org");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nHorloge synchronisee");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi coupe");
  } else {
    Serial.println("\nImpossible de synchroniser l'heure");
  }

  Serial.println("Pret pour acquisition (Intervalle: 20mn)");
}

// =====================================================

void loop() {
  // 1. S'il n'y a rien sur le port série, on ne fait rien
  if (!MegaSerial.available()) {
    return;
  }

  // 2. On lit IMPÉRATIVEMENT la ligne pour vider le buffer de l'ESP32
  String ligne = MegaSerial.readStringUntil('\n');
  ligne.trim();

  if (ligne.length() == 0) {
    return;
  }

  // 3. VÉRIFICATION DE LA PAUSE DES 20 MINUTES
  // Si on n'est pas au premier démarrage ET que les 20 minutes ne sont pas écoulées :
  if (!premiereMesure && (millis() - derniereSauvegardeMillis < DUREE_PAUSE)) {
    // On ignore la donnée reçue car on est en période de pause
    return;
  }

  Serial.print("Recu et accepte : ");
  Serial.println(ligne);

  // DECODAGE JSON
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, ligne);

  if (err) {
    Serial.print("Erreur JSON : ");
    Serial.println(err.c_str());
    return; // Si le JSON est corrompu, on quitte sans mettre à jour le chrono
  }

  // DATE HEURE
  char dateStr[25];
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    strcpy(dateStr, "NO_TIME");
  }

  // ECRITURE CSV
  File file = SD.open("/mesuresDECHARGE.csv", FILE_APPEND);
  if (!file) {
    Serial.println("Erreur ouverture fichier SD");
    return;
  }

  file.print(dateStr);
  file.print(";");
  file.print(doc["W1"]["raw"].as<float>());
  file.print(";");
  file.print(doc["W1"]["f"].as<float>());
  file.print(";");
  file.print(doc["W2"]["raw"].as<float>());
  file.print(";");
  file.print(doc["W2"]["f"].as<float>());
  file.print(";");
  file.print(doc["W3"]["raw"].as<float>());
  file.print(";");
  file.print(doc["W3"]["f"].as<float>());
  file.print(";");
  file.print(doc["W4"]["raw"].as<float>());
  file.print(";");
  file.println(doc["W4"]["f"].as<float>());
  file.close();

  Serial.print("Sauvegarde reussie : ");
  Serial.println(dateStr);

  // ===================================================
  // ENCLENCHEMENT DE LA PAUSE DE 20 MINUTES
  // ===================================================
  derniereSauvegardeMillis = millis(); // On enregistre le moment précis du succès
  premiereMesure = false;             // La toute première mesure est passée
  Serial.println("--> Debut de la pause de 20 minutes...");
}