# ESP-12F device telemetry

The optional heartbeat adapter sends a hardware-based ID and mining telemetry to the private `asvok/esp-device-manager` registry. It does not send Wi-Fi settings, the Duino-Coin account, or the mining key. Mining continues when the registry is unavailable; an attempted heartbeat may pause the loop for up to one second every 60 seconds.

The adapter supports ESP8266 and ESP32 boards. Define `DEVICE_MANAGER_BOARD` in the ignored local settings to show the physical model, for example `"ESP-12F"`, `"D1 Mini"`, or `"ESP32-C3"`. Without it, the registry reports the chipset family.

After the registry is running on `stb-lab`, add this line to the ignored `ESP_Code/Settings.local.h` on the ESP build machine:

```cpp
#define DEVICE_MANAGER_URL "http://<stb-lab-LAN-IP>:8088/v1/heartbeat"
```

Use the LAN address of `stb-lab`; keep the API port inside a trusted network. Compile and flash the `ESP-12F` PlatformIO environment. The registry will list the device under `esp8266:<chip-id>`. Its friendly name can be set on the server with `admin.py rename`.

Remove the definition to disable telemetry. This pilot has no remote configuration or OTA control. The hardware ID remains stable for the same ESP chip, while a friendly name can change independently.

## HTTPS server-only + token

The recommended secure mode uses the server certificate and one token unique to each device. Issue the token locally on `stb-lab`, copy it directly into this ignored file, and never copy it to Git or chat:

```cpp
#define DEVICE_MANAGER_URL "https://stb-lab.home:8088/v1/heartbeat"
#define DEVICE_MANAGER_HTTPS
#define DEVICE_MANAGER_TOKEN "token-issued-for-this-device-only"
#define DEVICE_MANAGER_SERVER_CA R"EOF(-----BEGIN CERTIFICATE-----
...contents of ca.crt...
-----END CERTIFICATE-----
)EOF"
```

The device waits for NTP time before sending HTTPS telemetry, because it validates certificate dates. Mining continues if NTP or the registry is unavailable. `DEVICE_MANAGER_TOKEN` is sent only in the `X-Device-Token` HTTPS header and never appears in the heartbeat JSON.

## mTLS optional

For a later mTLS rollout, add `DEVICE_MANAGER_TLS`, `DEVICE_MANAGER_CLIENT_CERT`, and `DEVICE_MANAGER_CLIENT_KEY` alongside the HTTPS settings. mTLS requires a client certificate for every HTTPS connection and is not needed for the recommended server-only HTTPS plus token mode.
