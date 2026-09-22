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

// Optional mTLS device-management setup. Keep real certificate values only in
// Settings.local.h, never in Git. Define DEVICE_MANAGER_URL and these values:
// #define DEVICE_MANAGER_TLS
// #define DEVICE_MANAGER_SERVER_CA R"EOF(-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----\n)EOF"
// #define DEVICE_MANAGER_CLIENT_CERT R"EOF(-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----\n)EOF"
// #define DEVICE_MANAGER_CLIENT_KEY R"EOF(-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n)EOF"

#endif
