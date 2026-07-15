#pragma once

#ifndef QAMELEO_MQTT_H
#define QAMELEO_MQTT_H

#include <PubSubClient.h>
#include <WiFi.h>

class QameleoMQTT {
private:
    WiFiClient wifiClient;
    PubSubClient mqtt;

    const char* broker = "mqtt.qameleo.fr";
    const int port = 1883;

    const char* user = "gamasenseit";
    const char* pass = "gamasenseit";

public:
    QameleoMQTT() : mqtt(wifiClient) {
        mqtt.setServer(broker, port);
    }

    void begin() {
        mqtt.setServer(broker, port);
    }


    bool connect() {
        mqtt.connect("ESP32_CLIENT", user, pass);
        return mqtt.connected();
    }

    void loop() {
        mqtt.loop();
    }

    bool send(const char* topic,
              uint32_t numEnvoi,
              float c1, float c2, float c3, float c4) {

        if (!mqtt.connected()) return false;

        char payload[128];

        snprintf(payload, sizeof(payload),
                 "-1;WAOU1;%lu;%.2f:%.2f:%.2f:%.2f",
                 (unsigned long)numEnvoi,
                 c1, c2, c3, c4);

        return mqtt.publish(topic, payload);
    }
};

#endif