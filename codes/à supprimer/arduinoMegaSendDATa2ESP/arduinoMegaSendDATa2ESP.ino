#include "capteur.h"

#include <SPI.h>
#include <SD.h>

Capteur capA0(A0, 7, 8);
Capteur capA1(A1, 9, 10);
Capteur reference45K(A2, 11, 12);

const byte SD_CS = 53;
const char* FILE_NAME = "SEC26.CSV";//sechage des capteurs V4;4

void setup()
{
    Serial.begin(115200);

    capA0.begin();
    capA1.begin();
    reference45K.begin();

    Serial.println("Initialisation SD...");

    if (!SD.begin(SD_CS))
    {
        Serial.println("Erreur SD");
        while (1);
    }

    Serial.println("SD OK");

    File f = SD.open(FILE_NAME, FILE_WRITE);

    if (!f)
    {
        Serial.println("Impossible d'ouvrir le fichier");
        while (1);
    }

    if (f.size() == 0)
    {
        f.println("Sechage capteurs le 26/06/2026");
        f.println("temps_ms,"
                  "CapteurBrut1,"
                  "CapteurBrut2,"
                  "Reference45K,"
                  "CapteurFiltre1,"
                  "CapteurFiltre2,"
                  "Reference45KFiltre");
    }

    f.close();

    Serial.println("Fichier pret");
}

void saveValues(float brut1,
                float brut2,
                float brutRef,
                float filt1,
                float filt2,
                float filtRef)
{
    File f = SD.open(FILE_NAME, FILE_WRITE);

    if (!f)
    {
        Serial.println("Erreur fichier");
        return;
    }

    f.print(millis());
    f.print(",");

    f.print(brut1);
    f.print(",");

    f.print(brut2);
    f.print(",");

    f.print(brutRef);
    f.print(",");

    f.print(filt1);
    f.print(",");

    f.print(filt2);
    f.print(",");

    f.println(filtRef);

    f.close();
}

void loop()
{
    capA0.update();
    capA1.update();
    reference45K.update();

    float brut0 = capA0.getRawValue();
    float brut1 = capA1.getRawValue();
    float brutRef = reference45K.getRawValue();

    float filt0 = capA0.getFiltered();
    float filt1 = capA1.getFiltered();
    float filtRef = reference45K.getFiltered();

    saveValues(brut0, brut1, brutRef,
               filt0, filt1, filtRef);

    Serial.print(brut0);
    Serial.print(" ; ");
    Serial.print(brut1);
    Serial.print(" ; ");
    Serial.println(brutRef);

    Serial.print(filt0);
    Serial.print(" ; ");
    Serial.print(filt1);
    Serial.print(" ; ");
    Serial.println(filtRef);

    unsigned long debut = millis();
unsigned long dernierAffichage = debut;

while (millis() - debut < 60UL * 1000 * 10)
{
    if (millis() - dernierAffichage >= 10UL * 1000)
    {
        dernierAffichage = millis();

        unsigned long restant =
            (60UL * 1000 * 10 - (millis() - debut)) / 1000;

        Serial.print("Temps restant : ");
        Serial.print(restant);
        Serial.println(" s");
    }
}

Serial.println("Fin de l'attente.");
}