#ifndef DEVICE_TELEMETRY_H
#define DEVICE_TELEMETRY_H

// Define DEVICE_MANAGER_URL only in the ignored Settings.local.h to opt in.
#if defined(ESP8266) && defined(DEVICE_MANAGER_URL)
void sendDeviceHeartbeat() {
    static unsigned long last_attempt = 0;
    const unsigned long now = millis();
    if (WiFi.status() != WL_CONNECTED ||
        (last_attempt != 0 && now - last_attempt < 60000UL)) {
        return;
    }
    last_attempt = now ? now : 1;

    WiFiClient telemetry_client;
    HTTPClient telemetry_http;
    telemetry_http.setTimeout(1000);
    if (!telemetry_http.begin(telemetry_client, DEVICE_MANAGER_URL)) {
        return;
    }

    StaticJsonDocument<384> payload;
    char device_id[15];
    snprintf(device_id, sizeof(device_id), "esp8266:%06X", ESP.getChipId());
    payload["device_id"] = device_id;
    payload["device_type"] = "duco-miner";
    payload["board"] = "ESP-12F";
    payload["firmware"] = SOFTWARE_VERSION;
    payload["uptime_s"] = now / 1000UL;
    payload["free_heap"] = ESP.getFreeHeap();
    payload["rssi_dbm"] = WiFi.RSSI();
    payload["hashrate_khs"] = (hashrate + hashrate_core_two) / 1000.0f;
    payload["accepted"] = accepted_share_count;
    payload["rejected"] = share_count >= accepted_share_count
        ? share_count - accepted_share_count : 0;

    String body;
    serializeJson(payload, body);
    telemetry_http.addHeader("Content-Type", "application/json");
    telemetry_http.POST(body);
    telemetry_http.end();
}
#else
inline void sendDeviceHeartbeat() {}
#endif

#endif
