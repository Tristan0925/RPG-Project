/*

It’s essentially the world manager — it knows what the world looks like and can help both the game logic (checking walls) and the visuals (drawing minimap).

*/


#include "Map.hpp" // includes map.hpp so we can use its functions
#include <SFML/Graphics.hpp>   
#include <cmath> 
#include <fstream>
#include <iostream>

Map::Map() {
    width = 0; // set to empty so we can fill the empty space with the txt file 
    height = 0;
}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename); // used for reading files
    if (!file.is_open()) {
        std::cerr << "Error: Could not open map file: " << filename << std::endl;
        return false;
    }
    grid.clear(); // preaparing the grid
    spawnX = spawnY = -1.0f; // reset spawn

    std::string line;
    int y = 0; // row index
    while (std::getline(file,line)) {
        std::vector<int> row;
        for (int x = 0; x < static_cast<int>(line.size()); x++) {
            char c = line[x];
            if (c == '1') row.push_back(1); // wall
            else if (c == 'P') {
                row.push_back(0); // it's floor, not wall
                spawnX = static_cast<float>(x);
                spawnY = static_cast<float>(y);
            }
            else row.push_back(0);          // empty
        }
        grid.push_back(row);
        y++; // move to next row
    }
    height = grid.size();
    width = (height > 0) ? grid[0].size() : 0; // avoids accessing grid[0] if the file was empty

    if (spawnX < 0 || spawnY < 0) {
        spawnX = 1.0f;
        spawnY = 1.0f;
    }

    file.close();
    return true;
}



bool Map::isWall(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return true; // outside bounds counts as wall - prevents player from going out of bounds... hopefully
    }
    return grid[y][x] == 1;
}

void Map::renderMiniMap(sf::RenderWindow& window, const sf::View& miniMapView, const sf::Vector2f& playerPos, float playerAngle) const {
    window.setView(miniMapView);

    const float TILE_SIZE = 64.f;
    sf::RectangleShape tileShape(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    tileShape.setFillColor(sf::Color::White);

    // draw walls
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (isWall(x, y)) {
                tileShape.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(tileShape);
            }
        }
    }
    
    // Draw player arrow
    sf::ConvexShape playerArrow;
    playerArrow.setPointCount(3);

    float size = 16.f; // size of arrow

    // Define a triangle pointing up by default
    playerArrow.setPoint(0, sf::Vector2f(size, 0.f));      // tip
    playerArrow.setPoint(1, sf::Vector2f(-size/2, -size/2));  // bottom left
    playerArrow.setPoint(2, sf::Vector2f(-size/2, size/2));   // bottom right

    playerArrow.setFillColor(sf::Color::Red);
    playerArrow.setOrigin(0.f, 0.f);      // origin at tip
    playerArrow.setPosition(playerPos);
    playerArrow.setRotation(playerAngle * 180.f / 3.14159f); // rotate to player facing

    window.draw(playerArrow);

}