

#include <SPI.h>

// MODULES
#include "EduroamManager.h"
#include "TimeManager.h"
#include "SDManager.h"
#include "mqtt.h"
#include "DataManager.h"

// Configuration matérielle et réseau globale
HardwareSerial MegaSerial(1);

// ====================================================================
// INSTANCIATION DES OBJETS
// ====================================================================

EduroamManager wifiAdmin(
  "eduroam",
  "bwd2978a@univ-tlse3.fr",
  "motDEpasse1999@"
);

TimeManager timeAdmin(
  "CET-1CEST,M3.5.0,M10.5.0/3"
);

SDManager sdAdmin(
  5,
  "/mesuresDechargeTable.csv"
);

QameleoMQTT mqttAdmin;

// IMPORTANT : DataManager dépend de SD + Time + MQTT
DataManager dataAdmin(
  5,
  sdAdmin,
  timeAdmin,
  mqttAdmin
);

// ====================================================================
// SETUP
// ====================================================================
void setup() {
  Serial.begin(115200);
  MegaSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("\nDémarrage ESP32 ");

  // SD INIT
  if (!sdAdmin.begin()) {
    Serial.println("SD KO -> blocage");
    while (true) delay(1000);
  }
  sdAdmin.createHeader();

  // WIFI EDUROAM
  if (wifiAdmin.connect()) {

    // TIME SYNC
    timeAdmin.syncTime();

    // MQTT INIT
    mqttAdmin.begin();

    if (mqttAdmin.connect()) {
      Serial.println("MQTT OK");
    } else {
      Serial.println("MQTT FAIL");
    }

  } else {
    Serial.println("WiFi FAIL");
  }

  Serial.println("Prêt pour acquisition autonome.");
}

// ====================================================================
// LOOP
// ====================================================================
void loop() {

  // MQTT loop obligatoire (sinon déconnexion)
  mqttAdmin.loop();

  // Lecture Mega
  if (MegaSerial.available()) {

    String ligne = MegaSerial.readStringUntil('\n');
    ligne.trim();

    if (ligne.length() > 0) {
      dataAdmin.processIncomingData(ligne);
    }
  }
}