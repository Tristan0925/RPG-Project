/*

It’s essentially the world manager — it knows what the world looks like and can help both the game logic (checking walls) and the visuals (drawing minimap).

*/


#include "Map.hpp" // includes map.hpp so we can use its functions
#include <SFML/Graphics.hpp>   
#include <cmath> 

Map::Map() {
    width = 10;
    height = 10;

    // Fill with empty spaces
    grid = std::vector<std::vector<int>>(height, std::vector<int>(width, 0)); // makes an empty room (no walls)

    // loops to build the outer walls - recall 0 is empty space and 1 is a wall
    for (int x = 0; x < width; ++x) {
        grid[0][x] = 1;          // top
        grid[height - 1][x] = 1; // bottom
    }
    for (int y = 0; y < height; ++y) {
        grid[y][0] = 1;          // left
        grid[y][width - 1] = 1;  // right
    }

    // added a wall in the middle of the space - testing
    for (int y = 2; y < height - 2; ++y) {
        grid[y][5] = 1;
    }
}

bool Map::isWall(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return true; // outside bounds counts as wall - prevents player from going out of bounds... hopefully
    }
    return grid[y][x] == 1;
}

void Map::renderMiniMap(sf::RenderWindow& window, const sf::Vector2f& playerPos, float playerAngle) const {
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

    // Draw player as a circle
    sf::CircleShape playerDot(5.f);
    playerDot.setFillColor(sf::Color::Red);
    playerDot.setOrigin(5.f, 5.f); // should set player in middle but currently not working
    playerDot.setPosition(offset.x + playerPos.x * scale / 40.f, 
    offset.y + playerPos.y * scale / 40.f);
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
