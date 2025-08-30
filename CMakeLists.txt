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

# Map generator tool
add_executable(generate_maps tools/generate_maps.cpp src/maze_generator.cpp)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
