#pragma once

#include <Arduino.h>

struct CellConfig {
    uint16_t cellId;
    uint8_t relayPin;
    uint8_t reedPin;
    uint8_t relayActiveLevel;
    uint8_t reedClosedLevel;
    uint8_t reedInputMode;
    bool reedRequiresExternalPulldown;
};

constexpr CellConfig CELLS[] = {
    {1, 13, 34, LOW, HIGH, INPUT, true},
    {2, 14, 35, LOW, HIGH, INPUT, true},
    {3, 16, 36, LOW, HIGH, INPUT, true},
    {4, 17, 39, LOW, HIGH, INPUT, true},
    {5, 18, 32, LOW, HIGH, INPUT_PULLDOWN, false},
    {6, 19, 33, LOW, HIGH, INPUT_PULLDOWN, false},
};

constexpr size_t CELL_COUNT =
    sizeof(CELLS) / sizeof(CELLS[0]);

inline uint8_t relayInactiveLevel(
    const CellConfig& cell
) {
    return cell.relayActiveLevel == HIGH ? LOW : HIGH;
}

inline const CellConfig* findCellById(
    uint16_t cellId
) {
    for (size_t index = 0; index < CELL_COUNT; index++) {
        if (CELLS[index].cellId == cellId) {
            return &CELLS[index];
        }
    }

    return nullptr;
}

inline int findCellIndexById(uint16_t cellId) {
    for (size_t index = 0; index < CELL_COUNT; index++) {
        if (CELLS[index].cellId == cellId) {
            return static_cast<int>(index);
        }
    }

    return -1;
}

inline void printCellMap() {
    Serial.println();
    Serial.println("Configured cells:");

    for (size_t index = 0; index < CELL_COUNT; index++) {
        const CellConfig& cell = CELLS[index];

        Serial.print("Cell ");
        Serial.print(cell.cellId);

        Serial.print(": relay GPIO");
        Serial.print(cell.relayPin);

        Serial.print(", reed GPIO");
        Serial.print(cell.reedPin);

        Serial.print(", relay active ");
        Serial.print(
            cell.relayActiveLevel == HIGH
                ? "HIGH"
                : "LOW"
        );

        Serial.print(", reed closed ");
        Serial.print(
            cell.reedClosedLevel == HIGH
                ? "HIGH"
                : "LOW"
        );

        if (cell.reedRequiresExternalPulldown) {
            Serial.print(", external pull-down required");
        }

        Serial.println();
    }
}