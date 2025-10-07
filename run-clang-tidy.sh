#!/bin/bash

# Check for build directory
if [ ! -d "./build" ]; then
    echo "Error: Build directory './build' not found."
    echo "Please generate it with: cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .."
    exit 1
fi

# Get macOS SDK path
SDK_PATH=$(xcrun --show-sdk-path 2>/dev/null)

if [ -z "$SDK_PATH" ]; then
    echo "Error: Failed to find macOS SDK path. Make sure Xcode is installed."
    exit 1
fi

# echo $SDK_PATH
TIDY_SUFFIX="--extra-arg=-isysroot --extra-arg=$SDK_PATH --extra-arg=-I/usr/include"
    

# Process include/ directory with .h and .hpp files
find include/ -type f -name "*.h" -exec clang-tidy -fix -p ./build $TIDY_SUFFIX {} \;
find include/ -type f -name "*.hpp" -exec clang-tidy -fix -p ./build $TIDY_SUFFIX {} \;

# Process src/ directory with .c and .cpp files; -fix-errors to fix according to header changes
find src/ -type f -name "*.c" -exec clang-tidy -fix-errors -p ./build $TIDY_SUFFIX {} \;
find src/ -type f -name "*.cpp" -exec clang-tidy -fix-errors -p ./build $TIDY_SUFFIX {} \;

echo "clang-tidy refactoring complete!"