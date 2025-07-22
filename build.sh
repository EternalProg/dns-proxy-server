#!/bin/bash

BUILD_MODE="${1:-debug}"

BUILD_MODE=$(echo "$BUILD_MODE" | tr '[:upper:]' '[:lower:]')

if [[ "$BUILD_MODE" == "debug" || "$BUILD_MODE" == "d" ]]; then
  DEBUG_BUILD_FLAGS="-DLOG_LEVEL=0"
  echo "Building in DEBUG mode..."
  echo "cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug $DEBUG_BUILD_FLAGS"
  cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug $DEBUG_BUILD_FLAGS
  echo "cmake --build build/debug"
  cmake --build build/debug

elif [[ "$BUILD_MODE" == "release" || "$BUILD_MODE" == "r" ]]; then
  RELEASE_BUILD_FLAGS="-DLOG_LEVEL=2"
  echo "Building in RELEASE mode..."
  echo "cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release $RELEASE_BUILD_FLAGS"
  cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release $RELEASE_BUILD_FLAGS
  echo "cmake --build build/release"
  cmake --build build/release

else
  echo "ERROR: Unknown build mode: '$BUILD_MODE'"
  echo "Usage: ./build.sh [debug|release] (default: debug)"
  exit 1
fi
