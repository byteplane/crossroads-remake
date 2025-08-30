#!/bin/bash

# C64-inspired Game Development Environment Setup for Ubuntu
# This script installs Emscripten, SDL2, development tools, and graphics utilities

set -e  # Exit on any error

echo "=== C64 Game Development Environment Setup ==="
echo "Setting up tools for Crossroads remake development..."
echo

# Update system packages
echo "📦 Updating system packages..."
sudo apt update
sudo apt upgrade -y

# Install essential build tools
echo "🔧 Installing build essentials..."
sudo apt install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    nodejs \
    npm \
    pkg-config

# Install SDL2 and development libraries
echo "🎮 Installing SDL2 and related libraries..."
sudo apt install -y \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    libsdl2-net-dev \
    libsdl2-gfx-dev

# Install additional useful libraries
echo "📚 Installing additional development libraries..."
sudo apt install -y \
    libjson-c-dev \
    libpng-dev \
    libjpeg-dev \
    libfreetype6-dev \
    zlib1g-dev

# Install Emscripten
echo "🌐 Installing Emscripten WebAssembly toolchain..."
cd ~
if [ ! -d "emsdk" ]; then
    git clone https://github.com/emscripten-core/emsdk.git
fi

cd emsdk
git pull
./emsdk install latest
./emsdk activate latest

# Add Emscripten to current session
source ./emsdk_env.sh

# Add Emscripten to bashrc for future sessions
if ! grep -q "source ~/emsdk/emsdk_env.sh" ~/.bashrc; then
    echo "" >> ~/.bashrc
    echo "# Emscripten WebAssembly toolchain" >> ~/.bashrc
    echo "source ~/emsdk/emsdk_env.sh" >> ~/.bashrc
fi

# Install graphics and level design tools
echo "🎨 Installing graphics and design tools..."

# Aseprite (pixel art editor) - install via Flatpak if available
if command -v flatpak &> /dev/null; then
    echo "Installing Aseprite via Flatpak..."
    sudo apt install -y flatpak
    flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
    flatpak install -y flathub org.aseprite.Aseprite
else
    echo "⚠️  Flatpak not available. Please install Aseprite manually from https://www.aseprite.org/"
fi

# Tiled Map Editor
echo "🗺️  Installing Tiled Map Editor..."
sudo apt install -y tiled

# GIMP for additional graphics work
echo "Installing GIMP for graphics editing..."
sudo apt install -y gimp

# Install Audacity for audio editing
echo "🔊 Installing Audacity for audio editing..."
sudo apt install -y audacity

# Install VSCode (optional but recommended)
echo "💻 Installing Visual Studio Code..."
if ! command -v code &> /dev/null; then
    wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
    sudo install -o root -g root -m 644 packages.microsoft.gpg /etc/apt/trusted.gpg.d/
    sudo sh -c 'echo "deb [arch=amd64,arm64,armhf signed-by=/etc/apt/trusted.gpg.d/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" > /etc/apt/sources.list.d/vscode.list'
    sudo apt update
    sudo apt install -y code
fi

# Install useful VSCode extensions for C++ and WebAssembly development
if command -v code &> /dev/null; then
    echo "Installing useful VSCode extensions..."
    code --install-extension ms-vscode.cpptools
    code --install-extension ms-vscode.cmake-tools
    code --install-extension ms-vscode.hexeditor
fi

# Create project directory structure
echo "📁 Setting up project directory structure..."
cd ~
mkdir -p ~/gamedev/crossroads-remake
cd ~/gamedev/crossroads-remake

# Create basic project structure
mkdir -p {src,assets/{sprites,sounds,maps},build,docs,tools}

# Create a basic CMakeLists.txt template
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(CrossroadsRemake)

set(CMAKE_CXX_STANDARD 17)

# Find SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
EOF

# Create a basic Makefile for Emscripten builds
cat > Makefile.emscripten << 'EOF'
# Emscripten build configuration
CXX = em++
CXXFLAGS = -std=c++17 -O2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2
CXXFLAGS += --preload-file assets
CXXFLAGS += -s ALLOW_MEMORY_GROWTH=1
CXXFLAGS += -s EXPORTED_FUNCTIONS='["_main"]'
CXXFLAGS += -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'

SOURCES = $(wildcard src/*.cpp)
TARGET = build/crossroads.html

$(TARGET): $(SOURCES)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -rf build/*

.PHONY: clean
EOF

# Create README with build instructions
cat > README.md << 'EOF'
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
EOF

echo
echo "✅ Setup complete!"
echo
echo "📍 Project created at: ~/gamedev/crossroads-remake"
echo
echo "🔄 To activate Emscripten in your current terminal:"
echo "   source ~/emsdk/emsdk_env.sh"
echo
echo "🏗️  To test the basic setup:"
echo "   cd ~/gamedev/crossroads-remake"
echo "   mkdir -p build && cd build"
echo "   cmake .. && make"
echo "   ./CrossroadsRemake"
echo
echo "🌐 To build for web:"
echo "   cd ~/gamedev/crossroads-remake"
echo "   source ~/emsdk/emsdk_env.sh"
echo "   make -f Makefile.emscripten"
echo
echo "🎨 Graphics tools installed:"
echo "   - Tiled (map editor): tiled"
echo "   - GIMP: gimp"
echo "   - Aseprite: flatpak run org.aseprite.Aseprite (if Flatpak available)"
echo
echo "🔊 Audio tools:"
echo "   - Audacity: audacity"
echo
echo "Happy coding! 🎮"
