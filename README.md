# 🎵 Audio Playback System

An **ESP32-based audio playback and streaming system** that allows users to upload, manage, and play music stored locally on serial flash memory or streamed via Bluetooth to external speakers.

---

## 🧭 Overview

This system is designed to enable **secure audio playback** with both local and Bluetooth streaming capabilities.  
Users can connect to the device’s **web interface**, upload music files, control playback, and select output modes (local speaker or Bluetooth).

---

## ⚙️ Hardware Overview

| Component | Description |
|------------|-------------|
| **MCU** | ESP32 (dual-core, 240 MHz, integrated Wi-Fi + Bluetooth) |
| **Audio Amplifier** | MAX98357A (Class-D I2S amplifier) |
| **Flash Storage** | W25Qxx Serial Flash for local MP3 storage |
| **Speaker Output** | Direct 4 Ω / 8 Ω load |
| **Power Supply** | 5 V input with onboard 3.3 V regulation |
| **Bluetooth** | A2DP sink profile for wireless playback |

---

## 🧩 System Architecture

| Layer | Components | Description |
|--------|-------------|-------------|
| **Application Layer** | Web server, File upload manager, Playback control | Manages user interface, HTTP requests, and playback commands |
| **Audio Layer** | Audio player, Bluetooth A2DP service | Handles MP3 decoding, DAC streaming, and BT audio |
| **Storage Layer** | SPI Flash driver (W25Qxx) | Manages song storage, file indexing, and retrieval |
| **Hardware Abstraction** | ESP-IDF drivers | Provides I2S, SPI, GPIO, Wi-Fi, and BT interfaces |
| **RTOS Layer** | FreeRTOS tasks | Handles concurrent playback, networking, and system monitoring |

---

## 💡 Key Features

- 🎶 Local audio playback from **W25Qxx SPI Flash**
- 🔊 **Bluetooth audio streaming (A2DP)** to wireless speakers
- 🌐 Built-in **web server interface** for:
  - Uploading MP3 files
  - Selecting playback mode (local / Bluetooth)
  - Controlling play, pause, next, previous
- ⚙️ **Real-time task management** using FreeRTOS
- 🔐 Optional **encrypted audio file storage**
- 🔈 **Low-latency playback** via I2S interface

---

## 🧠 Software Stack

| Component | Technology |
|------------|-------------|
| **Framework** | ESP-IDF |
| **RTOS** | FreeRTOS |
| **Audio** | A2DP Sink + I2S Audio Stream |
| **Web Interface** | Async Web Server + HTML Control UI |
| **Storage** | SPIFFS / W25Qxx Flash |
| **Language** | C / C++ |

---



## 🧰 Development Environment

| Tool | Version / Description |
|------|------------------------|
| **ESP-IDF** | v5.x |
| **Compiler** | Xtensa GCC |
| **Flash Tool** | `esptool.py` |
| **IDE** | VS Code / ESP-IDF Extension |
| **Hardware Design** |  |

---

## 📸 Images

| Description | Image |
|--------------|-------|
| Web Interface | ![Web UI](assets/ui_webpage.png) |
| Prototype Board | ![Prototype](assets/device_front.jpg) |

---

