# Solar Sizing Power Monitor - Skripsie 2022

Arduino-based household power monitoring system developed as a final year project at Stellenbosch University.

## Project Overview

This device was designed to be installed on a home's electricity distribution board to measure real-time power consumption. After collecting data for 1-2 weeks, the system would generate recommendations for appropriate solar system sizing to take the house off-grid, including cost estimates.

**Status**: This project is no longer under active development. It was completed as a university final year project (Skripsie) in 2022.

## Hardware

- **Microcontroller**: Arduino Nano 33 IoT (SAMD21)
- **Sensor**: 50A/50mA current transformer on analog pin A7
- **Connectivity**: WiFi (WiFiNINA), SD card storage
- **Sample Rate**: 2000 Hz for AC current measurement

## Features

- Real-time AC current and power measurement
- RMS calculation and frequency detection
- Local SD card storage with offline backlog capability
- Cloud data transmission every 30 minutes to Flask server
- Blynk IoT integration for live monitoring
- WiFi credential storage in flash memory
- OTA (Over-The-Air) firmware updates

## Key Measurements

- **Current**: RMS current from CT sensor
- **Power**: Calculated assuming 230V AC
- **Frequency**: Zero-crossing detection
- **Energy**: Cumulative usage in Wh over 30-minute intervals
- **Peak Power**: Maximum power recorded per interval

## Building and Flashing

Requires [PlatformIO](https://platformio.org/):

```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Open serial monitor
pio device monitor
```

## Configuration

Create `include/secrets.h` with your API key:

```cpp
#define API_KEY "your_api_key_here"
```

Update Flask server endpoint in `src/main.cpp` if needed (line 38).

## Architecture

The system uses hardware timer interrupts (TC5 at 2kHz) for precise ADC sampling, calculates RMS values over 1-second windows, buffers measurements, and periodically transmits aggregated data to a cloud server for analysis.

See `CLAUDE.md` for detailed architectural documentation.

## License

Developed as academic work at Stellenbosch University, 2022.

## Libraries Used

- Blynk IoT
- ArduinoJson
- WiFiNINA
- SAMD_TimerInterrupt (modified Timer5)
- SD card storage
- ArduinoOTA
