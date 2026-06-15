
void ndp_setup() {
  const int16_t stdOffset = json["std_offset"].as<int16_t>();
  const bool dstEnabled = json["dst_enable"].as<int>() == 1;

  if (dstEnabled) {
    ensureTimezoneConfigDefaults();

    const int16_t dstOffset = json["dst_offset"].as<int16_t>();
    TimeChangeRule dstRule = {"DST", (int8_t)json["dst_week"].as<int>(), (int8_t)json["dst_day"].as<int>(),
                              (int8_t)json["dst_month"].as<int>(), (int8_t)json["dst_hour"].as<int>(), dstOffset};
    TimeChangeRule stdRule = {"STD", (int8_t)json["std_week"].as<int>(), (int8_t)json["std_day"].as<int>(),
                              (int8_t)json["std_month"].as<int>(), (int8_t)json["std_hour"].as<int>(), stdOffset};
    TZ.setRules(dstRule, stdRule);

    Serial.printf("[TIME] DST starts: week=%d dow=%d month=%d hour=%d offset=%d min\n",
                  dstRule.week, dstRule.dow, dstRule.month, dstRule.hour, dstRule.offset);
    Serial.printf("[TIME] STD starts: week=%d dow=%d month=%d hour=%d offset=%d min\n",
                  stdRule.week, stdRule.dow, stdRule.month, stdRule.hour, stdRule.offset);
  } else {
    TimeChangeRule fixed = {"UTC", First, Sun, Jan, 0, stdOffset};
    TZ.setRules(fixed, fixed);
  }

  Udp.begin(localPort);
  setSyncProvider(getNtpLocalTime);
  setSyncInterval(3600);

  Serial.printf("[TIME] Offset %d min, DST %s\n", stdOffset, dstEnabled ? "on" : "off");
  Serial.printf("[TIME] Synced %04d-%02d-%02d %02d:%02d:%02d\n",
                year(), month(), day(), hour(), minute(), second());
}
