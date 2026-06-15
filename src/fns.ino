#include <FS.h>
#include <LittleFS.h>

static void logFilesystemInfo() {
  FSInfo info;
  if (LittleFS.info(info)) {
    Serial.printf("[CONF] LittleFS total=%u used=%u free=%u\n",
                  info.totalBytes, info.usedBytes, info.totalBytes - info.usedBytes);
  }
}

bool initFilesystem() {
  if (!LittleFS.begin()) {
    Serial.println("[CONF] Failed to mount LittleFS");
    return false;
  }

  if (LittleFS.exists("/config.json")) {
    logFilesystemInfo();
    return true;
  }

  // One-time migration from legacy SPIFFS. Never call SPIFFS.begin() with
  // auto-format after LittleFS has been used — it would wipe the partition.
  String spiffsConfig;
  bool hadSpiffsConfig = false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  SPIFFSConfig spiffsCfg(false);
  SPIFFS.setConfig(spiffsCfg);
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      File src = SPIFFS.open("/config.json", "r");
      if (src) {
        spiffsConfig = src.readString();
        hadSpiffsConfig = spiffsConfig.length() > 0;
        src.close();
      }
    }
    SPIFFS.end();
  }
#pragma GCC diagnostic pop

  if (hadSpiffsConfig) {
    File dst = LittleFS.open("/config.json", "w");
    if (dst) {
      dst.print(spiffsConfig);
      dst.close();
      Serial.println("[CONF] Migrated config from SPIFFS to LittleFS");
    } else {
      Serial.println("[CONF] Failed to write migrated config");
    }
  } else {
    Serial.println("[CONF] No config found, starting with defaults");
  }

  logFilesystemInfo();
  return true;
}

// EU-style defaults: summer time last Sun Mar, standard time last Sun Oct.
// Used when DST is enabled but transition dates were never configured (both
// months defaulting to January breaks the Timezone library).
void ensureTimezoneConfigDefaults() {
  if (json["dst_enable"].as<int>() != 1) return;

  const int dstMonth = json["dst_month"].as<int>();
  const int stdMonth = json["std_month"].as<int>();
  bool appliedDefaults = false;
  if (dstMonth < 1 || stdMonth < 1 || dstMonth == stdMonth) {
    json["dst_month"] = 3;
    json["std_month"] = 10;
    json["dst_week"] = 0;
    json["std_week"] = 0;
    json["dst_day"] = 1;
    json["std_day"] = 1;
    appliedDefaults = true;
    Serial.println("[CONF] Applied default DST rules (last Sun Mar / last Sun Oct)");
  }

  if (json["dst_week"].isNull()) json["dst_week"] = 0;
  if (json["std_week"].isNull()) json["std_week"] = 0;
  if (json["dst_day"].as<int>() < 1) json["dst_day"] = 1;
  if (json["std_day"].as<int>() < 1) json["std_day"] = 1;
  if (json["dst_hour"].isNull()) json["dst_hour"] = 1;
  if (json["std_hour"].isNull()) json["std_hour"] = 1;

  const int16_t stdOff = json["std_offset"].as<int16_t>();
  if (json["dst_offset"].isNull()) {
    json["dst_offset"] = stdOff + 60;
    appliedDefaults = true;
  }

  if (appliedDefaults) {
    saveConfig();
  }
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
    if (i < 5)
      result += ':';
  }
  result.toUpperCase();
  return result;
}

String macLastThreeSegments(const uint8_t* mac) {
  String result;
  for (int i = 3; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
  }
  result.toUpperCase();
  return result;
}

bool readConfig() {
  File stateFile = LittleFS.open("/config.json", "r");
  if (!stateFile) {
    Serial.println("[CONF] Failed to read config file... first run?");
    Serial.println("[CONF] Creating new file...");
    saveConfig();
    return false;
  }
  DeserializationError error = deserializeJson(json, stateFile.readString());
  stateFile.close();
  if (error) {
    Serial.print("[CONF] Failed to parse config: ");
    Serial.println(error.c_str());
    return false;
  }
  return true;
}

bool saveConfig() {
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("[CONF] Failed to open config file for writing");
    return false;
  }
  if (serializeJson(json, configFile) == 0) {
    Serial.println("[CONF] Failed to serialize config");
    configFile.close();
    return false;
  }
  configFile.flush();
  configFile.close();
  Serial.println("[CONF] Config saved");
  logFilesystemInfo();
  return true;
}
