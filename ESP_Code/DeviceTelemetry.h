#ifndef DEVICE_TELEMETRY_H
#define DEVICE_TELEMETRY_H

// Define DEVICE_MANAGER_URL only in the ignored Settings.local.h to opt in.
#if defined(ESP8266) && defined(DEVICE_MANAGER_URL)
#if defined(DEVICE_MANAGER_TLS)
    #include <time.h>
    #include <WiFiClientSecureBearSSL.h>

    #ifndef DEVICE_MANAGER_SERVER_CA
        #error "DEVICE_MANAGER_SERVER_CA is required when DEVICE_MANAGER_TLS is enabled"
    #endif
    #ifndef DEVICE_MANAGER_CLIENT_CERT
        #error "DEVICE_MANAGER_CLIENT_CERT is required when DEVICE_MANAGER_TLS is enabled"
    #endif
    #ifndef DEVICE_MANAGER_CLIENT_KEY
        #error "DEVICE_MANAGER_CLIENT_KEY is required when DEVICE_MANAGER_TLS is enabled"
    #endif

bool deviceManagerTimeReady() {
    static bool requested = false;
    const time_t minimum_valid_time = 1704067200; // 2024-01-01 UTC
    if (time(nullptr) >= minimum_valid_time) {
        return true;
    }
    if (!requested) {
        configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
        requested = true;
    }
    return false;
}
#endif

void sendDeviceHeartbeat() {
    static unsigned long last_attempt = 0;
    const unsigned long now = millis();
    if (WiFi.status() != WL_CONNECTED ||
        (last_attempt != 0 && now - last_attempt < 60000UL)) {
        return;
    }
    last_attempt = now ? now : 1;

#if defined(DEVICE_MANAGER_TLS)
    if (!deviceManagerTimeReady()) {
        return;
    }
    BearSSL::WiFiClientSecure telemetry_client;
    BearSSL::X509List server_ca(DEVICE_MANAGER_SERVER_CA);
    BearSSL::X509List client_cert(DEVICE_MANAGER_CLIENT_CERT);
    BearSSL::PrivateKey client_key(DEVICE_MANAGER_CLIENT_KEY);
    telemetry_client.setTrustAnchors(&server_ca);
    telemetry_client.setClientECCert(&client_cert, &client_key,
                                     BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, BR_KEYTYPE_EC);
    telemetry_client.setBufferSizes(512, 512);
#else
    WiFiClient telemetry_client;
#endif
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
    payload["capabilities"].add("mining");
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
