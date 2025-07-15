#!/bin/bash

# ESP32-2432S028R Test Runner Script
# Usage: ./run_tests.sh [host|unity|build|all]

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW} $1${NC}"
    echo -e "${YELLOW}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

run_host_tests() {
    print_header "Running Host Tests"
    
    cd test/host_test
    
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    cmake ..
    make
    
    if ./host_tests; then
        print_success "Host tests passed"
        return 0
    else
        print_error "Host tests failed"
        return 1
    fi
}

run_unity_tests() {
    print_header "Building Unity Tests for ESP32"
    
    cd test/unity
    
    # Check if ESP-IDF is available
    if ! command -v idf.py &> /dev/null; then
        print_error "ESP-IDF not found. Please install and source ESP-IDF first."
        return 1
    fi
    
    idf.py set-target esp32
    
    if idf.py build; then
        print_success "Unity tests built successfully"
        echo "To run on device: idf.py -p /dev/ttyUSB0 flash monitor"
        return 0
    else
        print_error "Unity tests build failed"
        return 1
    fi
}

run_main_build() {
    print_header "Building Main Project"
    
    # Check if ESP-IDF is available
    if ! command -v idf.py &> /dev/null; then
        print_error "ESP-IDF not found. Please install and source ESP-IDF first."
        return 1
    fi
    
    idf.py set-target esp32
    
    if idf.py build; then
        print_success "Main project built successfully"
        return 0
    else
        print_error "Main project build failed"
        return 1
    fi
}

cleanup() {
    print_header "Cleaning Build Artifacts"
    
    # Clean main build
    if [ -d "build" ]; then
        rm -rf build
        print_success "Cleaned main build directory"
    fi
    
    # Clean Unity test build
    if [ -d "test/unity/build" ]; then
        rm -rf test/unity/build
        print_success "Cleaned Unity test build directory"
    fi
    
    # Clean host test build
    if [ -d "test/host_test/build" ]; then
        rm -rf test/host_test/build
        print_success "Cleaned host test build directory"
    fi
}

show_help() {
    echo "ESP32-2432S028R Test Runner"
    echo ""
    echo "Usage: $0 [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  host     Run host-based tests (fast, no hardware needed)"
    echo "  unity    Build Unity tests for ESP32 device"
    echo "  build    Build main project"
    echo "  all      Run all tests and builds"
    echo "  clean    Clean all build artifacts"
    echo "  help     Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 host          # Quick validation"
    echo "  $0 unity         # Prepare device tests"
    echo "  $0 all           # Full CI-like test run"
}

# Main script logic
case "${1:-all}" in
    "host")
        run_host_tests
        ;;
    "unity")
        run_unity_tests
        ;;
    "build")
        run_main_build
        ;;
    "all")
        print_header "Running Complete Test Suite"
        
        failed=0
        
        if ! run_host_tests; then
            failed=1
        fi
        
        if ! run_main_build; then
            failed=1
        fi
        
        if ! run_unity_tests; then
            failed=1
        fi
        
        if [ $failed -eq 0 ]; then
            print_success "All tests and builds completed successfully!"
            exit 0
        else
            print_error "Some tests or builds failed!"
            exit 1
        fi
        ;;
    "clean")
        cleanup
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    *)
        echo "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
