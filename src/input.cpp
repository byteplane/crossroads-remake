#include "input.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// Global input state
InputState input;
TouchState touch;

void InputState::reset() {
    keysPressed.fill(false);
    keysReleased.fill(false);
    mouseButtonsPressed[0] = mouseButtonsPressed[1] = mouseButtonsPressed[2] = false;
    mouseButtonsReleased[0] = mouseButtonsReleased[1] = mouseButtonsReleased[2] = false;
    gamepadButtonsPressed.fill(false);
    gamepadButtonsReleased.fill(false);
    mouseDeltaX = mouseDeltaY = 0;
    actionPressed = secondaryPressed = false;
}

void initializeGamepad() {
    // Initialize game controller subsystem
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        std::cout << "Warning: Could not initialize game controller subsystem: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Check for connected game controllers
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            input.gamepad = SDL_GameControllerOpen(i);
            if (input.gamepad) {
                input.gamepadConnected = true;
                std::cout << "Gamepad connected: " << SDL_GameControllerName(input.gamepad) << std::endl;
                break; // Use first available controller
            }
        }
    }
}

void updateVirtualInput() {
    // Reset virtual input
    input.moveX = input.moveY = 0.0f;
    
    // Keyboard input (WASD and arrow keys)
    if (input.keys[SDL_SCANCODE_A] || input.keys[SDL_SCANCODE_LEFT]) input.moveX -= 1.0f;
    if (input.keys[SDL_SCANCODE_D] || input.keys[SDL_SCANCODE_RIGHT]) input.moveX += 1.0f;
    if (input.keys[SDL_SCANCODE_W] || input.keys[SDL_SCANCODE_UP]) input.moveY -= 1.0f;
    if (input.keys[SDL_SCANCODE_S] || input.keys[SDL_SCANCODE_DOWN]) input.moveY += 1.0f;
    
    // Gamepad input
    if (input.gamepadConnected && input.gamepad) {
        float leftX = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTX];
        float leftY = input.gamepadAxes[SDL_CONTROLLER_AXIS_LEFTY];
        
        // Apply deadzone
        const float deadzone = 0.2f;
        if (std::abs(leftX) > deadzone) input.moveX += leftX;
        if (std::abs(leftY) > deadzone) input.moveY += leftY;
        
        // D-pad input
        if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_LEFT]) input.moveX -= 1.0f;
        if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]) input.moveX += 1.0f;
        if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_UP]) input.moveY -= 1.0f;
        if (input.gamepadButtons[SDL_CONTROLLER_BUTTON_DPAD_DOWN]) input.moveY += 1.0f;
    }
    
    // Touch input (virtual joystick)
    if (touch.active) {
        float deltaX = touch.currentX - touch.startX;
        float deltaY = touch.currentY - touch.startY;
        float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        
        const float maxDistance = 50.0f; // pixels
        if (distance > 5.0f) { // minimum movement threshold
            input.moveX += std::max(-1.0f, std::min(1.0f, deltaX / maxDistance));
            input.moveY += std::max(-1.0f, std::min(1.0f, deltaY / maxDistance));
        }
    }
    
    // Clamp movement to [-1, 1]
    input.moveX = std::max(-1.0f, std::min(1.0f, input.moveX));
    input.moveY = std::max(-1.0f, std::min(1.0f, input.moveY));
    
    // Action buttons
    input.actionPressed = input.keysPressed[SDL_SCANCODE_SPACE] || 
                         input.keysPressed[SDL_SCANCODE_RETURN] ||
                         input.mouseButtonsPressed[0] ||
                         (input.gamepadConnected && input.gamepadButtonsPressed[SDL_CONTROLLER_BUTTON_A]);
                         
    input.secondaryPressed = input.keysPressed[SDL_SCANCODE_LSHIFT] ||
                            input.keysPressed[SDL_SCANCODE_RSHIFT] ||
                            input.mouseButtonsPressed[1] ||
                            (input.gamepadConnected && input.gamepadButtonsPressed[SDL_CONTROLLER_BUTTON_B]);
}

void handleEvent(SDL_Event& e) {
    switch (e.type) {
        case SDL_KEYDOWN:
            if (e.key.repeat == 0) { // Ignore key repeats
                input.keysPressed[e.key.keysym.scancode] = true;
            }
            input.keys[e.key.keysym.scancode] = true;
            break;
            
        case SDL_KEYUP:
            input.keys[e.key.keysym.scancode] = false;
            input.keysReleased[e.key.keysym.scancode] = true;
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (e.button.button >= 1 && e.button.button <= 3) {
                input.mouseButtons[e.button.button - 1] = true;
                input.mouseButtonsPressed[e.button.button - 1] = true;
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (e.button.button >= 1 && e.button.button <= 3) {
                input.mouseButtons[e.button.button - 1] = false;
                input.mouseButtonsReleased[e.button.button - 1] = true;
            }
            break;
            
        case SDL_MOUSEMOTION:
            input.mouseDeltaX = e.motion.x - input.mouseX;
            input.mouseDeltaY = e.motion.y - input.mouseY;
            input.mouseX = e.motion.x;
            input.mouseY = e.motion.y;
            break;
            
        case SDL_CONTROLLERDEVICEADDED:
            if (!input.gamepadConnected) {
                input.gamepad = SDL_GameControllerOpen(e.cdevice.which);
                if (input.gamepad) {
                    input.gamepadConnected = true;
                    std::cout << "Gamepad connected: " << SDL_GameControllerName(input.gamepad) << std::endl;
                }
            }
            break;
            
        case SDL_CONTROLLERDEVICEREMOVED:
            if (input.gamepad && e.cdevice.which == SDL_JoystickInstanceID(
                SDL_GameControllerGetJoystick(input.gamepad))) {
                SDL_GameControllerClose(input.gamepad);
                input.gamepad = nullptr;
                input.gamepadConnected = false;
                std::cout << "Gamepad disconnected" << std::endl;
            }
            break;
            
        case SDL_CONTROLLERBUTTONDOWN:
            if (input.gamepadConnected && e.cbutton.button < SDL_CONTROLLER_BUTTON_MAX) {
                input.gamepadButtons[e.cbutton.button] = true;
                input.gamepadButtonsPressed[e.cbutton.button] = true;
            }
            break;
            
        case SDL_CONTROLLERBUTTONUP:
            if (input.gamepadConnected && e.cbutton.button < SDL_CONTROLLER_BUTTON_MAX) {
                input.gamepadButtons[e.cbutton.button] = false;
                input.gamepadButtonsReleased[e.cbutton.button] = true;
            }
            break;
            
        case SDL_CONTROLLERAXISMOTION:
            if (input.gamepadConnected && e.caxis.axis < SDL_CONTROLLER_AXIS_MAX) {
                // Convert from -32768..32767 to -1.0..1.0
                input.gamepadAxes[e.caxis.axis] = e.caxis.value / 32768.0f;
            }
            break;
            
        case SDL_FINGERDOWN:
            touch.active = true;
            touch.startX = touch.currentX = e.tfinger.x;
            touch.startY = touch.currentY = e.tfinger.y;
            touch.startTime = SDL_GetTicks();
            break;
            
        case SDL_FINGERUP:
            // Check for tap (quick touch without much movement)
            if (touch.active) {
                uint32_t duration = SDL_GetTicks() - touch.startTime;
                float deltaX = touch.currentX - touch.startX;
                float deltaY = touch.currentY - touch.startY;
                float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
                
                if (duration < 200 && distance < 0.02f) { // Normalized coordinates
                    // Register as action press
                    input.actionPressed = true;
                }
            }
            touch.active = false;
            break;
            
        case SDL_FINGERMOTION:
            if (touch.active) {
                touch.currentX = e.tfinger.x;
                touch.currentY = e.tfinger.y;
            }
            break;
    }
}

void renderInputDebug(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    // Movement indicator (white square that moves based on input)
    if (input.moveX != 0.0f || input.moveY != 0.0f) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        int offsetX = (int)(input.moveX * 50);
        int offsetY = (int)(input.moveY * 50);
        
        SDL_Rect moveRect = {centerX + offsetX - 10, centerY + offsetY - 10, 20, 20};
        SDL_RenderFillRect(renderer, &moveRect);
    }
    
    // Action button feedback (red square when pressed)
    if (input.actionPressed) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect actionRect = {50, 50, 30, 30};
        SDL_RenderFillRect(renderer, &actionRect);
    }
    
    // Secondary button feedback (green square when pressed)
    if (input.secondaryPressed) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect secondaryRect = {100, 50, 30, 30};
        SDL_RenderFillRect(renderer, &secondaryRect);
    }
    
    // Mouse position indicator
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect mouseRect = {input.mouseX - 2, input.mouseY - 2, 4, 4};
    SDL_RenderFillRect(renderer, &mouseRect);
    
    // Touch indicator
    if (touch.active) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        int touchX = (int)(touch.currentX * screenWidth);
        int touchY = (int)(touch.currentY * screenHeight);
        SDL_Rect touchRect = {touchX - 5, touchY - 5, 10, 10};
        SDL_RenderFillRect(renderer, &touchRect);
        
        // Draw line from start to current position
        int startX = (int)(touch.startX * screenWidth);
        int startY = (int)(touch.startY * screenHeight);
        SDL_RenderDrawLine(renderer, startX, startY, touchX, touchY);
    }
}