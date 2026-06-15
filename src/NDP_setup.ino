
void ndp_setup() {
  if (json["dst_enable"].as<int>() == 1 && json["std_month"].as<int>() >= 1) {
    TimeChangeRule EDT2 = {"EDT", (int8_t)json["dst_week"].as<int>(), (int8_t)json["dst_day"].as<int>(),
                         (int8_t)json["dst_month"].as<int>(), (int8_t)json["dst_hour"].as<int>(), (int8_t)json["dst_offset"].as<int>()};
    TimeChangeRule EST2 = {"EST", (int8_t)json["std_week"].as<int>(), (int8_t)json["std_day"].as<int>(),
                         (int8_t)json["std_month"].as<int>(), (int8_t)json["std_hour"].as<int>(), (int8_t)json["std_offset"].as<int>()};
    TZ.setRules(EDT2, EST2);
  }
  // else keep default TZ rules from FLORA_FIRMWARE.ino (EDT/EST)

  Udp.begin(localPort);
  setSyncProvider(getNtpLocalTime);
  setSyncInterval(3600);

  Serial.printf("[TIME] Synced %04d-%02d-%02d %02d:%02d:%02d\n",
                year(), month(), day(), hour(), minute(), second());
}
