# Building Flora Firmware

Flora firmware targets the **ESP8285** (1 MB integrated flash). PlatformIO uses `board = esp8285`, which is the correct setting for this PCB even though the project name references ESP8266.

## Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or VS Code / Cursor extension)

## Clock variants

Select the VFD tube layout via the PlatformIO environment:

| Environment | Clock variant |
|-------------|---------------|
| `iv6` | IV-6 (6-digit) |
| `iv6_v2` | IV-6 V2 thin (default) |
| `iv12` | IV-12 thin |
| `iv22` | IV-22 (4-digit) |

## Build

```bash
# Default variant (iv6_v2)
pio run

# Specific variant
pio run -e iv22
```

Build all variants:

```bash
pio run -e iv6 -e iv6_v2 -e iv12 -e iv22
```

## Flash

The ESP8285 has 1 MB flash. Firmware uses the `eagle.flash.1m256.ld` layout (~743 KB app + 256 KB LittleFS). Do not use `eagle.flash.1m.ld` — that layout reserves no filesystem space and config will not persist.

Connect the board via USB (CH340 auto-reset) and upload:

```bash
pio run -e iv6_v2 -t upload
```

Flora uses **NodeMCU-style** DTR/RTS auto-reset (not the ESP-01 `ck` method). This is configured in `platformio.ini` as `upload_resetmethod = nodemcu`. If upload still fails:

1. Close the serial monitor or any other program using the COM port (Arduino IDE, PuTTY, etc.)
2. Specify the port explicitly: `pio run -e iv6_v2 -t upload --upload-port COM6`
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

## Hardware notes

- VFD shift registers use HSPI on GPIO 13 (MOSI) and GPIO 14 (SCK), with latch on GPIO 15.
- Colon LEDs use NeoPixelBus `NeoWs2813Method` — data must be on GPIO 2 or GPIO 3 per the Flora PCB layout.
