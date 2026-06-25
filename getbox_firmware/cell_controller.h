#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <cstring>

#include "cell_config.h"
#include "firmware_variables.h"
#include "mqtt_messages.h"

struct CellRuntime {
    bool relayActive;
    uint32_t relayDeactivateAt;

    bool rawDoorClosed;
    bool stableDoorClosed;
    uint32_t rawStateChangedAt;

    bool commandActive;
    bool waitingForDoorOpen;
    bool doorOpenedByCommand;

    uint32_t doorOpenDeadlineAt;
    uint32_t doorCloseDeadlineAt;

    uint32_t relayDurationMs;

    char requestId[REQUEST_ID_MAX_LENGTH + 1];
};

inline CellRuntime cellRuntime[CELL_COUNT] = {};

inline bool deadlineReached(
    uint32_t now,
    uint32_t deadline
) {
    return static_cast<int32_t>(now - deadline) >= 0;
}

inline void clearCellCommand(CellRuntime& runtime) {
    runtime.commandActive = false;
    runtime.waitingForDoorOpen = false;
    runtime.doorOpenedByCommand = false;
    runtime.doorOpenDeadlineAt = 0;
    runtime.doorCloseDeadlineAt = 0;
    runtime.relayDurationMs = 0;
    runtime.requestId[0] = '\0';
}

inline void initializeCellHardware() {
    Serial.println();
    Serial.println("Initializing cell GPIO...");

    for (size_t index = 0; index < CELL_COUNT; index++) {
        const CellConfig& cell = CELLS[index];
        CellRuntime& runtime = cellRuntime[index];

        digitalWrite(
            cell.relayPin,
            relayInactiveLevel(cell)
        );

        pinMode(cell.relayPin, OUTPUT);
        pinMode(cell.reedPin, cell.reedInputMode);

        const bool doorClosed =
            digitalRead(cell.reedPin)
            == cell.reedClosedLevel;

        runtime.relayActive = false;
        runtime.relayDeactivateAt = 0;

        runtime.rawDoorClosed = doorClosed;
        runtime.stableDoorClosed = doorClosed;
        runtime.rawStateChangedAt = millis();

        clearCellCommand(runtime);

        Serial.print("Cell ");
        Serial.print(cell.cellId);
        Serial.print(" initialized, door: ");
        Serial.println(
            doorClosed ? "closed" : "open"
        );
    }
}

inline bool isCellCommandActive(uint16_t cellId) {
    const int index = findCellIndexById(cellId);

    if (index < 0) {
        return false;
    }

    return cellRuntime[index].commandActive;
}

inline bool activateCellRelay(
    PubSubClient& client,
    uint16_t cellId,
    const char* requestId,
    uint32_t durationMs
) {
    const int index = findCellIndexById(cellId);

    if (index < 0) {
        return false;
    }

    const CellConfig& cell = CELLS[index];
    CellRuntime& runtime = cellRuntime[index];

    if (runtime.commandActive || runtime.relayActive) {
        publishCommandStatus(
            client,
            requestId,
            cellId,
            "error",
            "cell_busy",
            "Cell already has an active command"
        );

        return false;
    }

    copyRequestText(
        runtime.requestId,
        sizeof(runtime.requestId),
        requestId
    );

    runtime.commandActive = true;
    runtime.waitingForDoorOpen = true;
    runtime.doorOpenedByCommand = false;
    runtime.relayDurationMs = durationMs;

    const uint32_t now = millis();

    runtime.relayActive = true;
    runtime.relayDeactivateAt = now + durationMs;
    runtime.doorOpenDeadlineAt =
        now + DOOR_OPEN_TIMEOUT_MS;

    digitalWrite(
        cell.relayPin,
        cell.relayActiveLevel
    );

    Serial.print("Relay activated for cell ");
    Serial.print(cellId);
    Serial.print(" on GPIO");
    Serial.println(cell.relayPin);

    publishCommandStatus(
        client,
        requestId,
        cellId,
        "relay_activated",
        nullptr,
        nullptr,
        durationMs
    );

    return true;
}

inline void processRelayTimers() {
    const uint32_t now = millis();

    for (size_t index = 0; index < CELL_COUNT; index++) {
        const CellConfig& cell = CELLS[index];
        CellRuntime& runtime = cellRuntime[index];

        if (
            runtime.relayActive
            && deadlineReached(
                now,
                runtime.relayDeactivateAt
            )
        ) {
            digitalWrite(
                cell.relayPin,
                relayInactiveLevel(cell)
            );

            runtime.relayActive = false;
            runtime.relayDeactivateAt = 0;

            Serial.print("Relay deactivated for cell ");
            Serial.println(cell.cellId);
        }
    }
}

inline void handleDoorOpened(
    PubSubClient& client,
    size_t index
) {
    const CellConfig& cell = CELLS[index];
    CellRuntime& runtime = cellRuntime[index];

    if (
        runtime.commandActive
        && runtime.waitingForDoorOpen
    ) {
        runtime.waitingForDoorOpen = false;
        runtime.doorOpenedByCommand = true;
        runtime.doorOpenDeadlineAt = 0;
        runtime.doorCloseDeadlineAt =
            millis() + DOOR_CLOSE_TIMEOUT_MS;

        publishCommandStatus(
            client,
            runtime.requestId,
            cell.cellId,
            "door_opened"
        );

        publishCellEvent(
            client,
            runtime.requestId,
            cell.cellId,
            "normal_open",
            "open"
        );

        return;
    }

    publishCellEvent(
        client,
        nullptr,
        cell.cellId,
        "forced_open",
        "open"
    );
}

inline void handleDoorClosed(
    PubSubClient& client,
    size_t index
) {
    const CellConfig& cell = CELLS[index];
    CellRuntime& runtime = cellRuntime[index];

    if (
        runtime.commandActive
        && runtime.doorOpenedByCommand
    ) {
        publishCommandStatus(
            client,
            runtime.requestId,
            cell.cellId,
            "door_closed"
        );

        publishCellEvent(
            client,
            runtime.requestId,
            cell.cellId,
            "door_closed",
            "closed"
        );

        clearCellCommand(runtime);
        return;
    }

    publishCellEvent(
        client,
        nullptr,
        cell.cellId,
        "door_closed",
        "closed"
    );
}

inline void processReedSwitches(
    PubSubClient& client
) {
    const uint32_t now = millis();

    for (size_t index = 0; index < CELL_COUNT; index++) {
        const CellConfig& cell = CELLS[index];
        CellRuntime& runtime = cellRuntime[index];

        const bool currentDoorClosed =
            digitalRead(cell.reedPin)
            == cell.reedClosedLevel;

        if (currentDoorClosed != runtime.rawDoorClosed) {
            runtime.rawDoorClosed = currentDoorClosed;
            runtime.rawStateChangedAt = now;
        }

        if (
            runtime.rawDoorClosed
                != runtime.stableDoorClosed
            && now - runtime.rawStateChangedAt
                >= REED_DEBOUNCE_MS
        ) {
            runtime.stableDoorClosed =
                runtime.rawDoorClosed;

            Serial.print("Cell ");
            Serial.print(cell.cellId);
            Serial.print(" door: ");
            Serial.println(
                runtime.stableDoorClosed
                    ? "closed"
                    : "open"
            );

            if (runtime.stableDoorClosed) {
                handleDoorClosed(client, index);
            } else {
                handleDoorOpened(client, index);
            }
        }
    }
}

inline void processDoorTimeouts(
    PubSubClient& client
) {
    const uint32_t now = millis();

    for (size_t index = 0; index < CELL_COUNT; index++) {
        const CellConfig& cell = CELLS[index];
        CellRuntime& runtime = cellRuntime[index];

        if (
            runtime.commandActive
            && runtime.waitingForDoorOpen
            && runtime.doorOpenDeadlineAt != 0
            && deadlineReached(
                now,
                runtime.doorOpenDeadlineAt
            )
        ) {
            publishDeviceError(
                client,
                runtime.requestId,
                cell.cellId,
                "door_not_opened_timeout",
                "Door did not open after relay activation"
            );

            clearCellCommand(runtime);
            continue;
        }

        if (
            runtime.commandActive
            && runtime.doorOpenedByCommand
            && !runtime.stableDoorClosed
            && runtime.doorCloseDeadlineAt != 0
            && deadlineReached(
                now,
                runtime.doorCloseDeadlineAt
            )
        ) {
            publishDeviceError(
                client,
                runtime.requestId,
                cell.cellId,
                "door_left_open",
                "Door stayed open after timeout"
            );

            clearCellCommand(runtime);
        }
    }
}

inline void maintainCells(
    PubSubClient& client
) {
    processRelayTimers();
    processReedSwitches(client);
    processDoorTimeouts(client);
}