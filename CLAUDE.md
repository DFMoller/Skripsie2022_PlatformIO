# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a PlatformIO-based Arduino project developed as a final year project (Skripsie) at Stellenbosch University in 2022. The system implements a real-time household power monitoring device that measures AC current, calculates energy consumption, and transmits data to a cloud server for analysis.

**Project Goal**: Build a device that can be installed on a home's electricity distribution board to measure power consumption over 1-2 weeks, then generate recommendations for appropriate solar system sizing to take the house off-grid, including cost estimates.

**Target Hardware**: Arduino Nano 33 IoT (SAMD21 ARM Cortex-M0+ processor)

**Project Status**: Development completed - this was a university project and is no longer actively maintained.

## Build and Upload Commands

```bash
# Build the project
pio run

# Build and upload to connected device
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor

# Build, upload, and monitor in one command
pio run --target upload && pio device monitor

# Clean build files
pio run --target clean
```

## Key Configuration

- **Sample Rate**: 2000 Hz (configurable via `sampleRate` variable in main.cpp:31)
- **Serial Baud Rate**: 115200
- **Data Posting Interval**: Every 30 minutes to Flask server
- **Board**: Arduino Nano 33 IoT (atmelsam platform)

## Critical Setup Requirements

### secrets.h File

The project requires `include/secrets.h` (git-ignored) containing:
```cpp
#define API_KEY "your_api_key_here"
```

This file must be created manually before building. The API key is used for authentication when posting data to the Flask server at pythonanywhere.com.

## Architecture Overview

### High-Level Data Flow

1. **Timer5 Interrupt (2 kHz)**: Hardware timer TC5 triggers `Timer5_Handler()` every 500µs
2. **ADC Sampling**: Reads analog pin A7 for current measurement via current transformer
3. **Signal Processing**: Calculates RMS current and frequency using running sum-of-squares
4. **Buffering**: Stores PRMS (power) values in 2000-element buffer
5. **Network Communication**: Every 30 minutes, posts usage and peak data to Flask server
6. **Blynk Integration**: Real-time dashboard updates via Blynk IoT platform for monitoring
7. **Data Collection**: Flask server accumulates 1-2 weeks of data for solar sizing analysis

### Core Components (lib/ directory)

**Measurement System**:
- `measure/`: ADC sampling, RMS calculation, frequency detection
  - Implements zero-crossing detection for frequency measurement
  - Filters high-frequency noise in low-signal conditions
  - Converts ADC readings to current: `(AdcVal * 3300/1023 - 1650)/33.0 * 100/50`
  - Calculates power assuming 230V AC: `P = I_RMS * 230`

**Communication**:
- `comm/`: HTTP POST requests to Flask server, datetime formatting
  - Posts usage (Wh) and peak power (W) measurements every 30 minutes
  - Implements backlog system for offline operation (saves to SD card)
- `BlynkEdgent/`: Blynk IoT platform integration for real-time monitoring
- `BlynkEvent/`: Event notification system for Blynk

**Timing**:
- `Timer5/`: Custom hardware timer library for SAMD21 (modified from original Timer5 library)
  - Critical: Uses TC5 peripheral for precise interrupt timing
  - Modified to reduce truncation error in reload value calculation (v0.1.22)
- `timing/`: Legacy timer configuration functions (deprecated in favor of Timer5 class)

**Storage & State**:
- `System/`: System state tracking (datetime updates)
- `ConfigStore/`: Flash storage for WiFi credentials
- `Settings/`: Board configuration (LED pins, button pins)

**Utilities**:
- `stdout/`: Serial output helper functions
- `Indicator/`: LED status indicator
- `ResetButton/`: Configuration mode button handler

## Measurement Algorithm Details

Located in `lib/measure/measure.cpp`:

1. **ADC Configuration**: Prescaler reduced to DIV32 for faster sampling (line 65)
2. **Current Calculation**: 3-point moving average to reduce noise (line 48)
3. **Zero-Crossing Detection**: Counts midpoint crossings for frequency (lines 49-51)
4. **Noise Filtering**: Rejects measurements if frequency > 9% of sample rate (line 28)
5. **RMS Calculation**: Sum of squares over 1-second window (lines 21-45)

## Debug Features

Debug output is controlled by the `debug` flag (main.cpp:42):
- Pin 17: Toggles HIGH during entire IRQ handler execution
- Pin 18: Toggles HIGH during 1-second RMS calculation
- Used for measuring interrupt timing with oscilloscope

## Data Posting Flow

1. Every 30 minutes (on minute() % 30 == 0):
   - `preparePostData()`: Calculates usage (Wh) from buffered PRMS values
   - `postData()`: Sends JSON to Flask server at 21593698.pythonanywhere.com/postData
   - `postBacklog()`: Attempts to send failed posts from SD card backlog
   - `updateSystemDateTime()`: Syncs time from worldtimeapi.org

2. JSON format sent to server:
```json
{
  "datetime": "YYYY-MM-DD HH:MM",
  "api_key": "<from secrets.h>",
  "usage": <energy in Wh>,
  "peak": <max power in W>
}
```

The Flask server accumulates this data over 1-2 weeks to generate solar system sizing recommendations.

## Important Implementation Notes

- **Interrupt Safety**: The `readCurrent()` function runs in interrupt context - keep it fast
- **Buffer Overflow Protection**: PRMS buffer is limited to 2000 elements (measure.h:8)
- **Blynk Event Limiting**: Tracks daily event count to avoid quota limits (main.cpp:177-179)
- **Serial Timeout**: Waits max 15 seconds for Serial in setup() (main.cpp:148)
- **SD Card Backlog**: Failed posts are saved to SD card and retried later for reliability
- **Current Transformer**: Expects 50A/50mA CT connected to A7 with burden resistor

## Version History Context

Recent commits show focus on:
- v0.1.24: Debug pins moved to outer IRQ handler for total time measurement
- v0.1.22: Timer5 modifications to mitigate truncation error
- v0.1.20-21: Sampling rate tuning (1kHz → 2kHz) and debug pin configuration
- Focus on improving low-signal accuracy and interrupt timing precision

## Development Workflow

When modifying timing-critical code:
1. Test interrupt handler duration using debug pins 17/18
2. Verify sample rate accuracy via frequency measurements
3. Check RMS calculations with known calibration loads
4. Monitor Blynk dashboard for real-time verification

When adding new features:
1. Consider interrupt context constraints for Timer5_Handler
2. Update Blynk virtual pin mappings if needed (V0-V5 currently used)
3. Test SD card failure scenarios (backlog system)
4. Verify network error handling and retry logic
