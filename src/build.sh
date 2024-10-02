#!/bin/bash

# Step 1: Remove the previous build folder
rm -rf ../build

# Step 4: Run CMake to configure the project
cmake -B ../build

# Step 5: Build the project
make -C ../build

# Step 6: Run the executable
../build/tpOpenGL