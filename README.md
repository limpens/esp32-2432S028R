# ESP32-2432S028R

![Static Badge](https://img.shields.io/badge/DEVICE-ESP32--2432S028R-8A2BE2) ![Static Badge](https://img.shields.io/badge/MCU-ESP32-8A2BE2)
![Static Badge](https://img.shields.io/badge/OS-FreeRTOS-green) ![Static Badge](https://img.shields.io/badge/SDK-ESP--IDF%20v5.x-blue)
![CI](https://github.com/Rovel/esp32-2432S028R/workflows/ESP-IDF%20CI/badge.svg)

Barebones example, to demonstrate the usage of [esp_lvgl_port](https://components.espressif.com/components/espressif/esp_lvgl_port) and [lvgl](https://github.com/lvgl/lvgl/) on a Sunton 2432S028R display.

## Features

- Touch-enabled LVGL UI with multiple screens
- Keypad interface with PIN entry and NVS storage
- Touch calibration system with real-time coordinate display
- Comprehensive testing suite with CI/CD integration
- Driver abstractions for LCD and touch controller

## Quick Start

```bash
# Clone and build
git clone <repository-url>
cd esp32-2432S028R
idf.py set-target esp32
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash monitor
```

## Testing

This project includes comprehensive testing for reliable development:

### Quick Test Run
```bash
# Windows
run_tests.bat all

# Linux/Mac  
./run_tests.sh all
```

### Test Types

1. **Host Tests** - Fast unit tests that run without hardware
2. **Unity Tests** - On-device integration tests using ESP-IDF Unity framework
3. **Build Tests** - Compilation verification for multiple targets

### Continuous Integration

GitHub Actions automatically runs:
- ✅ Host-based unit tests
- ✅ ESP-IDF build verification  
- ✅ Code quality checks
- ✅ Multi-target compilation

See [test/README.md](test/README.md) for detailed testing documentation.

### See also:

* [The CYD github](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/tree/main) with a lot of information about these boards
