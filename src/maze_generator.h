#pragma once

#include <vector>
#include <string>
#include <random>

struct MazeConfig {
    struct Axis {
        bool symmetry = false;
        int border = 1;
        bool loop = false;
    };
    
    Axis horizontal;
    Axis vertical;
    float straightness = 0.0f;  // 0.0 to 1.0
    float imperfect = 0.0f;     // 0.0 to 1.0 (adds loops)
    float fill = 1.0f;          // 0.0 to 1.0 (density)
    float roomsFraction = 0.0f; // 0.0 to 1.0 (add rooms at dead ends)
    int hallWidth = 1;
    int wallWidth = 1;
    unsigned int seed = 0;      // 0 = random seed
};

struct DeadEnd {
    int x, y;
};

class MazeGenerator {
public:
    // Generate maze with given configuration
    static std::vector<std::vector<int>> generate(int width, int height, const MazeConfig& config = MazeConfig{});
    
    // Export maze to CSV format
    static bool exportToCSV(const std::vector<std::vector<int>>& maze, const std::string& filename);
    
    // Convert tile values: 0=floor(1), 255=wall(2), other=reserved
    static int convertTileValue(int mazeValue);

private:
    static constexpr int SOLID = 255;
    static constexpr int RESERVED = 127; 
    static constexpr int EMPTY = 0;
    
    struct Direction {
        int x, y;
    };
    
    struct StackEntry {
        int x, y;
        Direction step;
    };
    
    // Helper functions
    static void shuffle(std::vector<Direction>& directions, std::mt19937& rng);
    static bool unexplored(const std::vector<std::vector<int>>& maze, int x, int y, int ignoreReserved);
    static void setMaze(std::vector<std::vector<int>>& maze, int x, int y, int value, 
                       const MazeConfig& config, int w, int h);
    static void addRooms(std::vector<std::vector<int>>& maze, const std::vector<DeadEnd>& deadEnds,
                        const MazeConfig& config);
};