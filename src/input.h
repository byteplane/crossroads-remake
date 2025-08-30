#pragma once

#include <SDL2/SDL.h>
#include <array>

// Input state tracking
struct InputState {
    // Keyboard state
    std::array<bool, SDL_NUM_SCANCODES> keys{};
    std::array<bool, SDL_NUM_SCANCODES> keysPressed{};  // True for one frame when pressed
    std::array<bool, SDL_NUM_SCANCODES> keysReleased{}; // True for one frame when released
    
    // Mouse state
    int mouseX = 0, mouseY = 0;
    int mouseDeltaX = 0, mouseDeltaY = 0;
    bool mouseButtons[5] = {false}; // Left, Middle, Right, X1, X2
    bool mouseButtonsPressed[5] = {false};
    bool mouseButtonsReleased[5] = {false};
    
    // Joystick/Gamepad state (for first connected gamepad)
    SDL_GameController* gamepad = nullptr;
    bool gamepadConnected = false;
    std::array<bool, SDL_CONTROLLER_BUTTON_MAX> gamepadButtons{};
    std::array<bool, SDL_CONTROLLER_BUTTON_MAX> gamepadButtonsPressed{};
    std::array<bool, SDL_CONTROLLER_BUTTON_MAX> gamepadButtonsReleased{};
    std::array<float, SDL_CONTROLLER_AXIS_MAX> gamepadAxes{};
    
    // Virtual directional input (combines keyboard, gamepad, and touch)
    float moveX = 0.0f, moveY = 0.0f;  // -1.0 to 1.0
    bool actionPressed = false;  // Primary action (shoot/select)
    bool secondaryPressed = false;  // Secondary action (jump/cancel)
    
    void reset();
};

// Touch handling for mobile/web
struct TouchState {
    bool active = false;
    float startX = 0, startY = 0;
    float currentX = 0, currentY = 0;
    uint32_t startTime = 0;
};

// Global input state
extern InputState input;
extern TouchState touch;

// Function declarations
void initializeGamepad();
void updateVirtualInput();
void handleEvent(SDL_Event& e);
void renderInputDebug(SDL_Renderer* renderer, int screenWidth, int screenHeight);