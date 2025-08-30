#include "tilemap.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <filesystem>

Tilemap::Tilemap(int width, int height) 
    : width_(width), height_(height), tileTexture_(nullptr), tilesPerRow_(16) {
    tiles_.resize(width_ * height_);
}

Tilemap::~Tilemap() {
    if (tileTexture_) {
        SDL_DestroyTexture(tileTexture_);
    }
}

void Tilemap::resize(int width, int height) {
    width_ = width;
    height_ = height;
    tiles_.resize(width_ * height_);
}

void Tilemap::clear() {
    for (auto& tile : tiles_) {
        tile = Tile();
    }
}

void Tilemap::fill(TileType type, bool solid) {
    for (auto& tile : tiles_) {
        tile = Tile(type, solid);
    }
}

Tile& Tilemap::getTile(int x, int y) {
    static Tile emptyTile;
    if (!isValidPosition(x, y)) {
        return emptyTile;
    }
    return tiles_[y * width_ + x];
}

const Tile& Tilemap::getTile(int x, int y) const {
    static const Tile emptyTile;
    if (!isValidPosition(x, y)) {
        return emptyTile;
    }
    return tiles_[y * width_ + x];
}

void Tilemap::setTile(int x, int y, TileType type, bool solid, uint8_t variant) {
    if (isValidPosition(x, y)) {
        tiles_[y * width_ + x] = Tile(type, solid, variant);
    }
}

bool Tilemap::isValidPosition(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

bool Tilemap::isSolid(int x, int y) const {
    return getTile(x, y).solid;
}

bool Tilemap::loadTileTexture(SDL_Renderer* renderer, const char* filename, int tilesPerRow) {
    SDL_Surface* surface = SDL_LoadBMP(filename);
    if (!surface) {
        std::cout << "Warning: Could not load tile texture " << filename << ": " << SDL_GetError() << std::endl;
        createDefaultTexture(renderer);
        return false;
    }
    
    tileTexture_ = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!tileTexture_) {
        std::cout << "Warning: Could not create tile texture: " << SDL_GetError() << std::endl;
        createDefaultTexture(renderer);
        return false;
    }
    
    tilesPerRow_ = tilesPerRow;
    return true;
}

void Tilemap::createDefaultTexture(SDL_Renderer* renderer) {
    // Create texture for Crossroads-style walls
    const int textureSize = 256; // 16x16 tiles in a 256x256 texture
    SDL_Surface* surface = SDL_CreateRGBSurface(0, textureSize, textureSize, 32, 
                                                0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!surface) return;
    
    // Fill surface with black initially
    SDL_FillRect(surface, nullptr, 0x000000FF);
    
    SDL_Rect tileRect = {0, 0, TILE_SIZE, TILE_SIZE};
    
    // Colors matching original Crossroads style
    uint32_t brickOrange = 0xD2691EFF;  // Orange brick color
    uint32_t brickDark = 0x8B4513FF;    // Dark brown border
    uint32_t floorBlack = 0x000000FF;   // Black floor
    uint32_t brickLight = 0xF4A460FF;   // Light orange highlight
    
    for (int tileIndex = 0; tileIndex < 17; ++tileIndex) {
        int x = (tileIndex % tilesPerRow_) * TILE_SIZE;
        int y = (tileIndex / tilesPerRow_) * TILE_SIZE;
        tileRect.x = x;
        tileRect.y = y;
        
        TileType type = static_cast<TileType>(tileIndex);
        
        switch (type) {
            case TileType::EMPTY:
                SDL_FillRect(surface, &tileRect, floorBlack);
                break;
                
            case TileType::FLOOR:
                SDL_FillRect(surface, &tileRect, floorBlack);
                break;
                
            case TileType::WALL_BRICK:
                // Orange brick with dark border
                SDL_FillRect(surface, &tileRect, brickOrange);
                // Add dark border
                for (int i = 0; i < TILE_SIZE; ++i) {
                    uint32_t* pixels = (uint32_t*)surface->pixels;
                    int pitch = surface->pitch / 4;
                    // Top and bottom borders
                    pixels[(y) * pitch + (x + i)] = brickDark;
                    pixels[(y + TILE_SIZE - 1) * pitch + (x + i)] = brickDark;
                    // Left and right borders  
                    pixels[(y + i) * pitch + (x)] = brickDark;
                    pixels[(y + i) * pitch + (x + TILE_SIZE - 1)] = brickDark;
                }
                break;
                
            case TileType::WALL_TOP:
            case TileType::WALL_BOTTOM:
            case TileType::WALL_LEFT:
            case TileType::WALL_RIGHT:
            case TileType::WALL_TOP_LEFT:
            case TileType::WALL_TOP_RIGHT:
            case TileType::WALL_BOTTOM_LEFT:
            case TileType::WALL_BOTTOM_RIGHT:
            case TileType::WALL_INNER_TOP_LEFT:
            case TileType::WALL_INNER_TOP_RIGHT:
            case TileType::WALL_INNER_BOTTOM_LEFT:
            case TileType::WALL_INNER_BOTTOM_RIGHT:
                // All wall variations use same brick pattern for now
                SDL_FillRect(surface, &tileRect, brickOrange);
                // Add dark border
                for (int i = 0; i < TILE_SIZE; ++i) {
                    uint32_t* pixels = (uint32_t*)surface->pixels;
                    int pitch = surface->pitch / 4;
                    pixels[(y) * pitch + (x + i)] = brickDark;
                    pixels[(y + TILE_SIZE - 1) * pitch + (x + i)] = brickDark;
                    pixels[(y + i) * pitch + (x)] = brickDark;
                    pixels[(y + i) * pitch + (x + TILE_SIZE - 1)] = brickDark;
                }
                break;
                
            case TileType::WATER:
                SDL_FillRect(surface, &tileRect, 0x0000FFFF); // Blue
                break;
                
            case TileType::GRASS:
                SDL_FillRect(surface, &tileRect, 0x00FF00FF); // Green
                break;
                
            default:
                SDL_FillRect(surface, &tileRect, floorBlack);
                break;
        }
    }
    
    tileTexture_ = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!tileTexture_) {
        std::cout << "Error: Could not create default tile texture: " << SDL_GetError() << std::endl;
    }
}

void Tilemap::render(SDL_Renderer* renderer, int cameraX, int cameraY, 
                    int screenWidth, int screenHeight) const {
    if (!tileTexture_) return;
    
    // Calculate which tiles are visible
    int startTileX = std::max(0, cameraX / TILE_SIZE);
    int startTileY = std::max(0, cameraY / TILE_SIZE);
    int endTileX = std::min(width_ - 1, (cameraX + screenWidth) / TILE_SIZE + 1);
    int endTileY = std::min(height_ - 1, (cameraY + screenHeight) / TILE_SIZE + 1);
    
    // Render visible tiles
    for (int y = startTileY; y <= endTileY; ++y) {
        for (int x = startTileX; x <= endTileX; ++x) {
            const Tile& tile = getTile(x, y);
            if (tile.type != TileType::EMPTY) {
                int screenX = x * TILE_SIZE - cameraX;
                int screenY = y * TILE_SIZE - cameraY;
                renderTile(renderer, tile.type, tile.variant, screenX, screenY);
            }
        }
    }
}

void Tilemap::renderTile(SDL_Renderer* renderer, TileType type, uint8_t variant, 
                        int screenX, int screenY) const {
    if (!tileTexture_) return;
    
    int tileIndex = static_cast<int>(type) + variant;
    int srcX = (tileIndex % tilesPerRow_) * TILE_SIZE;
    int srcY = (tileIndex / tilesPerRow_) * TILE_SIZE;
    
    SDL_Rect srcRect = {srcX, srcY, TILE_SIZE, TILE_SIZE};
    SDL_Rect dstRect = {screenX, screenY, TILE_SIZE, TILE_SIZE};
    
    SDL_RenderCopy(renderer, tileTexture_, &srcRect, &dstRect);
}

void Tilemap::generateTestMap() {
    clear();
    
    // Create a simple test pattern
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (x == 0 || x == width_ - 1 || y == 0 || y == height_ - 1) {
                // Border walls
                setTile(x, y, TileType::WALL_BRICK, true);
            } else if ((x + y) % 4 == 0) {
                // Some scattered walls
                setTile(x, y, TileType::WALL_BRICK, true);
            } else if (x % 3 == 0 && y % 3 == 0) {
                // Water spots
                setTile(x, y, TileType::WATER, false);
            } else if ((x + y) % 3 == 1) {
                // Grass areas
                setTile(x, y, TileType::GRASS, false);
            } else {
                // Floor
                setTile(x, y, TileType::FLOOR, false);
            }
        }
    }
}

void Tilemap::generateCheckerboard() {
    clear();
    
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            TileType type = ((x + y) % 2 == 0) ? TileType::FLOOR : TileType::GRASS;
            setTile(x, y, type, false);
        }
    }
}

void Tilemap::generateBorder() {
    clear();
    fill(TileType::FLOOR, false);
    
    // Add border walls
    for (int x = 0; x < width_; ++x) {
        setTile(x, 0, TileType::WALL_BRICK, true);
        setTile(x, height_ - 1, TileType::WALL_BRICK, true);
    }
    
    for (int y = 0; y < height_; ++y) {
        setTile(0, y, TileType::WALL_BRICK, true);
        setTile(width_ - 1, y, TileType::WALL_BRICK, true);
    }
}


// Helper functions
void Tilemap::placeBrickWalls(int x, int y, int width, int height) {
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            setTile(x + dx, y + dy, TileType::WALL_BRICK, true);
        }
    }
}

void Tilemap::createRoom(int x, int y, int width, int height) {
    // Clear the room area
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            if (isValidPosition(x + dx, y + dy)) {
                setTile(x + dx, y + dy, TileType::FLOOR, false);
            }
        }
    }
}

void Tilemap::createCorridor(int x1, int y1, int x2, int y2, bool horizontal) {
    if (horizontal) {
        int startX = std::min(x1, x2);
        int endX = std::max(x1, x2);
        for (int x = startX; x <= endX; ++x) {
            if (isValidPosition(x, y1)) {
                setTile(x, y1, TileType::FLOOR, false);
            }
        }
    } else {
        int startY = std::min(y1, y2);
        int endY = std::max(y1, y2);
        for (int y = startY; y <= endY; ++y) {
            if (isValidPosition(x1, y)) {
                setTile(x1, y, TileType::FLOOR, false);
            }
        }
    }
}

bool Tilemap::loadFromCSV(const std::string& filename) {
    std::string fullPath = "assets/maps/" + filename;
    std::ifstream file(fullPath);
    
    if (!file.is_open()) {
        std::cout << "Error: Could not open map file: " << fullPath << std::endl;
        return false;
    }
    
    std::string line;
    
    // Read dimensions from first line
    if (!std::getline(file, line)) {
        std::cout << "Error: Empty map file: " << fullPath << std::endl;
        return false;
    }
    
    std::stringstream ss(line);
    std::string cell;
    
    if (!std::getline(ss, cell, ',')) {
        std::cout << "Error: Invalid dimensions in map file: " << fullPath << std::endl;
        return false;
    }
    int newWidth = std::stoi(cell);
    
    if (!std::getline(ss, cell, ',')) {
        std::cout << "Error: Invalid dimensions in map file: " << fullPath << std::endl;
        return false;
    }
    int newHeight = std::stoi(cell);
    
    // Resize map to match CSV dimensions
    resize(newWidth, newHeight);
    clear();
    
    // Read tile data
    int row = 0;
    while (std::getline(file, line) && row < height_) {
        std::stringstream rowSS(line);
        int col = 0;
        
        while (std::getline(rowSS, cell, ',') && col < width_) {
            int tileId = std::stoi(cell);
            TileType type = static_cast<TileType>(tileId);
            bool solid = (type >= TileType::WALL_BRICK && type <= TileType::WALL_INNER_BOTTOM_RIGHT);
            setTile(col, row, type, solid);
            col++;
        }
        row++;
    }
    
    file.close();
    std::cout << "Loaded map: " << filename << " (" << width_ << "x" << height_ << ")" << std::endl;
    return true;
}

std::vector<std::string> Tilemap::getAvailableMaps() const {
    std::vector<std::string> maps;
    std::string mapsDir = "assets/maps";
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(mapsDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                maps.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Warning: Could not read maps directory: " << e.what() << std::endl;
    }
    
    return maps;
}

