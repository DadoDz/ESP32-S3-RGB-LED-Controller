# ESP32-S3 RGB LED Controller

ESP-IDF firmware that drives a WS2812 RGB LED on an ESP32-S3, controlled remotely via a Firebase Realtime Database node.

```
Firebase Realtime Database  <-- polled every 5s -->  ESP32-S3  --->  WS2812 LED
        (/led node)
```

Write `power` / `red` / `green` / `blue` / `brightness` to `/led` in your Firebase Realtime Database (from a companion app, curl, the Firebase console, whatever) and the board will pick it up and update the LED.

## Features

- 🌈 Full RGB color + brightness control (0-100%)
- 🔌 Power on/off
- 📶 WiFi station mode with auto-reconnect
- 🔒 TLS via the ESP-IDF cert bundle — no hardcoded certs
- ⚙️ WiFi/Firebase settings configured through `menuconfig`, not hardcoded in source

## Hardware

- ESP32-S3 dev board
- WS2812 (NeoPixel) LED — GPIO 48 by default, matches the onboard LED on most ESP32-S3 devkits (change `LED_GPIO` in `menuconfig` or `config/project_config.h` if yours is wired differently)

## Project structure

```
main/
├── main.c                     app entry point, FreeRTOS tasks
├── config/project_config.h    pins, timing, task config
├── wifi/                      WiFi station mode + reconnect
├── firebase/                  HTTPS REST client + JSON parsing
├── led/                       WS2812 driver (RMT peripheral)
├── Kconfig.projbuild          menuconfig options (WiFi/Firebase settings)
└── CMakeLists.txt
```

## Setup

### 1. Firebase

1. Create a Firebase project and enable the **Realtime Database**.
2. Grab your database URL, e.g. `https://your-project-default-rtdb.firebaseio.com`.
3. The full path the board reads from is `<your-db-url>/led.json`.
4. This project doesn't authenticate — if you make the repo/database public, lock down your database rules before relying on it for anything real.

Expected shape of the `/led` node:

```json
{
  "power": true,
  "red": 255,
  "green": 0,
  "blue": 0,
  "brightness": 100
}
```

### 2. Build & flash

Requires [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)

```bash
idf.py set-target esp32s3
idf.py menuconfig
```

Under **"LED Controller Configuration"**, set:
- WiFi SSID
- WiFi Password
- Firebase Realtime Database URL (with `/led.json`)

Then:

```bash
idf.py build flash monitor
```

Your WiFi password and DB URL end up in `sdkconfig`, which is gitignored, so they never get committed.
