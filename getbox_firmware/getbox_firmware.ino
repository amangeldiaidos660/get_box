#include <Arduino.h>

#include "firmware_variables.h"
#include "wifi_connection.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("GetBox firmware start");
    Serial.print("Device: ");
    Serial.println(DEVICE_NAME);
    Serial.print("Firmware: ");
    Serial.println(FIRMWARE_VERSION);

    connectToWiFi();
}

void loop() {
    // Placeholder for the next stage: MQTT connection and command handling.
    delay(1000);
}
