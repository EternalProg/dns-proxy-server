#!/bin/bash

BUILD_MODE="${1:-debug}"

BUILD_MODE=$(echo "$BUILD_MODE" | tr '[:upper:]' '[:lower:]')

if [[ "$BUILD_MODE" == "debug" || "$BUILD_MODE" == "d" ]]; then
  echo "Building in DEBUG mode..."
  echo "cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug"
  cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
  echo "cmake --build build/debug"
  cmake --build build/debug

elif [[ "$BUILD_MODE" == "release" || "$BUILD_MODE" == "r" ]]; then
  echo "Building in RELEASE mode..."
  echo "cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release"
  cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
  echo "cmake --build build/release"
  cmake --build build/release

else
  echo "ERROR: Unknown build mode: '$BUILD_MODE'"
  echo "Usage: ./build.sh [debug|release] (default: debug)"
  exit 1
fi

