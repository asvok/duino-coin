// Copy this file to Settings.local.h and enter your private settings there.
// Settings.local.h is ignored by Git.
#ifndef DUCO_SETTINGS_LOCAL_H
#define DUCO_SETTINGS_LOCAL_H

extern char *DUCO_USER = "your_username";
extern char *MINER_KEY = "";
extern char *RIG_IDENTIFIER = "Auto";
extern const char SSID[] = "your_wifi_name";
extern const char PASSWORD[] = "your_wifi_password";

// Optional: report the physical board accurately in device management.
// #define DEVICE_MANAGER_BOARD "ESP-12F"

// Optional HTTPS device-management setup. Keep the real token and certificate
// value only in Settings.local.h, never in Git.
// #define DEVICE_MANAGER_URL "https://stb-lab.home:8088/v1/heartbeat"
// #define DEVICE_MANAGER_HTTPS
// #define DEVICE_MANAGER_TOKEN "unique token issued for this device"
// Required for signed pull OTA. Use the same 32+ character value as
// DEVICE_FIRMWARE_SIGNING_KEY on ESP Device Manager.
// #define DEVICE_MANAGER_FIRMWARE_KEY "replace-with-a-long-random-signing-key"
// Set this to the exact version registered with the uploaded .bin artifact.
// #define DEVICE_FIRMWARE_VERSION "4.3-managed.1"
// #define DEVICE_MANAGER_SERVER_CA \
//   "-----BEGIN CERTIFICATE-----\\n" \
//   "...one PEM line at a time...\\n" \
//   "-----END CERTIFICATE-----\\n"
//
// Optional mTLS adds a client certificate and private key to HTTPS.
// #define DEVICE_MANAGER_TLS
// #define DEVICE_MANAGER_CLIENT_CERT R"EOF(-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----\n)EOF"
// #define DEVICE_MANAGER_CLIENT_KEY R"EOF(-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n)EOF"

// Optional legacy LAN push OTA. Managed pull OTA does not require this.
// Keep disabled unless the LAN is trusted and ArduinoOTA is explicitly needed.
// #define ENABLE_LOCAL_ARDUINO_OTA

#endif
