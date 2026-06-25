#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "firmware_variables.h"


inline bool isSystemTimeValid() {
    return time(nullptr) > 1700000000;
}

inline bool connectToWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");

    const uint32_t startedAt = millis();

    while (
        WiFi.status() != WL_CONNECTED
        && millis() - startedAt < WIFI_CONNECTION_TIMEOUT_MS
    ) {
        delay(500);
        Serial.print('.');
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi connection failed");
        return false;
    }

    Serial.println("Wi-Fi connected");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal RSSI: ");
    Serial.println(WiFi.RSSI());

    return true;
}

inline bool ensureWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    static uint32_t lastReconnectAttemptAt = 0;
    const uint32_t now = millis();

    if (
        lastReconnectAttemptAt != 0
        && now - lastReconnectAttemptAt
            < WIFI_RECONNECT_INTERVAL_MS
    ) {
        return false;
    }

    lastReconnectAttemptAt = now;

    Serial.println(
        "Wi-Fi disconnected. Starting reconnect..."
    );

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    return false;
}

inline bool synchronizeSystemTime() {
    if (isSystemTimeValid()) {
        return true;
    }

    Serial.println("Synchronizing system time...");

    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2
    );

    const uint32_t startedAt = millis();

    while (
        !isSystemTimeValid()
        && millis() - startedAt < TIME_SYNC_TIMEOUT_MS
    ) {
        delay(500);
        Serial.print('.');
    }

    Serial.println();

    if (!isSystemTimeValid()) {
        Serial.println("Time synchronization failed");
        return false;
    }

    Serial.print("System time synchronized: ");
    Serial.println(static_cast<unsigned long>(time(nullptr)));

    return true;
}