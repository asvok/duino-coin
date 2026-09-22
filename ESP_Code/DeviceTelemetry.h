#ifndef DEVICE_TELEMETRY_H
#define DEVICE_TELEMETRY_H

// Define DEVICE_MANAGER_URL only in the ignored Settings.local.h to opt in.
#if defined(DEVICE_MANAGER_URL)
#ifndef DEVICE_MANAGER_BOARD
    #if defined(ESP8266)
        #define DEVICE_MANAGER_BOARD "ESP8266"
    #else
        #define DEVICE_MANAGER_BOARD "ESP32"
    #endif
#endif
#if defined(DEVICE_MANAGER_TLS) || defined(DEVICE_MANAGER_HTTPS)
    #include <time.h>

    #if defined(ESP8266)
        #include <WiFiClientSecureBearSSL.h>
    #endif

    #ifndef DEVICE_MANAGER_SERVER_CA
        #error "DEVICE_MANAGER_SERVER_CA is required when HTTPS telemetry is enabled"
    #endif
    #if defined(DEVICE_MANAGER_TLS)
    #ifndef DEVICE_MANAGER_CLIENT_CERT
        #error "DEVICE_MANAGER_CLIENT_CERT is required when DEVICE_MANAGER_TLS is enabled"
    #endif
    #ifndef DEVICE_MANAGER_CLIENT_KEY
        #error "DEVICE_MANAGER_CLIENT_KEY is required when DEVICE_MANAGER_TLS is enabled"
    #endif
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

#if defined(DEVICE_MANAGER_TLS) || defined(DEVICE_MANAGER_HTTPS)
    if (!deviceManagerTimeReady()) {
        return;
    }
    #if defined(ESP8266)
    BearSSL::WiFiClientSecure telemetry_client;
    BearSSL::X509List server_ca(DEVICE_MANAGER_SERVER_CA);
    telemetry_client.setTrustAnchors(&server_ca);
    #if defined(DEVICE_MANAGER_TLS)
    BearSSL::X509List client_cert(DEVICE_MANAGER_CLIENT_CERT);
    BearSSL::PrivateKey client_key(DEVICE_MANAGER_CLIENT_KEY);
    telemetry_client.setClientECCert(&client_cert, &client_key,
                                     BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, BR_KEYTYPE_EC);
    #endif
    telemetry_client.setBufferSizes(512, 512);
    #else
    WiFiClientSecure telemetry_client;
    telemetry_client.setCACert(DEVICE_MANAGER_SERVER_CA);
    #if defined(DEVICE_MANAGER_TLS)
    telemetry_client.setCertificate(DEVICE_MANAGER_CLIENT_CERT);
    telemetry_client.setPrivateKey(DEVICE_MANAGER_CLIENT_KEY);
    #endif
    #endif
#else
    WiFiClient telemetry_client;
#endif
    HTTPClient telemetry_http;
    telemetry_http.setTimeout(1000);
    if (!telemetry_http.begin(telemetry_client, DEVICE_MANAGER_URL)) {
        return;
    }

    StaticJsonDocument<384> payload;
    char device_id[24];
    #if defined(ESP8266)
    snprintf(device_id, sizeof(device_id), "esp8266:%06X", ESP.getChipId());
    #else
    snprintf(device_id, sizeof(device_id), "esp32:%012llX", ESP.getEfuseMac());
    #endif
    payload["device_id"] = device_id;
    payload["device_type"] = "duco-miner";
    payload["board"] = DEVICE_MANAGER_BOARD;
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
    #if defined(DEVICE_MANAGER_TOKEN)
    telemetry_http.addHeader("X-Device-Token", DEVICE_MANAGER_TOKEN);
    #endif
    telemetry_http.POST(body);
    telemetry_http.end();
}
#else
inline void sendDeviceHeartbeat() {}
#endif

#endif
