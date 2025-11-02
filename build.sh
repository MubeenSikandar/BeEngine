#!/bin/bash

echo "Building GameEngine..."

if [ ! -d "build" ]; then
    mkdir build
fi

cd build
cmake ..
cmake --build .

echo ""
echo "Build complete!"
echo "Run: ./build/bin/Sandbox"
