# ESP-12F device telemetry

The optional heartbeat adapter sends a hardware-based ID and mining telemetry to the private `asvok/esp-device-manager` registry. It does not send Wi-Fi settings, the Duino-Coin account, or the mining key. Mining continues when the registry is unavailable; an attempted heartbeat may pause the loop for up to one second every 60 seconds.

After the registry is running on `stb-lab`, add this line to the ignored `ESP_Code/Settings.local.h` on the ESP build machine:

```cpp
#define DEVICE_MANAGER_URL "http://<stb-lab-LAN-IP>:8088/v1/heartbeat"
```

Use the LAN address of `stb-lab`; keep the API port inside a trusted network. Compile and flash the `ESP-12F` PlatformIO environment. The registry will list the device under `esp8266:<chip-id>`. Its friendly name can be set on the server with `admin.py rename`.

Remove the definition to disable telemetry. This pilot has no remote configuration or OTA control. The hardware ID remains stable for the same ESP chip, while a friendly name can change independently.
