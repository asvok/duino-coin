#ifndef MANAGED_DEVICE_H
#define MANAGED_DEVICE_H

#if defined(DEVICE_MANAGER_URL)

#include <ArduinoJson.h>
#include <EEPROM.h>

#if defined(ESP8266)
  #include <Updater.h>
  #include <bearssl/bearssl.h>
#else
  #include <Update.h>
  #include <esp_ota_ops.h>
  #include <mbedtls/md.h>
  #include <mbedtls/sha256.h>
#endif

#ifndef DEVICE_MANAGER_FIRMWARE_KEY
  #define DEVICE_MANAGER_FIRMWARE_KEY ""
#endif
#ifndef DEVICE_MANAGER_TOKEN
  #define DEVICE_MANAGER_TOKEN ""
#endif

static const uint32_t MANAGED_MAGIC = 0x4455434F;
static const size_t MANAGED_EEPROM_SIZE = 768;

struct ManagedSettings {
  uint32_t magic;
  uint32_t crc;
  uint32_t version;
  uint8_t pending;
  uint8_t rollback;
  char ssid[33];
  char wifi_password[64];
  char duco_user[41];
  char miner_key[65];
  char rig_identifier[25];
  char previous_ssid[33];
  char previous_wifi_password[64];
};

static ManagedSettings managedSettings = {};
static String managedConfigStatus = "pending";
static String managedOtaStatus = "idle";
static String managedOtaError = "";
static int managedOtaProgress = 0;

String managedDeviceId() {
  char value[24];
#if defined(ESP8266)
  snprintf(value, sizeof(value), "esp8266:%06X", ESP.getChipId());
#else
  snprintf(value, sizeof(value), "esp32:%012llX", ESP.getEfuseMac());
#endif
  return String(value);
}

uint32_t managedCrc(const uint8_t *data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xEDB88320 & (-(int32_t)(crc & 1)));
  }
  return ~crc;
}

uint32_t managedSettingsCrc(const ManagedSettings &settings) {
  ManagedSettings copy = settings;
  copy.crc = 0;
  return managedCrc(reinterpret_cast<const uint8_t *>(&copy), sizeof(copy));
}

void managedSave() {
  managedSettings.magic = MANAGED_MAGIC;
  managedSettings.crc = managedSettingsCrc(managedSettings);
  EEPROM.put(0, managedSettings);
  EEPROM.commit();
}

void managedBegin(MiningConfig *configuration) {
  EEPROM.begin(MANAGED_EEPROM_SIZE);
  EEPROM.get(0, managedSettings);
  if (managedSettings.magic != MANAGED_MAGIC || managedSettings.crc != managedSettingsCrc(managedSettings)) {
    memset(&managedSettings, 0, sizeof(managedSettings));
    managedConfigStatus = "pending";
    return;
  }
  configuration->DUCO_USER = managedSettings.duco_user;
  configuration->MINER_KEY = managedSettings.miner_key;
  configuration->RIG_IDENTIFIER = managedSettings.rig_identifier;
  if (managedSettings.rig_identifier[0]) RIG_IDENTIFIER = managedSettings.rig_identifier;
  managedConfigStatus = managedSettings.pending ? "pending" : (managedSettings.rollback ? "rollback" : "applied");
}

const char *managedWifiSsid() {
  return managedSettings.magic == MANAGED_MAGIC && managedSettings.ssid[0] ? managedSettings.ssid : SSID;
}

const char *managedWifiPassword() {
  return managedSettings.magic == MANAGED_MAGIC && managedSettings.ssid[0] ? managedSettings.wifi_password : PASSWORD;
}

bool managedWifiPending() { return managedSettings.magic == MANAGED_MAGIC && managedSettings.pending; }

void managedConfirmConnection() {
#if !defined(ESP8266)
  esp_ota_mark_app_valid_cancel_rollback();
#endif
  if (managedWifiPending()) {
    managedSettings.pending = 0;
    managedSettings.previous_ssid[0] = 0;
    managedSettings.previous_wifi_password[0] = 0;
    managedConfigStatus = "applied";
    managedSave();
  } else if (managedSettings.rollback) {
    managedSettings.rollback = 0;
    managedSave();
  }
}

void managedRollbackAndRestart() {
  if (!managedWifiPending()) return;
  if (managedSettings.previous_ssid[0]) {
    strlcpy(managedSettings.ssid, managedSettings.previous_ssid, sizeof(managedSettings.ssid));
    strlcpy(managedSettings.wifi_password, managedSettings.previous_wifi_password, sizeof(managedSettings.wifi_password));
  } else {
    managedSettings.ssid[0] = 0;
    managedSettings.wifi_password[0] = 0;
  }
  managedSettings.pending = 0;
  managedSettings.rollback = 1;
  managedConfigStatus = "rollback";
  managedSave();
  ESP.restart();
}

void managedCopy(JsonVariantConst source, const char *name, char *target, size_t size) {
  const char *value = source[name] | "";
  strlcpy(target, value, size);
}

bool managedApplyConfig(JsonObjectConst config) {
  const uint32_t version = config["version"] | 0;
  if (!version || version == managedSettings.version) return false;
  if (managedSettings.magic == MANAGED_MAGIC && managedSettings.ssid[0]) {
    strlcpy(managedSettings.previous_ssid, managedSettings.ssid, sizeof(managedSettings.previous_ssid));
    strlcpy(managedSettings.previous_wifi_password, managedSettings.wifi_password, sizeof(managedSettings.previous_wifi_password));
  } else {
    strlcpy(managedSettings.previous_ssid, SSID, sizeof(managedSettings.previous_ssid));
    strlcpy(managedSettings.previous_wifi_password, PASSWORD, sizeof(managedSettings.previous_wifi_password));
  }
  managedCopy(config, "ssid", managedSettings.ssid, sizeof(managedSettings.ssid));
  managedCopy(config, "wifi_password", managedSettings.wifi_password, sizeof(managedSettings.wifi_password));
  managedCopy(config, "duco_user", managedSettings.duco_user, sizeof(managedSettings.duco_user));
  managedCopy(config, "miner_key", managedSettings.miner_key, sizeof(managedSettings.miner_key));
  managedCopy(config, "rig_identifier", managedSettings.rig_identifier, sizeof(managedSettings.rig_identifier));
  managedSettings.version = version;
  managedSettings.pending = 1;
  managedSettings.rollback = 0;
  managedConfigStatus = "pending";
  managedSave();
  return true;
}

String managedHex(const uint8_t *bytes, size_t length) {
  static const char alphabet[] = "0123456789abcdef";
  String output;
  output.reserve(length * 2);
  for (size_t i = 0; i < length; ++i) { output += alphabet[bytes[i] >> 4]; output += alphabet[bytes[i] & 15]; }
  return output;
}

String managedHmac(const String &message) {
  const char *key = DEVICE_MANAGER_FIRMWARE_KEY;
  uint8_t output[32];
#if defined(ESP8266)
  br_hmac_key_context keyContext;
  br_hmac_context context;
  br_hmac_key_init(&keyContext, &br_sha256_vtable, key, strlen(key));
  br_hmac_init(&context, &keyContext, 0);
  br_hmac_update(&context, message.c_str(), message.length());
  br_hmac_out(&context, output);
#else
  mbedtls_md_context_t context;
  mbedtls_md_init(&context);
  mbedtls_md_setup(&context, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_md_hmac_starts(&context, reinterpret_cast<const unsigned char *>(key), strlen(key));
  mbedtls_md_hmac_update(&context, reinterpret_cast<const unsigned char *>(message.c_str()), message.length());
  mbedtls_md_hmac_finish(&context, output);
  mbedtls_md_free(&context);
#endif
  return managedHex(output, sizeof(output));
}

String managedAbsoluteUrl(const String &path) {
  String base = DEVICE_MANAGER_URL;
  int marker = base.indexOf("/v1/");
  if (marker >= 0) base.remove(marker);
  return path.startsWith("http") ? path : base + path;
}

bool managedInstallFirmware(JsonObjectConst firmware) {
  const String board = firmware["board"] | "";
  const String version = firmware["version"] | "";
  const String expectedSha = firmware["sha256"] | "";
  const String signature = firmware["signature"] | "";
  const String url = managedAbsoluteUrl(firmware["url"] | "");
  const size_t size = firmware["size"] | 0;
  const String manifest = board + "\n" + version + "\n" + expectedSha + "\n" + String(size);
  if (board != DEVICE_MANAGER_BOARD || strlen(DEVICE_MANAGER_FIRMWARE_KEY) < 32 || managedHmac(manifest) != signature) {
    managedOtaStatus = "failed"; managedOtaError = "manifest verification failed"; return false;
  }
  if (!size || size > ESP.getFreeSketchSpace()) {
    managedOtaStatus = "failed"; managedOtaError = "insufficient OTA space"; return false;
  }
#if defined(DEVICE_MANAGER_TLS) || defined(DEVICE_MANAGER_HTTPS)
  #if defined(ESP8266)
  BearSSL::WiFiClientSecure client;
  BearSSL::X509List serverCa(DEVICE_MANAGER_SERVER_CA);
  client.setTrustAnchors(&serverCa);
  #if defined(DEVICE_MANAGER_TLS)
  BearSSL::X509List clientCert(DEVICE_MANAGER_CLIENT_CERT);
  BearSSL::PrivateKey clientKey(DEVICE_MANAGER_CLIENT_KEY);
  client.setClientECCert(&clientCert, &clientKey, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, BR_KEYTYPE_EC);
  #endif
  client.setBufferSizes(1024, 1024);
  #else
  WiFiClientSecure client;
  client.setCACert(DEVICE_MANAGER_SERVER_CA);
  #if defined(DEVICE_MANAGER_TLS)
  client.setCertificate(DEVICE_MANAGER_CLIENT_CERT);
  client.setPrivateKey(DEVICE_MANAGER_CLIENT_KEY);
  #endif
  #endif
#else
  WiFiClient client;
#endif
  HTTPClient http;
  http.setTimeout(10000);
  if (!http.begin(client, url)) { managedOtaStatus = "failed"; managedOtaError = "download setup failed"; return false; }
  http.addHeader("X-Device-ID", managedDeviceId());
  http.addHeader("X-Device-Token", DEVICE_MANAGER_TOKEN);
  const int code = http.GET();
  if (code != HTTP_CODE_OK || static_cast<size_t>(http.getSize()) != size || !Update.begin(size)) {
    managedOtaStatus = "failed"; managedOtaError = "firmware download rejected"; http.end(); return false;
  }
  managedOtaStatus = "downloading";
  WiFiClient *stream = http.getStreamPtr();
  uint8_t buffer[1024];
  size_t written = 0;
#if defined(ESP8266)
  br_sha256_context sha;
  br_sha256_init(&sha);
#else
  mbedtls_sha256_context sha;
  mbedtls_sha256_init(&sha);
  mbedtls_sha256_starts_ret(&sha, 0);
#endif
  while (written < size) {
    const size_t available = stream->available();
    if (!available) { if (!http.connected()) break; delay(1); yield(); continue; }
    const size_t count = stream->readBytes(buffer, min(sizeof(buffer), min(available, size - written)));
    if (!count || Update.write(buffer, count) != count) break;
#if defined(ESP8266)
    br_sha256_update(&sha, buffer, count);
#else
    mbedtls_sha256_update_ret(&sha, buffer, count);
#endif
    written += count;
    managedOtaProgress = static_cast<int>((written * 100) / size);
    yield();
  }
  uint8_t digest[32];
#if defined(ESP8266)
  br_sha256_out(&sha, digest);
#else
  mbedtls_sha256_finish_ret(&sha, digest);
  mbedtls_sha256_free(&sha);
#endif
  http.end();
  if (written != size || managedHex(digest, sizeof(digest)) != expectedSha) {
    Update.end(false); managedOtaStatus = "failed"; managedOtaError = "firmware SHA-256 mismatch"; return false;
  }
  managedOtaStatus = "installing";
  if (!Update.end(true)) { managedOtaStatus = "failed"; managedOtaError = "firmware install failed"; return false; }
  managedOtaStatus = "restarting";
  delay(200);
  ESP.restart();
  return true;
}

void managedHandleResponse(const String &body) {
  DynamicJsonDocument document(1536);
  if (deserializeJson(document, body)) return;
  JsonObjectConst management = document["management"];
  if (management.isNull()) return;
  JsonObjectConst config = management["config"];
  if (!config.isNull() && managedApplyConfig(config)) { delay(200); ESP.restart(); return; }
  JsonObjectConst firmware = management["firmware"];
  if (!firmware.isNull()) managedInstallFirmware(firmware);
}

uint32_t managedConfigVersion() { return managedSettings.magic == MANAGED_MAGIC ? managedSettings.version : 0; }

#else
inline void managedBegin(MiningConfig *) {}
inline const char *managedWifiSsid() { return SSID; }
inline const char *managedWifiPassword() { return PASSWORD; }
inline bool managedWifiPending() { return false; }
inline void managedRollbackAndRestart() {}
#endif

#endif
