# Building Flora Firmware

Flora firmware targets the **ESP8285** (1 MB integrated flash). PlatformIO uses `board = esp8285`, which is the correct setting for this PCB even though the project name references ESP8266.

Requires **espressif8266 platform 4.x** (Arduino core 3.1+, GCC 10+) for NeoPixelBus 2.7+.

## Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or VS Code / Cursor extension)

## Clock variants

Select the VFD tube layout via the PlatformIO environment:

| Environment | Clock variant |
|-------------|---------------|
| `iv6` | IV-6 (6-digit) |
| `iv6_v2` | IV-6 V2 thin |
| `iv12` | IV-12 thin |
| `iv22` | IV-22 (4-digit, default) |

## Build

```bash
# Default variant (iv22)
pio run

# Specific variant
pio run -e iv6_v2
```

Build all variants:

```bash
pio run -e iv6 -e iv6_v2 -e iv12 -e iv22
```

## Flash

The ESP8285 has 1 MB flash. Firmware uses the `eagle.flash.1m256.ld` layout (~743 KB app + 256 KB LittleFS). Do not use `eagle.flash.1m.ld` — that layout reserves no filesystem space and config will not persist.

Connect the board via USB (CH340 auto-reset) and upload:

```bash
pio run -t upload
```

Flora uses **NodeMCU-style** DTR/RTS auto-reset (not the ESP-01 `ck` method). This is configured in `platformio.ini` as `upload_resetmethod = nodemcu`. If upload still fails:

1. Close the serial monitor or any other program using the COM port (Arduino IDE, PuTTY, etc.)
2. Specify the port explicitly: `pio run -t upload --upload-port COM6`
3. Manual bootloader entry: hold **FLASH**, tap **RESET**, release **RESET**, then release **FLASH**, and upload immediately

Serial monitor at 115200 baud:

```bash
pio device monitor
```

## Configuration storage

Runtime settings are stored in **LittleFS** at `/config.json`. You do **not** need to run `pio run -t uploadfs` — LittleFS is formatted automatically on first use when the partition is empty.

Devices upgrading from v5.1.x (SPIFFS) automatically migrate their config on first boot after OTA.

If you need to wipe settings manually:

```bash
pio device monitor
```

Then press reset while watching for `[CONF]` messages. To factory-reset the filesystem from code, use the ESP8266 `LittleFS.format()` API (not normally required).

## OTA update

With Wi-Fi connected, browse to `http://<clock-ip>/update` (default credentials: `flora` / `flora`).

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

## Hardware notes

- VFD shift registers use HSPI on GPIO 13 (MOSI) and GPIO 14 (SCK), with latch on GPIO 15.
- Colon LEDs use NeoPixelBus `NeoWs2813Method` — data must be on GPIO 2 or GPIO 3 per the Flora PCB layout.
