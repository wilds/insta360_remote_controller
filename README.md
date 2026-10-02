# ESP32-C3 Insta360 BLE Remote Control

An open-source, low-power Bluetooth Low Energy (BLE) physical remote control for Insta360 cameras built using ESP32-C3 Super Mini and the lightweight `NimBLE-Arduino` library.

## Features

- **Automated Discovery & Reconnection**: Continuously scans for active Insta360 cameras and connects automatically upon power-up.
- **MAC Address Persistence**: Saves the paired camera's MAC address to non-volatile storage (`NVS`/`Preferences`) during the first connection for fast auto-pairing on subsequent startups.
- **Full Video & Shutter Control**: Supports start/stop video recording via the onboard BOOT button (GPIO 9) with hardware debouncing.
- **Status & Telemetry Decoding**: Parses incoming BLE notification packets (`BE82` characteristic) including battery level, camera recording state, storage events, and error responses.
- **Heartbeat Maintenance**: Sends periodic keep-alive packets (`1200ms`) to prevent camera idle timeout.
- **State Machine Architecture**: Non-blocking state management with clear visual feedback via built-in status LED.

---

## Hardware Pinout

| Component | ESP32-C3 Pin | Mode | Description |
| :--- | :--- | :--- | :--- |
| **Onboard BOOT Button** | `GPIO 9` | `INPUT_PULLUP` | Built-in physical button (active LOW) used as shutter trigger |
| **Onboard Status LED** | `GPIO 8` | `OUTPUT` | Built-in LED indicator (active LOW) |

---

## LED Status Indications

- **Fast Blinking (`100ms`)**: Scanning for camera / Establishing GATT connection.
- **Solid ON**: Connected, paired, and ready in idle standby mode.
- **Slow Blinking (`1000ms`)**: Active video recording in progress.

---

## Dependency Setup

This project requires the **NimBLE-Arduino** library for efficient BLE memory management and fast connection setup.

### PlatformIO (`platformio.ini`)

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
lib_deps =
    h2zero/NimBLE-Arduino@^1.4.1
```

---

## Protocol Overview

The remote communicates with the camera via custom 16-byte raw payloads on the following BLE GATT structure:

- **Service UUID**: `0000be80-0000-1000-8000-00805f9b34fb`
- **Write Characteristic**: `0000be81-0000-1000-8000-00805f9b34fb`
- **Notify Characteristic**: `0000be82-0000-1000-8000-00805f9b34fb`

### Command Reference Table

| Opcode | Name | Description |
| :---: | :--- | :--- |
| `0x01` | `PAIRING_HANDSHAKE` | Initial pairing request |
| `0x02` | `HEARTBEAT` | Periodic keep-alive ping |
| `0x03` | `TAKE_PHOTO` | Trigger single photo capture |
| `0x04` | `START_VIDEO` | Start standard video recording |
| `0x05` | `STOP_VIDEO` | Stop video recording |
| `0x0E` | `SHUTTER_BUTTON` | Hardware shutter toggle (Red Button) |
| `0x0D` | `POWER_OFF` | Remote shutdown |
| `0x1F` | `QUERY_STATUS` | Query system status ping |
| `0x21` | `QUERY_BATTERY` | Query camera battery percentage |
| `0x2B` | `MARK_HILIGHT` | Add HiLight tag during video capture |
| `0x53` | `QUERY_STORAGE` | Query SD storage & remaining capacity |

---

## Getting Started

1. Power up the ESP32-C3 Super Mini. The onboard **BOOT button (GPIO 9)** acts directly as the video shutter trigger.
2. Flash the firmware to your board.
3. Turn on your Insta360 camera and ensure Bluetooth remote discovery is enabled.