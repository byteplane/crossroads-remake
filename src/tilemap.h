#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <string>
#include <fstream>

// Constants
const int TILE_SIZE = 16;

// Tile types
enum class TileType : uint8_t {
    EMPTY = 0,
    FLOOR = 1,
    
    // Wall types matching original Crossroads style
    WALL_BRICK = 2,          // Orange brick wall (main wall type)
    WALL_TOP = 3,            // Top edge of wall
    WALL_BOTTOM = 4,         // Bottom edge of wall  
    WALL_LEFT = 5,           // Left edge of wall
    WALL_RIGHT = 6,          // Right edge of wall
    WALL_TOP_LEFT = 7,       // Top-left corner
    WALL_TOP_RIGHT = 8,      // Top-right corner
    WALL_BOTTOM_LEFT = 9,    // Bottom-left corner
    WALL_BOTTOM_RIGHT = 10,  // Bottom-right corner
    WALL_INNER_TOP_LEFT = 11,     // Inner corner top-left
    WALL_INNER_TOP_RIGHT = 12,    // Inner corner top-right
    WALL_INNER_BOTTOM_LEFT = 13,  // Inner corner bottom-left
    WALL_INNER_BOTTOM_RIGHT = 14, // Inner corner bottom-right
    
    // Additional tile types
    WATER = 15,
    GRASS = 16,
    
    // Add more tile types as needed
    MAX_TILES = 255
};

// Individual tile data
struct Tile {
    TileType type = TileType::EMPTY;
    uint8_t variant = 0;  // For tile variations (different sprites for same type)
    bool solid = false;   // For collision detection
    
    Tile() = default;
    Tile(TileType t, bool isSolid = false, uint8_t var = 0) 
        : type(t), variant(var), solid(isSolid) {}
};

// Tilemap class
class Tilemap {
private:
    int width_;
    int height_;
    std::vector<Tile> tiles_;
    SDL_Texture* tileTexture_;
    int tilesPerRow_;  // Number of tiles per row in the texture
    
public:
    Tilemap(int width, int height);
    ~Tilemap();
    
    // Map management
    void resize(int width, int height);
    void clear();
    void fill(TileType type, bool solid = false);
    
    // Tile access
    Tile& getTile(int x, int y);
    const Tile& getTile(int x, int y) const;
    void setTile(int x, int y, TileType type, bool solid = false, uint8_t variant = 0);
    
    // Utility functions
    bool isValidPosition(int x, int y) const;
    bool isSolid(int x, int y) const;
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    
    // Texture management
    bool loadTileTexture(SDL_Renderer* renderer, const char* filename, int tilesPerRow = 16);
    void createDefaultTexture(SDL_Renderer* renderer);
    
    // Rendering
    void render(SDL_Renderer* renderer, int cameraX = 0, int cameraY = 0, 
                int screenWidth = 640, int screenHeight = 400) const;
    void renderTile(SDL_Renderer* renderer, TileType type, uint8_t variant, 
                   int screenX, int screenY) const;
    
    // CSV map loading
    bool loadFromCSV(const std::string& filename);
    std::vector<std::string> getAvailableMaps() const;
    
    // Map generation (for testing)
    void generateTestMap();
    void generateCheckerboard();
    void generateBorder();
    
private:
    // Helper functions for testing map generation
    void placeBrickWalls(int x, int y, int width, int height);
    void createRoom(int x, int y, int width, int height);
    void createCorridor(int x1, int y1, int x2, int y2, bool horizontal = true);
    
    // Coordinate conversion
    static int worldToTileX(int worldX) { return worldX / TILE_SIZE; }
    static int worldToTileY(int worldY) { return worldY / TILE_SIZE; }
    static int tileToWorldX(int tileX) { return tileX * TILE_SIZE; }
    static int tileToWorldY(int tileY) { return tileY * TILE_SIZE; }
};