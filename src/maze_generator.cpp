#include "maze_generator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <cmath>

std::vector<std::vector<int>> MazeGenerator::generate(int w, int h, const MazeConfig& config) {
    const bool hSymmetry = config.horizontal.symmetry;
    const int hBorder = config.horizontal.border;
    const bool hWrap = config.horizontal.loop && !(hSymmetry && hBorder);
    
    const bool vSymmetry = config.vertical.symmetry;
    const int vBorder = config.vertical.border;
    const bool vWrap = config.vertical.loop && !(vSymmetry && vBorder);
    
    // Setup random number generator
    std::mt19937 rng;
    if (config.seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    } else {
        rng.seed(config.seed);
    }
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Account for edges that will later be stripped
    if (!hBorder) {
        ++w;
        if (!hWrap) ++w;
    }
    
    if (!vBorder) {
        ++h;
        if (!vWrap) ++h;
    }
    
    float imperfect = std::min(1.0f, std::max(0.0f, config.imperfect));
    float fill = config.fill;
    float reserveProb = std::pow(1.0f - std::min(std::max(0.0f, fill * 0.9f + 0.1f), 1.0f), 1.6f);
    
    // Ensure proper dimensions
    if (hWrap) {
        if (hSymmetry) {
            w = std::round((w - 2) / 4.0f) * 4 + 2;
        } else {
            w += w & 1;
        }
    } else {
        w += ~(w & 1);
    }
    
    if (vWrap) {
        if (vSymmetry) {
            h = std::round((h - 2) / 4.0f) * 4 + 2;
        } else {
            h += h & 1;
        }
    } else {
        h += ~(h & 1);
    }
    
    // Initialize maze to solid
    std::vector<std::vector<int>> maze(w, std::vector<int>(h, SOLID));
    
    // Reserve some regions
    if (reserveProb > 0) {
        for (int x = 1; x < w; x += 2) {
            for (int y = 1; y < h; y += 2) {
                if (dist(rng) < reserveProb) {
                    maze[x][y] = RESERVED;
                }
            }
        }
    }
    
    // Carve hallways using stack-based approach
    std::vector<StackEntry> stack;
    std::vector<DeadEnd> deadEnds;
    
    int startX = std::floor(w / 4.0f) * 2 - 1;
    int startY = std::floor(h / 4.0f) * 2 - 1;
    stack.push_back({startX, startY, {0, 0}});
    deadEnds.push_back({startX, startY});
    
    std::vector<Direction> directions = {{-1, 0}, {1, 0}, {0, 1}, {0, -1}};
    int ignoreReserved = std::max(w, h);
    
    while (!stack.empty()) {
        StackEntry cur = stack.back();
        stack.pop_back();
        
        if (unexplored(maze, cur.x, cur.y, ignoreReserved)) {
            // Mark visited
            setMaze(maze, cur.x, cur.y, EMPTY, config, w, h);
            
            // Carve wall back towards source
            setMaze(maze, cur.x - cur.step.x, cur.y - cur.step.y, EMPTY, config, w, h);
            
            --ignoreReserved;
            
            // Shuffle directions
            shuffle(directions, rng);
            
            // Prioritize straight lines
            if (dist(rng) < config.straightness) {
                for (int i = 0; i < 4; ++i) {
                    if (directions[i].x == cur.step.x && directions[i].y == cur.step.y) {
                        std::swap(directions[i], directions[3]);
                        break;
                    }
                }
            }
            
            // Check neighbors
            bool deadEnd = true;
            for (const auto& step : directions) {
                int x = cur.x + step.x * 2;
                int y = cur.y + step.y * 2;
                
                if (hWrap) x = (x + w) % w;
                if (vWrap) y = (y + h) % h;
                
                if (x >= 0 && y >= 0 && x < w && y < h && unexplored(maze, x, y, ignoreReserved)) {
                    stack.push_back({x, y, step});
                    deadEnd = false;
                }
            }
            
            if (deadEnd) {
                deadEnds.push_back({cur.x, cur.y});
            }
        }
    }
    
    // Add imperfections (loops)
    if (imperfect > 0) {
        int hBdry = hWrap ? 0 : 1;
        int vBdry = vWrap ? 0 : 1;
        
        auto remove = [&](int x, int y) {
            int a = maze[x][(y + 1) % h];
            int b = maze[x][(y - 1 + h) % h];
            int c = maze[(x + 1) % w][y];
            int d = maze[(x - 1 + w) % w][y];
            if (std::min({a, b, c, d}) == EMPTY) {
                setMaze(maze, x, y, EMPTY, config, w, h);
            }
        };
        
        std::uniform_int_distribution<int> xDist(0, w * 0.5 - hBdry * 2 - 1);
        std::uniform_int_distribution<int> yDist(0, h * 0.5 - vBdry * 2 - 1);
        
        for (int i = std::ceil(imperfect * w * h / 3.0f); i > 0; --i) {
            remove(xDist(rng) * 2 + 1, yDist(rng) * 2 + vBdry * 2);
            remove(xDist(rng) * 2 + hBdry * 2, yDist(rng) * 2 + 1);
        }
    }
    
    // Add rooms if requested
    if (config.roomsFraction > 0) {
        addRooms(maze, deadEnds, config);
    }
    
    return maze;
}

bool MazeGenerator::exportToCSV(const std::vector<std::vector<int>>& maze, const std::string& filename) {
    std::string fullPath = "assets/maps/" + filename;
    std::ofstream file(fullPath);
    
    if (!file.is_open()) {
        std::cout << "Error: Could not create CSV file: " << fullPath << std::endl;
        return false;
    }
    
    int width = maze.size();
    int height = maze.empty() ? 0 : maze[0].size();
    
    // Write dimensions
    file << width << "," << height << "\n";
    
    // Write tile data (row by row)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x > 0) file << ",";
            file << convertTileValue(maze[x][y]);
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Exported maze to: " << fullPath << " (" << width << "x" << height << ")" << std::endl;
    return true;
}

int MazeGenerator::convertTileValue(int mazeValue) {
    switch (mazeValue) {
        case EMPTY: return 1;      // Floor
        case SOLID: return 2;      // Brick wall
        case RESERVED: return 2;   // Also brick wall
        default: return 1;         // Default to floor
    }
}

void MazeGenerator::shuffle(std::vector<Direction>& directions, std::mt19937& rng) {
    for (int i = directions.size() - 1; i > 0; --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        std::swap(directions[i], directions[j]);
    }
}

bool MazeGenerator::unexplored(const std::vector<std::vector<int>>& maze, int x, int y, int ignoreReserved) {
    int c = maze[x][y];
    return (c == SOLID) || ((c == RESERVED) && (ignoreReserved > 0));
}

void MazeGenerator::setMaze(std::vector<std::vector<int>>& maze, int x, int y, int value, 
                           const MazeConfig& config, int w, int h) {
    x = (x + w) % w;
    y = (y + h) % h;
    
    maze[x][y] = value;
    
    const bool hSymmetry = config.horizontal.symmetry;
    const bool vSymmetry = config.vertical.symmetry;
    const bool hWrap = config.horizontal.loop && !(hSymmetry && config.horizontal.border);
    const bool vWrap = config.vertical.loop && !(vSymmetry && config.vertical.border);
    
    const int hBorderOffset = hWrap ? 0 : 1;
    const int vBorderOffset = vWrap ? 0 : 1;
    
    const int u = w - x - hBorderOffset;
    const int v = h - y - vBorderOffset;
    
    if (hSymmetry && u < w) {
        maze[u][y] = value;
        if (vSymmetry && v < h) {
            maze[u][v] = value;
        }
    }
    
    if (vSymmetry && v < h) {
        maze[x][v] = value;
    }
}

void MazeGenerator::addRooms(std::vector<std::vector<int>>& maze, const std::vector<DeadEnd>& deadEnds,
                            const MazeConfig& config) {
    int w = maze.size();
    int h = maze[0].size();
    
    float roomsFraction = std::max(0.0f, std::min(1.0f, config.roomsFraction));
    
    int a = std::ceil(0.6f * (config.hallWidth + config.wallWidth) / std::max(roomsFraction, 0.4f));
    int b = a;
    
    int last = std::floor((deadEnds.size() - 1) * roomsFraction);
    for (int i = last; i >= 0; --i) {
        const DeadEnd& c = deadEnds[i];
        
        int u = std::floor(c.x - a / 2.0f);
        int v = std::floor(c.y - b / 2.0f);
        
        for (int x = std::max(config.wallWidth, u - a); 
             x <= std::min(w - config.wallWidth - 1, u + a); ++x) {
            for (int y = std::max(config.wallWidth, v - b); 
                 y <= std::min(h - config.wallWidth, v + b); ++y) {
                maze[x][y] = EMPTY;
            }
        }
    }
    
    // Restore symmetry after adding rooms
    if (config.horizontal.symmetry) {
        int offset = config.horizontal.loop ? config.hallWidth + 1 : 1;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x <= w / 2; ++x) {
                maze[w - offset - x][y] = maze[x][y];
            }
        }
    }
    
    if (config.vertical.symmetry) {
        int offset = config.vertical.loop ? config.hallWidth + 1 : 1;
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y <= h / 2; ++y) {
                maze[x][h - offset - y] = maze[x][y];
            }
        }
    }
}