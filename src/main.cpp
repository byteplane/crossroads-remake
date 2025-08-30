#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include "input.h"
#include "tilemap.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 400;

// Game window and renderer
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
bool running = true;

// Game world
Tilemap* tilemap = nullptr;
int cameraX = 0, cameraY = 0;
std::vector<std::string> availableMaps;
int currentMapIndex = 0;





void gameLoop() {
    // Reset per-frame input state
    input.reset();
    
    // Handle events
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
        
        // Special case: ESC to quit (useful for web version)
        if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            running = false;
        }
        
        handleEvent(e);
    }
    
    // Update virtual input state
    updateVirtualInput();
    
    // Update camera based on input
    const float cameraSpeed = 2.0f;
    cameraX += (int)(input.moveX * cameraSpeed);
    cameraY += (int)(input.moveY * cameraSpeed);
    
    // Clamp camera to map bounds
    int maxCameraX = std::max(0, tilemap->getWidth() * TILE_SIZE - SCREEN_WIDTH);
    int maxCameraY = std::max(0, tilemap->getHeight() * TILE_SIZE - SCREEN_HEIGHT);
    cameraX = std::max(0, std::min(cameraX, maxCameraX));
    cameraY = std::max(0, std::min(cameraY, maxCameraY));
    
    // Cycle through different maps with 'n' key
    if (input.keysPressed[SDL_SCANCODE_N]) {
        if (!availableMaps.empty()) {
            currentMapIndex = (currentMapIndex + 1) % availableMaps.size();
            if (tilemap->loadFromCSV(availableMaps[currentMapIndex])) {
                // Reset camera to center
                cameraX = (tilemap->getWidth() * TILE_SIZE - SCREEN_WIDTH) / 2;
                cameraY = (tilemap->getHeight() * TILE_SIZE - SCREEN_HEIGHT) / 2;
            }
        }
    }
    
    
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer);
    
    // Render tilemap
    if (tilemap) {
        tilemap->render(renderer, cameraX, cameraY, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
    
    // Render input debug visualization (smaller, in corner)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
    SDL_Rect debugBg = {SCREEN_WIDTH - 120, 10, 110, 60};
    SDL_RenderFillRect(renderer, &debugBg);
    
    // Show movement as small indicator
    if (input.moveX != 0.0f || input.moveY != 0.0f) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        int centerX = SCREEN_WIDTH - 65;
        int centerY = 40;
        int offsetX = (int)(input.moveX * 20);
        int offsetY = (int)(input.moveY * 20);
        SDL_Rect moveRect = {centerX + offsetX - 3, centerY + offsetY - 3, 6, 6};
        SDL_RenderFillRect(renderer, &moveRect);
    }
    
    SDL_RenderPresent(renderer);
    
#ifndef __EMSCRIPTEN__
    // Only exit in native builds; web version handles this differently
    if (!running) {
        exit(0);
    }
#endif
}

int main(int argc, char* argv[]) {
    // Initialize SDL with video and game controller support
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    window = SDL_CreateWindow("Crossroads Remake - Maze Generator",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_WIDTH, SCREEN_HEIGHT,
                             SDL_WINDOW_SHOWN);
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize gamepad support
    initializeGamepad();
    
    // Create and initialize tilemap
    tilemap = new Tilemap(50, 30);  // 50x30 tiles (800x480 world)
    tilemap->createDefaultTexture(renderer);
    
    // Load available maps and set initial map
    availableMaps = tilemap->getAvailableMaps();
    if (!availableMaps.empty()) {
        tilemap->loadFromCSV(availableMaps[0]);
    } else {
        std::cout << "Warning: No CSV maps found, using test pattern" << std::endl;
        tilemap->generateTestMap();
    }
    
    // Center camera initially
    cameraX = (tilemap->getWidth() * TILE_SIZE - SCREEN_WIDTH) / 2;
    cameraY = (tilemap->getHeight() * TILE_SIZE - SCREEN_HEIGHT) / 2;
    
    std::cout << "=== Crossroads Maze Generator ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Movement: WASD, Arrow Keys (camera movement)" << std::endl;
    std::cout << "  N: Cycle through available maps" << std::endl;
    std::cout << "  Quit: ESC" << std::endl;
    std::cout << "Map size: " << tilemap->getWidth() << "x" << tilemap->getHeight() << " tiles" << std::endl;
    
    if (!availableMaps.empty()) {
        std::cout << "Found " << availableMaps.size() << " CSV maps" << std::endl;
        std::cout << "Current map: " << availableMaps[currentMapIndex] << std::endl;
    } else {
        std::cout << "No CSV maps found, using generated maze" << std::endl;
    }
    
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(gameLoop, 60, 1);
#else
    while (running) {
        gameLoop();
        SDL_Delay(16); // ~60 FPS
    }
#endif
    
    // Cleanup
    delete tilemap;
    if (input.gamepad) {
        SDL_GameControllerClose(input.gamepad);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}