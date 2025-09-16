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

void Map::renderMiniMap(sf::RenderWindow& window, const sf::Vector2f& playerPos, float playerAngle) const {
    const float TILE_SIZE = 64.f;
    const float scale = 20.f; // pixels per grid tile on minimap
    const sf::Vector2f offset(20.f, 400.f); // bottom-left corner

    // draw walls from grid - Loops through the grid and if a tile is a wall, draws a gray rectangle at the right spot on the minimap.
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] == 1) { // wall
                sf::RectangleShape miniWall;
                miniWall.setSize(sf::Vector2f(scale, scale));
                miniWall.setFillColor(sf::Color(200, 200, 200));
                miniWall.setPosition(offset.x + x * scale, offset.y + y * scale);
                window.draw(miniWall);
            }
        }
    }

    // convert player world pos -> minimap pos
    float miniX = offset.x + (playerPos.x / TILE_SIZE) * scale;
    float miniY = offset.y + (playerPos.y / TILE_SIZE) * scale;

    // Draw player as a circle
    sf::CircleShape playerDot(5.f);
    playerDot.setFillColor(sf::Color::Red);
    playerDot.setOrigin(5.f, 5.f); // should set player in middle but currently not working
    playerDot.setPosition(miniX, miniY);
    window.draw(playerDot);

    // Draw facing direction
    sf::VertexArray dirLine(sf::Lines, 2);
    dirLine[0].position = playerDot.getPosition();
    dirLine[0].color = sf::Color::Red;
    dirLine[1].position = playerDot.getPosition() + 
    sf::Vector2f(std::cos(playerAngle), std::sin(playerAngle)) * 20.f;
    dirLine[1].color = sf::Color::Red;
    window.draw(dirLine);
}
