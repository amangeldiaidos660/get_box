#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <cstring>

#include "cell_config.h"
#include "cell_controller.h"
#include "firmware_variables.h"
#include "mqtt_messages.h"
#include "request_cache.h"

inline bool publishPayloadError(
    PubSubClient& client,
    const char* errorCode,
    const char* message
) {
    return publishDeviceError(
        client,
        nullptr,
        0,
        errorCode,
        message
    );
}

inline void handleOpenCellCommand(
    PubSubClient& client,
    const byte* payload,
    unsigned int length
) {
    if (length == 0 || length > MQTT_COMMAND_MAX_SIZE) {
        publishPayloadError(
            client,
            "invalid_payload",
            "Command payload size is invalid"
        );
        return;
    }

    JsonDocument document;

    const DeserializationError error =
        deserializeJson(
            document,
            payload,
            length
        );

    if (error) {
        publishPayloadError(
            client,
            "invalid_payload",
            error.c_str()
        );
        return;
    }

    if (
        !document["request_id"].is<const char*>()
        || !document["action"].is<const char*>()
        || !document["box_id"].is<const char*>()
        || !document["controller_id"].is<const char*>()
        || !document["cell_id"].is<int>()
        || !document["duration_ms"].is<int>()
    ) {
        publishPayloadError(
            client,
            "invalid_payload",
            "Required fields are missing or invalid"
        );
        return;
    }

    const char* requestId = document["request_id"];
    const char* action = document["action"];
    const char* boxId = document["box_id"];
    const char* controllerId =
        document["controller_id"];

    const int cellIdValue = document["cell_id"];
    const int durationMs = document["duration_ms"];

    if (
        requestId == nullptr
        || requestId[0] == '\0'
        || strlen(requestId) > REQUEST_ID_MAX_LENGTH
    ) {
        publishPayloadError(
            client,
            "invalid_payload",
            "request_id is empty or too long"
        );
        return;
    }

    RequestCacheItem* cached =
        findCachedRequest(requestId);

    if (cached != nullptr) {
        Serial.println(
            "Duplicate request_id received"
        );

        publishCommandStatus(
            client,
            cached->requestId,
            cached->cellId,
            cached->status,
            cached->errorCode[0] != '\0'
                ? cached->errorCode
                : nullptr,
            "Duplicate command was not executed"
        );

        return;
    }

    if (strcmp(boxId, BOX_ID) != 0) {
        publishCommandStatus(
            client,
            requestId,
            cellIdValue > 0 ? cellIdValue : 0,
            "error",
            "wrong_box"
        );
        return;
    }

    if (strcmp(controllerId, CONTROLLER_ID) != 0) {
        publishCommandStatus(
            client,
            requestId,
            cellIdValue > 0 ? cellIdValue : 0,
            "error",
            "wrong_controller"
        );
        return;
    }

    if (strcmp(action, "open_cell") != 0) {
        publishCommandStatus(
            client,
            requestId,
            cellIdValue > 0 ? cellIdValue : 0,
            "error",
            "unsupported_action"
        );
        return;
    }

    if (cellIdValue < 1 || cellIdValue > UINT16_MAX) {
        publishCommandStatus(
            client,
            requestId,
            0,
            "error",
            "unknown_cell"
        );
        return;
    }

    const uint16_t cellId =
        static_cast<uint16_t>(cellIdValue);

    if (findCellById(cellId) == nullptr) {
        publishCommandStatus(
            client,
            requestId,
            cellId,
            "error",
            "unknown_cell"
        );
        return;
    }

    if (
        durationMs < RELAY_DURATION_MIN_MS
        || durationMs > RELAY_DURATION_MAX_MS
    ) {
        publishCommandStatus(
            client,
            requestId,
            cellId,
            "error",
            "invalid_duration"
        );
        return;
    }

    if (isCellCommandActive(cellId)) {
        publishCommandStatus(
            client,
            requestId,
            cellId,
            "error",
            "cell_busy",
            "This cell already has an active command"
        );
        return;
    }

    publishCommandStatus(
        client,
        requestId,
        cellId,
        "accepted"
    );

    activateCellRelay(
        client,
        cellId,
        requestId,
        static_cast<uint32_t>(durationMs)
    );
}