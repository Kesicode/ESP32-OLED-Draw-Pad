# ESP32 OLED Draw Pad

A lightweight Wi-Fi drawing pad built with an **ESP32** and a **128×64 I2C OLED display**. Draw from a phone or computer browser and the strokes are transmitted to the ESP32 in real time using **WebSockets** and rendered on the OLED.

![ESP32 OLED Draw Pad](images/project.jpg)

## ✨ Features

- 🖊️ Draw directly from a browser
- ⚡ Real-time WebSocket communication
- 🖥️ Live preview on a 128×64 OLED
- 📏 Adjustable pen size
- 🧹 Clear the OLED from the web interface
- 📱 Works with touch and mouse/pointer input
- 📡 Supports normal Wi-Fi and an ESP32 fallback access point
- 🌐 HTTP server on port `80`
- 🔌 WebSocket server on port `81`

## 🧰 Hardware

| Component | Specification |
|---|---|
| Microcontroller | ESP32 development board |
| Display | 0.96-inch OLED, 128×64 |
| Interface | I2C |
| OLED address | `0x3C` |
| I2C SDA | GPIO 21 |
| I2C SCL | GPIO 22 |
| Power | USB |

### Prototype

![OLED and ESP32 hardware](images/oled.jpg)

## 🔌 Wiring

```text
ESP32          OLED
----------------------
3V3   -------- VCC
GND   -------- GND
GPIO21 -------- SDA
GPIO22 -------- SCL
```

> The exact power connection can vary by OLED module. Use a 3.3 V-compatible module and verify the module's pinout before powering it.

## 💻 Software

### Required libraries

Install these libraries through the Arduino IDE Library Manager:

- **Adafruit GFX Library**
- **Adafruit SSD1306**
- **WebSockets** (by Markus Sattler / Links2004)

The ESP32 Arduino core provides `WiFi.h`, `WebServer.h`, and `Wire.h`.

### Upload

1. Open `code/ESP32_OLED_Draw_Pad.ino` in Arduino IDE.
2. Select your ESP32 board.
3. Enter your Wi-Fi credentials locally:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
```

4. Upload the program.
5. Open Serial Monitor at `115200` baud.
6. Check the IP address shown by the ESP32.
7. Open that IP address in a browser.

**Do not commit your real Wi-Fi password to a public GitHub repository.**

## 📡 Fallback Access Point

If the configured Wi-Fi network cannot be reached, the ESP32 also starts a local access point:

```text
SSID: ESP-Canvas
Password: 12345678
```

Connect your phone/computer to this network and open:

```text
192.168.4.1
```

## 🖥️ How it works

```text
Phone / PC Browser
        │
        │ HTTP
        ▼
   ESP32 Web Server
        │
        │ WebSocket :81
        ▼
   Drawing Commands
        │
        ▼
  OLED Frame Buffer
        │
        ▼
   128×64 OLED Display
```

The browser converts pointer/touch movements into compact drawing commands. The ESP32 receives those commands over WebSocket and updates the OLED display.

### Command format

```text
P,x,y,size
```
Draw a point.

```text
L,x0,y0,x1,y1,size
```
Draw a line segment.

```text
CLR
```
Clear the OLED.

## 📸 Project Gallery

### Web Interface

![Web interface](images/web-interface.jpg)

### Hardware

![Project hardware](images/project.jpg)

### OLED Display

![OLED display](images/oled.jpg)

## 📁 Project Structure

```text
ESP32-OLED-Draw-Pad/
├── code/
│   └── ESP32_OLED_Draw_Pad.ino
├── images/
│   ├── project.jpg
│   ├── oled.jpg
│   └── web-interface.jpg
├── README.md
└── LICENSE   (optional)
```

## 🚀 Future Improvements

- Add an undo/redo system
- Add multiple drawing tools
- Add OLED screenshot/export
- Add persistent drawing storage
- Add configurable Wi-Fi through a captive portal
- Add OTA firmware updates
- Improve stroke smoothing

## 📄 License

This project is open source. You may modify and build upon it for learning and personal projects.
