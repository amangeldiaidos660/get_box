#include <Arduino.h>

#include "firmware_variables.h"
#include "wifi_connection.h"
#include "mqtt_topics.h"
#include "mqtt_connection.h"

void printFirmwareConfiguration() {
    Serial.println();
    Serial.println("GetBox firmware start");

    Serial.print("Device: ");
    Serial.println(DEVICE_NAME);

    Serial.print("Firmware: ");
    Serial.println(FIRMWARE_VERSION);

    Serial.print("Box ID: ");
    Serial.println(BOX_ID);

    Serial.print("Controller ID: ");
    Serial.println(CONTROLLER_ID);

    Serial.print("MQTT server: ");
    Serial.print(MQTT_HOST);
    Serial.print(':');
    Serial.println(MQTT_PORT);

    Serial.print("Command topic: ");
    Serial.println(mqttCommandTopic());

    Serial.print("Status topic: ");
    Serial.println(mqttStatusTopic());

    Serial.print("Event topic: ");
    Serial.println(mqttEventTopic());

    Serial.print("Heartbeat topic: ");
    Serial.println(mqttHeartbeatTopic());

    Serial.print("Error topic: ");
    Serial.println(mqttErrorTopic());
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    printFirmwareConfiguration();
    configureMqtt();

    if (connectToWiFi()) {
        synchronizeSystemTime();
        connectToMqtt();
    }
}

void loop() {
    if (!ensureWiFiConnection()) {
        delay(50);
        return;
    }

    if (!isSystemTimeValid()) {
        if (!synchronizeSystemTime()) {
            delay(50);
            return;
        }
    }

    maintainMqttConnection();

    delay(10);
}
