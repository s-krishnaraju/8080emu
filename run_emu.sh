#!/bin/bash 

SOURCE_DIR="./"
BUILD_DIR="./build"

mkdir -p "$BUILD_DIR"

cmake -S "$SOURCE_DIR" -B "$BUILD_DIR"

cmake --build "$BUILD_DIR"

"$BUILD_DIR"/8080emu ./invaders_rom/invaders


