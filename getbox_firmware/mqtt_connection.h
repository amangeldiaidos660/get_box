#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "command_handler.h"
#include "firmware_variables.h"
#include "mqtt_ca_certificate.h"
#include "mqtt_topics.h"
#include "wifi_connection.h"

inline WiFiClientSecure mqttSecureClient;
inline PubSubClient mqttClient(mqttSecureClient);

inline uint32_t lastMqttReconnectAttemptAt = 0;
inline uint32_t lastHeartbeatAt = 0;

inline const String& commandTopic() {
    static const String topic = mqttCommandTopic();
    return topic;
}

inline const String& heartbeatTopic() {
    static const String topic = mqttHeartbeatTopic();
    return topic;
}

inline void mqttMessageCallback(
    char* topic,
    byte* payload,
    unsigned int length
) {
    Serial.println();
    Serial.println("MQTT message received");

    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Payload: ");

    for (unsigned int index = 0; index < length; index++) {
        Serial.print(static_cast<char>(payload[index]));
    }

    Serial.println();

    if (commandTopic() != topic) {
        Serial.println("Ignored message from unexpected topic");
        return;
    }

    handleOpenCellCommand(
        mqttClient,
        payload,
        length
    );
}

inline bool publishHeartbeat(bool retained = false) {
    if (!mqttClient.connected()) {
        return false;
    }

    const String ipAddress = WiFi.localIP().toString();

    char payload[384];

    const int payloadLength = snprintf(
        payload,
        sizeof(payload),
        "{"
        "\"box_id\":\"%s\","
        "\"controller_id\":\"%s\","
        "\"device_type\":\"cell_controller\","
        "\"status\":\"online\","
        "\"ip\":\"%s\","
        "\"uptime_sec\":%lu,"
        "\"free_heap\":%u,"
        "\"firmware_version\":\"%s\""
        "}",
        BOX_ID,
        CONTROLLER_ID,
        ipAddress.c_str(),
        static_cast<unsigned long>(millis() / 1000),
        ESP.getFreeHeap(),
        FIRMWARE_VERSION
    );

    if (
        payloadLength < 0
        || payloadLength >= static_cast<int>(sizeof(payload))
    ) {
        Serial.println("Heartbeat payload is too large");
        return false;
    }

    const bool published = mqttClient.publish(
        heartbeatTopic().c_str(),
        payload,
        retained
    );

    if (published) {
        Serial.print("Heartbeat published: ");
        Serial.println(payload);
    } else {
        Serial.println("Heartbeat publish failed");
    }

    return published;
}

inline void configureMqtt() {
    mqttSecureClient.setCACert(MQTT_CA_CERTIFICATE);

    mqttClient.setServer(
        MQTT_HOST,
        MQTT_PORT
    );

    mqttClient.setCallback(mqttMessageCallback);
    mqttClient.setBufferSize(1024);
    mqttClient.setKeepAlive(30);
    mqttClient.setSocketTimeout(10);
}

inline bool connectToMqtt() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    if (!isSystemTimeValid()) {
        Serial.println(
            "MQTT connection skipped: system time is invalid"
        );
        return false;
    }

    const String willPayload =
        "{"
        "\"box_id\":\"" + String(BOX_ID) + "\","
        "\"controller_id\":\"" + String(CONTROLLER_ID) + "\","
        "\"device_type\":\"cell_controller\","
        "\"status\":\"offline\","
        "\"firmware_version\":\"" + String(FIRMWARE_VERSION) + "\""
        "}";

    Serial.print("Connecting to MQTT ");
    Serial.print(MQTT_HOST);
    Serial.print(':');
    Serial.println(MQTT_PORT);

    const bool connected = mqttClient.connect(
        MQTT_CLIENT_ID,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        heartbeatTopic().c_str(),
        1,
        true,
        willPayload.c_str(),
        true
    );

    if (!connected) {
        Serial.print("MQTT connection failed, state: ");
        Serial.println(mqttClient.state());
        return false;
    }

    Serial.println("MQTT connected");

    const bool subscribed = mqttClient.subscribe(
        commandTopic().c_str(),
        1
    );

    if (!subscribed) {
        Serial.println("MQTT command subscription failed");
        mqttClient.disconnect();
        return false;
    }

    Serial.print("Subscribed to: ");
    Serial.println(commandTopic());

    publishHeartbeat(true);
    lastHeartbeatAt = millis();

    return true;
}

inline void maintainMqttConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!mqttClient.connected()) {
        const uint32_t now = millis();

        if (
            lastMqttReconnectAttemptAt == 0
            || now - lastMqttReconnectAttemptAt
                >= MQTT_RECONNECT_INTERVAL_MS
        ) {
            lastMqttReconnectAttemptAt = now;

            if (connectToMqtt()) {
                lastMqttReconnectAttemptAt = 0;
            }
        }

        return;
    }

    mqttClient.loop();

    const uint32_t now = millis();

    if (now - lastHeartbeatAt >= HEARTBEAT_INTERVAL_MS) {
        if (publishHeartbeat(false)) {
            lastHeartbeatAt = now;
        }
    }
}