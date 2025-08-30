# Crossroads Remake

A modern recreation of the classic Commodore 64 game Crossroads, built with C++ and SDL2, targeting both native Linux and WebAssembly.

## Building

### Native Linux Build
```bash
mkdir -p build
cd build
cmake ..
make
./CrossroadsRemake
```

### WebAssembly Build
```bash
# Make sure Emscripten is activated
source ~/emsdk/emsdk_env.sh

# Build for web
make -f Makefile.emscripten

# Serve locally (from build directory)
python3 -m http.server 8000
# Then open http://localhost:8000/crossroads.html
```

## Project Structure
- `src/` - C++ source code
- `assets/` - Game assets (sprites, sounds, maps)
- `build/` - Build output
- `docs/` - Documentation
- `tools/` - Development utilities
