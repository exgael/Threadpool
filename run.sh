#!/bin/bash

# Remove the old build directory if it exists
if [ -d "build" ]; then
    rm -rf build
fi

# Create the new build directory
mkdir build

# Run CMake and build the project
cmake -S . -B build
cd build
make

# Run the tests
make test

# Go back to the project's root directory
cd ..
