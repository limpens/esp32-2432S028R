# ESP32-2432S028R Testing Guide

This project includes comprehensive testing for the ESP32-2432S028R development board firmware.

## Test Structure

```
test/
├── unity/              # On-device Unity tests
│   ├── main/          # Test runner main
│   └── components/    # Test components
├── host_test/         # Host-based tests (CI-friendly)
│   └── mocks/        # Mock implementations
└── README.md         # This file
```

## Running Tests

### 1. Host Tests (Fast, for CI)

Host tests run on your development machine without hardware:

```bash
cd test/host_test
mkdir build && cd build
cmake ..
make
./host_tests
```

### 2. On-Device Unity Tests

Unity tests run on the actual ESP32 hardware:

```bash
cd test/unity
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Once flashed, the device will start an interactive test menu. You can:
- Press `*` to run all tests
- Press a test number to run specific tests
- View real-time test results

### 3. Main Project Build Test

Verify the main project builds correctly:

```bash
cd ..  # Back to project root
idf.py set-target esp32
idf.py build
```

## Test Categories

### Driver Tests
- **LCD Tests**: GPIO configuration, SPI bus initialization
- **Touch Tests**: Touch controller setup, coordinate validation

### Frontend Tests  
- **Keypad Tests**: PIN validation, NVS storage
- **Calibration Tests**: Touch point validation, coordinate transformation

### Integration Tests
- **NVS Storage**: Reading/writing configuration data
- **System Tests**: Memory management, task creation

## Continuous Integration

GitHub Actions automatically runs:

1. **Host Tests**: Fast validation of core logic
2. **Build Tests**: Ensures code compiles for ESP32
3. **Code Quality**: Static analysis and formatting checks
4. **Artifact Collection**: Saves build outputs for debugging

## Adding New Tests

### For Host Tests
1. Add test function to `test/host_test/test_main.c`
2. Use simple assertions: `TEST_ASSERT`, `TEST_ASSERT_EQUAL`
3. Mock any hardware dependencies

### For Unity Tests  
1. Create new test file in `test/unity/components/test_*/`
2. Use Unity macros: `TEST_CASE`, `TEST_ASSERT_EQUAL`
3. Add to CMakeLists.txt in the component

### Test Naming Convention
- File: `test_<module>.c`
- Function: `test_<functionality>()`
- Test case: `"<description>", "[tag]"`

## Debugging Failed Tests

### Host Test Failures
- Check console output for assertion details
- Use GDB: `gdb ./host_tests`
- Add printf debugging to mock implementations

### Unity Test Failures
- Monitor serial output: `idf.py monitor`
- Check ESP32 logs for memory/stack issues
- Use Unity's verbose output modes

### CI Failures
- Check GitHub Actions logs
- Download build artifacts for analysis
- Verify ESP-IDF version compatibility

## Test Coverage

Current test coverage includes:
- ✅ Basic driver functionality
- ✅ Input validation
- ✅ NVS operations
- ✅ Touch coordinate handling
- ✅ Calibration logic
- ⏳ LVGL UI components (planned)
- ⏳ Hardware integration (planned)

## Performance Benchmarks

Key performance metrics tested:
- Touch response time: < 50ms
- Screen refresh rate: 60fps target
- Memory usage: < 80% of available heap
- Boot time: < 3 seconds to UI ready

## Best Practices

1. **Test Early**: Write tests alongside feature development
2. **Mock Hardware**: Use mocks for CI to enable fast feedback
3. **Test Real Hardware**: Validate on actual device before release
4. **Document Tests**: Clear test names and comments
5. **Automate Everything**: Let CI catch regressions automatically
