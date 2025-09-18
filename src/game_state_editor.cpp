//This is the main game file

#include <SFML/Graphics.hpp>
#include <cmath>
#include "game_state.hpp"
#include "game_state_editor.hpp"

void GameStateEditor::draw(const float dt) //If you draw things, put them here
{
    
    // Camera rotation angles
    const float PI = 3.14159f;
    const float FOV = PI / 2.0f;  // 90 degree field of view

    // Screen size for calculations
    const int screenWidth = 1920;
    const int screenHeight = 1080;

     sf::RectangleShape ceiling(sf::Vector2f(800, 300));
        ceiling.setFillColor(sf::Color(30, 30, 30));
        ceiling.setPosition(0, 0);
        this->game->window.draw(ceiling);
        
        // Draw floor
        sf::RectangleShape floor(sf::Vector2f(800, 300));
        floor.setFillColor(sf::Color(50, 50, 50));
        floor.setPosition(0, 300);
        this->game->window.draw(floor);

        // Draw 3D world using raycasting
        for (int x = 0; x < screenWidth; x++) {
            // Calculate ray angle relative to player's facing
            float rayAngle = (this->game->player.getAngle() - FOV / 2.0f) + ((float)x / screenWidth) * FOV; // this means leftmost ray = left edge of vision, rightmost ray = right edge
            
            // Ray position and direction
            float rayX = this->game->player.getPosition().x / 64.f; // scale to grid size (64px per cell)
            float rayY = this->game->player.getPosition().y / 64.f;
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
                if (this->game->map.isWall(mapX, mapY)) hit = 1;
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
                 this->game->window.draw(line, 2, sf::Lines);
            }

        // DRaw world here 
        // Draw minimap last (so it overlays)
        this->game->map.renderMiniMap( this->game->window, this->game->player.getPosition(), this->game->player.getAngle());
        
    return;
}

void GameStateEditor::update(const float dt) //If something needs to be updated based on dt, then go here
{

   moveSpeed = 100.f * dt;   // movement speed
  this->game->player.update(dt);
    return;
}

void GameStateEditor::handleInput() //Inputs go here
{
    sf::Event event;
 
    while(this->game->window.pollEvent(event))
    {
        switch(event.type)
        {
            /* Close the window */
            case sf::Event::Closed:
            {
                this->game->window.close();
                break;
            }
            /* Resize the window */
            case sf::Event::Resized:
            {
                gameView.setSize(event.size.width, event.size.height);
                guiView.setSize(event.size.width, event.size.height);
                this->game->background.setPosition(this->game->window.mapPixelToCoords(sf::Vector2i(0, 0), this->guiView));
                this->game->background.setScale(
                    float(event.size.width) / float(this->game->background.getTexture()->getSize().x),
                    float(event.size.height) / float(this->game->background.getTexture()->getSize().y));
                break;
            }
            default: break;
        }
    }
    
        // Foward movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            this->game->player.moveForward(moveSpeed, this->game->map); // collisions later
        }

        // Backward movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            this->game->player.moveBackward(moveSpeed, this->game->map);
        }

        
        static bool leftPressed = false;
        static bool rightPressed = false;
        
        // Turn left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            if (!leftPressed) {
                this->game->player.turnLeft();
                leftPressed = true;
            }
        } else {
            leftPressed = false;
        }

        // Turn right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            if (!rightPressed) {
             this->game->player.turnRight();
                rightPressed = true;
            }
        } else {
            rightPressed = false;
        }
                
 
    return;
}

GameStateEditor::GameStateEditor(Game* game) //This is a constructor
{

    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->guiView.setSize(pos);
    this->gameView.setSize(pos);
    pos *= 0.5f;
    this->guiView.setCenter(pos);
    this->gameView.setCenter(pos);

    moveSpeed = 0.f;
}