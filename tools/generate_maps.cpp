#include "../src/maze_generator.h"
#include <iostream>
#include <string>

void generateSampleMaps() {
    std::cout << "Generating sample maze maps..." << std::endl;
    
    // Classic maze
    MazeConfig classic;
    classic.horizontal.border = 1;
    classic.vertical.border = 1;
    classic.straightness = 0.3f;
    classic.fill = 0.8f;
    auto classicMaze = MazeGenerator::generate(50, 30, classic);
    MazeGenerator::exportToCSV(classicMaze, "generated_classic.csv");
    
    // Symmetric maze
    MazeConfig symmetric;
    symmetric.horizontal.symmetry = true;
    symmetric.vertical.symmetry = true;
    symmetric.horizontal.border = 1;
    symmetric.vertical.border = 1;
    symmetric.straightness = 0.5f;
    symmetric.fill = 0.9f;
    auto symmetricMaze = MazeGenerator::generate(50, 30, symmetric);
    MazeGenerator::exportToCSV(symmetricMaze, "generated_symmetric.csv");
    
    // Loopy maze with rooms
    MazeConfig loopy;
    loopy.horizontal.border = 1;
    loopy.vertical.border = 1;
    loopy.imperfect = 0.3f;
    loopy.fill = 0.6f;
    loopy.roomsFraction = 0.4f;
    loopy.straightness = 0.1f;
    auto loopyMaze = MazeGenerator::generate(50, 30, loopy);
    MazeGenerator::exportToCSV(loopyMaze, "generated_loopy.csv");
    
    // Dense maze
    MazeConfig dense;
    dense.horizontal.border = 1;
    dense.vertical.border = 1;
    dense.fill = 1.0f;
    dense.straightness = 0.8f;
    auto denseMaze = MazeGenerator::generate(50, 30, dense);
    MazeGenerator::exportToCSV(denseMaze, "generated_dense.csv");
    
    std::cout << "Generated 4 sample maps in assets/maps/" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string command = argv[1];
        
        if (command == "samples") {
            generateSampleMaps();
            return 0;
        }
        
        if (command == "custom" && argc >= 5) {
            int width = std::stoi(argv[2]);
            int height = std::stoi(argv[3]);
            std::string filename = argv[4];
            
            MazeConfig config;
            if (argc > 5) config.straightness = std::stof(argv[5]);
            if (argc > 6) config.imperfect = std::stof(argv[6]);
            if (argc > 7) config.fill = std::stof(argv[7]);
            if (argc > 8) config.roomsFraction = std::stof(argv[8]);
            
            auto maze = MazeGenerator::generate(width, height, config);
            MazeGenerator::exportToCSV(maze, filename);
            return 0;
        }
    }
    
    std::cout << "Maze Generator Tool" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << argv[0] << " samples" << std::endl;
    std::cout << "    Generate 4 sample maps with different configurations" << std::endl;
    std::cout << "  " << argv[0] << " custom <width> <height> <filename.csv> [straightness] [imperfect] [fill] [rooms]" << std::endl;
    std::cout << "    Generate custom maze with specified parameters" << std::endl;
    std::cout << "Parameters (0.0-1.0):" << std::endl;
    std::cout << "  straightness: How straight corridors are (default 0.0)" << std::endl;
    std::cout << "  imperfect: Add loops/cycles (default 0.0)" << std::endl;
    std::cout << "  fill: Maze density (default 1.0)" << std::endl;
    std::cout << "  rooms: Add rooms at dead ends (default 0.0)" << std::endl;
    
    return 1;
}