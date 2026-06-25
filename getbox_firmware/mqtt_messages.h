#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "firmware_variables.h"
#include "mqtt_topics.h"
#include "request_cache.h"

inline bool publishCommandStatus(
    PubSubClient& client,
    const char* requestId,
    uint16_t cellId,
    const char* status,
    const char* errorCode = nullptr,
    const char* message = nullptr,
    uint32_t durationMs = 0
) {
    JsonDocument response;

    response["request_id"] = requestId;
    response["box_id"] = BOX_ID;
    response["controller_id"] = CONTROLLER_ID;
    response["cell_id"] = cellId;
    response["status"] = status;

    if (errorCode != nullptr && errorCode[0] != '\0') {
        response["error_code"] = errorCode;
    }

    if (message != nullptr && message[0] != '\0') {
        response["message"] = message;
    }

    if (durationMs > 0) {
        response["duration_ms"] = durationMs;
    }

    char payload[512];

    const size_t length = serializeJson(
        response,
        payload,
        sizeof(payload)
    );

    if (length == 0) {
        Serial.println("Status serialization failed");
        return false;
    }

    const String topic = mqttStatusTopic();

    const bool published = client.publish(
        topic.c_str(),
        reinterpret_cast<const uint8_t*>(payload),
        length,
        false
    );

    Serial.print("Command status: ");
    Serial.println(payload);

    if (requestId != nullptr && requestId[0] != '\0') {
        saveRequestStatus(
            requestId,
            cellId,
            status,
            errorCode
        );
    }

    return published;
}

inline bool publishCellEvent(
    PubSubClient& client,
    const char* requestId,
    uint16_t cellId,
    const char* event,
    const char* doorState
) {
    JsonDocument response;

    if (requestId != nullptr && requestId[0] != '\0') {
        response["request_id"] = requestId;
    }

    response["box_id"] = BOX_ID;
    response["controller_id"] = CONTROLLER_ID;
    response["cell_id"] = cellId;
    response["event"] = event;
    response["door_state"] = doorState;

    char payload[512];

    const size_t length = serializeJson(
        response,
        payload,
        sizeof(payload)
    );

    if (length == 0) {
        return false;
    }

    const String topic = mqttEventTopic();

    const bool published = client.publish(
        topic.c_str(),
        reinterpret_cast<const uint8_t*>(payload),
        length,
        false
    );

    Serial.print("Cell event: ");
    Serial.println(payload);

    return published;
}

inline bool publishDeviceError(
    PubSubClient& client,
    const char* requestId,
    uint16_t cellId,
    const char* errorCode,
    const char* message
) {
    JsonDocument response;

    if (requestId != nullptr && requestId[0] != '\0') {
        response["request_id"] = requestId;
    }

    response["box_id"] = BOX_ID;
    response["controller_id"] = CONTROLLER_ID;

    if (cellId > 0) {
        response["cell_id"] = cellId;
    }

    response["status"] = "error";
    response["error_code"] = errorCode;
    response["message"] = message;

    char payload[512];

    const size_t length = serializeJson(
        response,
        payload,
        sizeof(payload)
    );

    if (length == 0) {
        return false;
    }

    const String topic = mqttErrorTopic();

    const bool published = client.publish(
        topic.c_str(),
        reinterpret_cast<const uint8_t*>(payload),
        length,
        false
    );

    Serial.print("Device error: ");
    Serial.println(payload);

    if (requestId != nullptr && requestId[0] != '\0') {
        saveRequestStatus(
            requestId,
            cellId,
            "error",
            errorCode
        );
    }

    return published;
}