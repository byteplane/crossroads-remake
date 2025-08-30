# Crossroads Remake Project Memory

## Project Overview
This is a C++ recreation of the classic Commodore 64 game "Crossroads" using SDL2, targeting both native Linux and WebAssembly.

## Build Commands
- **Native build**: `mkdir -p build && cd build && cmake .. && make`
- **WebAssembly build**: `make -f Makefile.emscripten`
- **Run native**: `./build/CrossroadsRemake`
- **Test commands**: No automated tests configured yet

## Code Conventions
- **Language**: C++17
- **Naming**: snake_case for variables/functions, PascalCase for classes
- **Headers**: Use `#pragma once`
- **Memory**: Manual memory management with `new`/`delete`
- **Constants**: ALL_CAPS for defines, const for runtime constants

## Architecture
- **main.cpp**: Game loop, event handling, camera control
- **tilemap.h/cpp**: Tile-based rendering and maze generation algorithms
- **input.h/cpp**: Cross-platform input abstraction (keyboard/gamepad/touch)
- **Assets**: Located in `assets/` (sprites, sounds, maps)

## Key Features
- CSV-based tile map system with external map files
- Procedural maze generator tool with flood-fill algorithm
- Camera system with smooth scrolling and boundary clamping
- Multi-platform input handling (desktop and web)
- 16x16 pixel tile system with detailed wall corner types
- Support for symmetry, loops, rooms, and configurable generation parameters

## Controls
- Movement: WASD/Arrow keys (camera)
- N: Cycle through CSV maps
- ESC: Quit

## Map System
- **CSV Format**: First line contains width,height; remaining lines contain tile IDs
- **Tile IDs**: 0=empty, 1=floor, 2=brick wall, 15=water, 16=grass
- **Location**: Maps stored in `assets/maps/*.csv`
- **Generator Tool**: `./build/generate_maps samples` creates procedural maps
- **Custom Generation**: `./build/generate_maps custom <w> <h> <file.csv> [params]`

## Dependencies
- SDL2 (graphics/input)
- SDL2_image (texture loading)
- SDL2_mixer (audio)
- Emscripten (for WebAssembly builds)
- C++17 filesystem support