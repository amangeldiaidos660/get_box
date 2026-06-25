#pragma once

// Wi-Fi credentials for the first setup stage.
// Replace these placeholders with your real network values before flashing.
constexpr char WIFI_SSID[] = "AlmaU.Community";
constexpr char WIFI_PASSWORD[] = "ILoveAlmaU2024!";

// constexpr char WIFI_SSID[] = "A36";
// constexpr char WIFI_PASSWORD[] = "passv2020";

// Optional firmware metadata.
constexpr char DEVICE_NAME[] = "getbox_cell_controller";
constexpr char FIRMWARE_VERSION[] = "0.5.0";


constexpr char BOX_ID[] = "box_001";
constexpr char CONTROLLER_ID[] = "ctrl_01";

constexpr char MQTT_HOST[] = "91.243.71.86";
constexpr uint16_t MQTT_PORT = 8883;
constexpr char MQTT_USERNAME[] = "ctrl_01";
constexpr char MQTT_PASSWORD[] = "fLh+hkxTFMVeqUcHaFJtBF+iOzrVIrgmwmD1OtRBMQo=";


constexpr char MQTT_CLIENT_ID[] = "getbox_box_001_ctrl_01";

constexpr char NTP_SERVER_1[] = "pool.ntp.org";
constexpr char NTP_SERVER_2[] = "time.google.com";

constexpr uint32_t WIFI_CONNECTION_TIMEOUT_MS = 20000;
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000;
constexpr uint32_t TIME_SYNC_TIMEOUT_MS = 20000;
constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 5000;
constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000;

constexpr uint16_t MQTT_COMMAND_MAX_SIZE = 768;

constexpr uint16_t RELAY_DURATION_MIN_MS = 100;
constexpr uint16_t RELAY_DURATION_MAX_MS = 3000;

constexpr uint32_t REED_DEBOUNCE_MS = 100;
constexpr uint32_t DOOR_OPEN_TIMEOUT_MS = 5000;
constexpr uint32_t DOOR_CLOSE_TIMEOUT_MS = 30000;

constexpr size_t REQUEST_ID_MAX_LENGTH = 64;
constexpr size_t RECENT_REQUEST_COUNT = 12;