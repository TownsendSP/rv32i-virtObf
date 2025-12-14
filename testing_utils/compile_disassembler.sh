#!/bin/bash
# completely llm-generated
set -e

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Project root: $PROJECT_ROOT"

# Create build directory if it doesn't exist
BUILD_DIR="$PROJECT_ROOT/cmake-build-debug"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake configuration
echo "Configuring CMake..."
cmake ..

# Build the disassembler target
echo "Building disassembler..."
cmake --build . --target disassembler

echo "Compilation complete."
