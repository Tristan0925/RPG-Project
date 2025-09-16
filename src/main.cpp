/*
Entry point for the RPG game.
Sets up the game window, handles the main loop, and manages player movement via camera.

-- This version simulates a first-person view by moving the camera instead of a visible player sprite.

*/

// sf is the SFML version of a namespace, std being the standard one. Namespaces are basically like containers that hold variables, functions, and the like. Kinda similar to modules in python (i think).

#include <SFML/Graphics.hpp> 
#include <SFML/System.hpp>
#include <cmath>
#include "Player.hpp"
#include "Map.hpp"


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "RPG Starter"); // create a window
    window.setFramerateLimit(60); // set frame rate

    // declare entites
    Map map; 

    if (!map.loadFromFile("assets/map1.txt")) {
        return -1; // failed to load
    }

    // spawnPos from map
    sf::Vector2f spawn(map.getSpawnX(), map.getSpawnY());

    // create player at that location
    Player player(spawn);
    
    sf::Clock clock; // frame timing, in accordance with delta

    // Camera rotation angles
    const float PI = 3.14159f;
    const float FOV = PI / 3.0f; // 60 degree feild of view
    // Screen size for calculations
    const int screenWidth = 800;
    const int screenHeight = 600;

    // Main game loop, runs until the window is closed
    while (window.isOpen()) {
        sf::Event event;

        // Poll and handle events (like closing the window)
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close(); // Close the window 
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) 
                window.close(); // Close the window             
        }

        float dt = clock.restart().asSeconds(); // dt = seconds since last frame (makes speed frame dependent)
        float moveSpeed = 100.f * dt;   // movement speed
        float rotSpeed  = 2.0f * dt;    // rotation speed

        // handle imput

        // Foward movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            player.moveForward(moveSpeed, map); // collisions later
        }

        // Backward movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            player.moveBackward(moveSpeed, map);
        }

        // Rotate left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            player.rotate(-rotSpeed);
        }

        // Rotate right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            player.rotate(rotSpeed);
        }

        window.clear(sf::Color::Black);

        // Draw ceiling
        sf::RectangleShape ceiling(sf::Vector2f(800, 300));
        ceiling.setFillColor(sf::Color(30, 30, 30));
        ceiling.setPosition(0, 0);
        window.draw(ceiling);
        
        // Draw floor
        sf::RectangleShape floor(sf::Vector2f(800, 300));
        floor.setFillColor(sf::Color(50, 50, 50));
        floor.setPosition(0, 300);
        window.draw(floor);

        // Draw 3D world using raycasting
        for (int x = 0; x < screenWidth; x++) {
            // Calculate ray angle relative to player's facing
            float rayAngle = (player.getAngle() - FOV / 2.0f) + ((float)x / screenWidth) * FOV; // this means leftmost ray = left edge of vision, rightmost ray = right edge
            
            // Ray position and direction
            float rayX = player.getPosition().x / 64.f; // scale to grid size (64px per cell)
            float rayY = player.getPosition().y / 64.f;
            float rayDirX = cos(rayAngle); // this gives you a unit vector pointing in the direction the ray should travel
            float rayDirY = sin(rayAngle); // Example: if angle = 0, direction = (1, 0) → to the right.
            
            // DDA setup
            /*
            DDA = Digital Differential Analyzer - a step-by-step way to move through the grid.

            - You start from the player’s grid position.

            - At each step, decide if the ray crosses into the next grid cell in X or Y.

            - Keep stepping until you hit a wall.
            */
            int mapX = int(rayX);
            int mapY = int(rayY);
            
            float sideDistX;
            float sideDistY;
            
            float deltaDistX = (rayDirX == 0) ? 1e30f : fabs(1 / rayDirX);
            float deltaDistY = (rayDirY == 0) ? 1e30f : fabs(1 / rayDirY);
            float perpWallDist;
            
            int stepX, stepY;
            int hit = 0;
            int side;
            
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (rayX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0f - rayX) * deltaDistX;
            }
            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (rayY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0f - rayY) * deltaDistY;
            }
            
            // DDA loop - ray marching
            while (hit == 0) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                if (map.isWall(mapX, mapY)) hit = 1;
            }
            
            if (side == 0) // Once a wall is hit, you compute the distance from the player to the wall
                perpWallDist = (sideDistX - deltaDistX);
            else
                perpWallDist = (sideDistY - deltaDistY);
            
            // Wall height
            // Near wall = small denominator = tall line; Far wall = big denominator = short line.
            int lineHeight = (int)(screenHeight / perpWallDist);
            
            int drawStart = -lineHeight / 2 + screenHeight / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + screenHeight / 2;
            if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;
            
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, drawStart), side == 1 ? sf::Color(180,180,180) : sf::Color::White),
                sf::Vertex(sf::Vector2f(x, drawEnd),   side == 1 ? sf::Color(180,180,180) : sf::Color::White)
                };
                window.draw(line, 2, sf::Lines);
            }

        // DRaw world here 
        // Draw minimap last (so it overlays)
        map.renderMiniMap(window, player.getPosition(), player.getAngle());
        
        window.display();
    }

    return 0; // Exit the program
}
