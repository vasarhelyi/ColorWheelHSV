#!/bin/bash

echo ""
echo "Configuring ColorWheelHSV for host platform..."
mkdir -p build
cd build
rm -f CMakeCache.txt
cmake ..
cd ..
echo ""

