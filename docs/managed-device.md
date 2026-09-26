# Managed configuration and pull OTA

The ESP miner can opt in to ESP Device Manager by defining the settings shown in
`ESP_Code/Settings.local.example.h`. The local settings file is ignored by Git.

## Bootstrap

1. Issue a unique device token in ESP Device Manager.
2. Generate a random firmware signing key of at least 32 characters and configure
   the same value as `DEVICE_FIRMWARE_SIGNING_KEY` on the server and
   `DEVICE_MANAGER_FIRMWARE_KEY` in the private device build.
3. Set the exact physical board in `DEVICE_MANAGER_BOARD`.
4. Set `DEVICE_FIRMWARE_VERSION` to the artifact version that will be uploaded.
5. Build and flash the managed firmware once over USB.

After bootstrap, the device posts management state with its authenticated
heartbeat. A pending complete configuration is saved to EEPROM and restarted.
The previous Wi-Fi credentials remain available until the new connection sends a
successful heartbeat. If the candidate cannot connect within 30 seconds, the
device restores the previous credentials and restarts.

## Full application OTA

The device accepts an artifact only when all of these checks pass:

- the artifact board exactly matches `DEVICE_MANAGER_BOARD`;
- the manifest HMAC-SHA256 matches the private firmware signing key;
- the image fits the free sketch/OTA area;
- the downloaded size and SHA-256 match the manifest.

The image is streamed into the inactive application area and activated only after
verification. Normal pull OTA replaces the application, not the bootloader or
partition layout. Keep USB recovery available and always deploy to one pilot
device before a wider rollout.

Legacy LAN `ArduinoOTA` is disabled by default. Define
`ENABLE_LOCAL_ARDUINO_OTA` only when explicitly needed on a trusted LAN.
