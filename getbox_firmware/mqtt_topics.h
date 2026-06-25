#pragma once

#include <Arduino.h>

#include "firmware_variables.h"

inline String mqttCommandTopic() {
    return "getbox/" + String(BOX_ID)
        + "/cell/" + String(CONTROLLER_ID)
        + "/cmd";
}

inline String mqttStatusTopic() {
    return "getbox/" + String(BOX_ID)
        + "/cell/" + String(CONTROLLER_ID)
        + "/status";
}

inline String mqttEventTopic() {
    return "getbox/" + String(BOX_ID)
        + "/cell/" + String(CONTROLLER_ID)
        + "/event";
}

inline String mqttHeartbeatTopic() {
    return "getbox/" + String(BOX_ID)
        + "/device/" + String(CONTROLLER_ID)
        + "/heartbeat";
}

inline String mqttErrorTopic() {
    return "getbox/" + String(BOX_ID)
        + "/device/" + String(CONTROLLER_ID)
        + "/error";
}