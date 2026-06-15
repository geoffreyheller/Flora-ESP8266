# IV-22 Flora Clock — PlatformIO Build & Flash

Step-by-step instructions for building and flashing firmware on the **IV-22 (4-digit)** Flora clock with PlatformIO.

For other tube variants and general notes, see [BUILD.md](BUILD.md).

## What you need

- Flora IV-22 clock (ESP8285, CH340 USB)
- USB cable
- Windows, macOS, or Linux
- [PlatformIO](https://platformio.org/install) — CLI or the VS Code / Cursor extension

## 1. Get the source

```bash
git clone https://github.com/mcer12/Flora-ESP8266.git
cd Flora-ESP8266
```

## 2. Use the correct environment

The IV-22 **must** be built with the `iv22` environment. Other environments (`iv6`, `iv6_v2`, `iv12`) target different tube layouts and pin maps — flashing the wrong one will not drive the display correctly.

| Environment | Clock |
|-------------|-------|
| **`iv22`** | **IV-22 (4-digit)** ← use this |
| `iv6` | IV-6 (6-digit) |
| `iv6_v2` | IV-6 V2 thin (6-digit) |
| `iv12` | IV-12 thin (6-digit) |

## 3. Build

```bash
pio run -e iv22
```

A successful build ends with `SUCCESS` and writes the firmware to `.pio/build/iv22/firmware.bin`.

## 4. Connect the clock

1. Plug the clock into USB.
2. Note the serial port:
   - **Windows:** Device Manager → Ports (COM & LPT) → e.g. `COM6`
   - **macOS:** `/dev/cu.usbserial-*` or `/dev/cu.wchusbserial*`
   - **Linux:** `/dev/ttyUSB0` or similar

Flora uses a **CH340** with **NodeMCU-style auto-reset** (already set in `platformio.ini` as `upload_resetmethod = nodemcu`).

## 5. Flash

**Close the serial monitor** (and any other program using the COM port) before uploading.

```bash
pio run -e iv22 -t upload
```

If PlatformIO does not pick the right port:

```bash
pio run -e iv22 -t upload --upload-port COM6
```

Replace `COM6` with your port.

### Upload fails?

1. Close serial monitor, Arduino IDE, PuTTY, etc.
2. Retry with an explicit port (command above).
3. Manual bootloader entry: hold **FLASH**, tap **RESET**, release **RESET**, then release **FLASH**, and upload immediately.

## 6. Serial monitor

Open the monitor at **115200 baud**:

```bash
pio device monitor
```

Or in Cursor/VS Code: PlatformIO → Monitor.

### Healthy boot (example)

After Wi-Fi and NTP are configured, serial output should look like:

```
[BOOT] Clock variant: IV-22 (4-digit)
[CONF] LittleFS total=262144 used=12288 free=249856
[WIFI] Connecting to: YourNetwork
[WIFI] Successfully connected to: YourNetwork
[WIFI] Mac address: ...
[WIFI] IP address: 192.168.x.x
[NTP] Sync success! Received NTP timestamp: ...
[TIME] Synced 2026-06-15 06:28:22
[DISP] 6:28 -> digits 0628
```

- **`[BOOT] Clock variant: IV-22 (4-digit)`** confirms the correct firmware variant.
- **`[DISP] ...`** confirms the software time matches what should appear on the tubes.

## 7. First-time Wi-Fi setup

If no credentials are saved, the clock starts a captive portal:

1. Connect your phone or PC to the Wi-Fi network **`FLORA_XXXXXX`** (suffix from the clock MAC).
2. Open a browser; the config page should appear automatically.
3. Enter Wi-Fi SSID, password, timezone, and NTP server (optional).
4. Save — the clock reboots and joins your network.

Settings are stored in **LittleFS** at `/config.json` on the 256 KB filesystem partition.

## 8. OTA updates (optional)

With Wi-Fi connected, browse to:

```
http://<clock-ip>/update
```

Default credentials: `flora` / `flora`

## Libraries

PlatformIO pulls dependencies from `platformio.ini` (`lib_deps`) and from the **ESP8266 Arduino framework** (`platform = espressif8266`). The firmware also includes one local driver in `src/`.

### External libraries (`lib_deps`)

| Library | Version (pinned) | Purpose |
|---------|------------------|---------|
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | ^7.2.0 | Read/write `/config.json` on LittleFS (Wi-Fi credentials, brightness, timezone, NTP server, etc.) |
| [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) | ^2.7.9 | Drive WS2812 colon LEDs (`NeoWs2813Method` on GPIO 2); color animations and status indicators (connecting, NTP sync, errors) |
| [Time](https://github.com/PaulStoffregen/Time) | ^1.6.1 | `TimeLib` — system time (`hour()`, `minute()`, `second()`), sync interval, and NTP time provider hook |
| [Timezone](https://github.com/JChristensen/Timezone) | ^1.2.4 | Convert UTC from NTP to local time with daylight saving rules (US/EU rules configurable via portal) |
| [ESP8266TimerInterrupt](https://github.com/khoih-prog/ESP8266TimerInterrupt) | ^1.6.0 | Hardware timer ISR at 200 µs for flicker-free VFD PWM refresh (`TimerHandler`) |

### ESP8266 Arduino framework libraries

These ship with the `espressif8266` platform and are linked automatically — no separate `lib_deps` entry.

| Library | Purpose |
|---------|---------|
| **Arduino core** (`Arduino.h`) | `setup()` / `loop()`, GPIO, `Serial`, timing (`millis`, `delay`), watchdog |
| **ESP8266WiFi** | Station mode Wi-Fi connect, hostname, static IP config, MAC address |
| **WiFiClient** | TCP client support used by the web stack |
| **WiFiUdp** | UDP socket for NTP requests (port 8888 local) |
| **DNSServer** | Captive portal DNS redirect in config mode (`FLORA_XXXXXX` AP) |
| **ESP8266WebServer** | HTTP server for config portal, diyHue API, and status pages |
| **ESP8266HTTPUpdateServer** | Over-the-air firmware upload at `/update` |
| **ESP8266mDNS** | mDNS responder (`flora.local`) — optional, currently commented out in firmware |
| **LittleFS** | On-flash filesystem for persistent `config.json` (256 KB partition) |
| **FS** | Base filesystem API used with LittleFS |
| **Ticker** | Periodic callbacks for segment crossfade and colon color timers (non-ISR) |

### Local firmware module (not a PlatformIO library)

| Module | Location | Purpose |
|--------|----------|---------|
| **shift_register** | `src/shift_register.cpp` | IRAM-safe HSPI bit-bang to VFD shift registers (GPIO 13/14/15); must run from ISR without calling flash-resident `SPI.transfer()` |

### Transitive / build-only dependencies

NeoPixelBus may pull in **I2S** from the framework for some LED methods; Flora uses `NeoWs2813Method` (GPIO bit-bang), so I2S is not used at runtime. PlatformIO resolves these automatically during the build.

## Quick reference

| Task | Command |
|------|---------|
| Build IV-22 | `pio run -e iv22` |
| Flash IV-22 | `pio run -e iv22 -t upload` |
| Flash (specific port) | `pio run -e iv22 -t upload --upload-port COM6` |
| Serial monitor | `pio device monitor` |
| List ports | `pio device list` |

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| All digits show **8** | Wrong `iv22` environment, or HSPI/display init issue | Rebuild and flash with `-e iv22`; check `[BOOT]` line |
| Reboot loop / `Exception (0)` during Wi-Fi | Display ISR calling flash-resident SPI code | Use current firmware (IRAM-safe shift driver in `src/shift_register.cpp`) |
| Config lost after reboot | Wrong flash layout | `platformio.ini` must use `board_build.ldscript = eagle.flash.1m256.ld` |
| Upload: “Access is denied” on COM port | Serial monitor still open | Close monitor, then upload |
| NTP fails, then succeeds | Custom NTP host unreachable | Firmware retries 3× then falls back to `pool.ntp.org` |
| `[BOOT]` shows wrong variant | Wrong environment flashed | `pio run -e iv22 -t upload` |

## Hardware notes (IV-22)

- MCU: **ESP8285** (1 MB flash, configured as `board = esp8285`)
- VFD shift registers: HSPI **GPIO 13** (data), **GPIO 14** (clock), **GPIO 15** (latch)
- Colon LEDs: WS2812 on **GPIO 2** (NeoPixelBus `NeoWs2813Method`)
- Upload speed: 115200 baud (`upload_speed = 115200` in `platformio.ini`)
