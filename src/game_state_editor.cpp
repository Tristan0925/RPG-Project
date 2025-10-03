//This is the main game file

#include <SFML/Graphics.hpp>
#include <cmath>
#include "game_state.hpp"
#include "game_state_editor.hpp"
#include <stdio.h>

void GameStateEditor::draw(const float dt) //If you draw things, put them here
{
    
    // Camera rotation angles
    const float PI = 3.14159f;
    const float FOV = PI / 3.0f;  // 90 degree field of view

    // Screen size for calculations
    const int screenWidth = 1920;
    const int screenHeight = 1080;

     sf::RectangleShape ceiling(sf::Vector2f(1920, 1080));
        ceiling.setFillColor(sf::Color(250, 250, 0));
        ceiling.setPosition(0, -450);
        this->game->window.draw(ceiling);
        
        // Draw floor
        sf::RectangleShape floory(sf::Vector2f(1920, 1080));
        floory.setFillColor(sf::Color(129,133,137));
        floory.setPosition(0, 550);
        this->game->window.draw(floory);

        // Draw 3D world using raycasting
        for (int x = 0; x < screenWidth; x++) {                       //for every pixel of the screen's width, make a vertical column
            // Calculate ray angle relative to player's facing. This shows what the player is looking at
            float rayAngle = (this->game->player.getAngle() - FOV / 2.0f) + ((float)x / screenWidth) * FOV; // this means leftmost ray = left edge of vision, rightmost ray = right edge
          






            
            // Ray position and direction
            float rayX = this->game->player.getPosition().x / 64.f; // scale to grid size (64px per cell)
            float rayY = this->game->player.getPosition().y / 64.f;
            float rayDirX = cos(rayAngle); // this gives you a unit vector pointing in the direction the ray should travel
            float rayDirY = sin(rayAngle); // Example: if angle = 0, direction = (1, 0) → to the right.
            //Above converts pixels to the map units, essentially dividing the map into separate cells.


            // DDA setup
            /*
            DDA = Digital Differential Analyzer - a step-by-step way to move through the grid.

            - You start from the player’s grid position.

            - At each step, decide if the ray crosses into the next grid cell in X or Y.

            - Keep stepping until you hit a wall.
            */
            int mapX = int(rayX); //get player position
            int mapY = int(rayY);
            
            float sideDistX; 
            float sideDistY;
            float wallX;
            
            float deltaDistX = (rayDirX == 0) ? 1e30f : fabs(1 / rayDirX);
            float deltaDistY = (rayDirY == 0) ? 1e30f : fabs(1 / rayDirY); 
            //This figures out the distance needed to travel between cells
            float perpWallDist;
            float rawDist;
            
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
            } //Send rays until they hit a wall
            
            float playerAngle = this->game->player.getAngle();

            if (side == 0){ // Once a wall is hit, you compute the distance from the player to the wall
                rawDist = sideDistX - deltaDistX;
                perpWallDist = rawDist * cos(rayAngle - playerAngle);
                wallX = rayY + rawDist * rayDirY;}
            else{
                rawDist = sideDistY - deltaDistY;
                perpWallDist = rawDist * cos(rayAngle - playerAngle);
                wallX = rayX + rawDist * rayDirX;}
            
            printf("playerAngle: %f, rayAngle: %f, angleDiff: %f\n", this->game->player.getAngle(), rayAngle, rayAngle - this->game->player.getAngle());
printf("rawDist: %f, perpWallDist: %f\n", rawDist, perpWallDist);
            // Wall height
            // Near wall = small denominator = tall line; Far wall = big denominator = short line.
            int lineHeight = (int)(screenHeight / perpWallDist);
            
            int drawStart = -lineHeight / 2 + screenHeight / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + screenHeight / 2;
            if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;
        wallX -= floor(wallX); //get fractional part of wallX
        int texX = int(wallX * (this->game->texmgr.getRef("level1walls").getSize().x));
        
        if ((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0)){
            texX = this->game->texmgr.getRef("level1walls").getSize().x - texX - 1;
        }

   

    // Create a quad for this vertical slice
    sf::Vertex quad[4];

    float texXf = static_cast<float>(texX);
    float texXfNext = texXf + 1.0f; // one pixel width on texture

    quad[0].position = sf::Vector2f(x, drawStart);
    quad[1].position = sf::Vector2f(x + 1, drawStart);
    quad[2].position = sf::Vector2f(x + 1, drawEnd);
    quad[3].position = sf::Vector2f(x, drawEnd);

    
    quad[0].texCoords = sf::Vector2f(texXf, 0);
    quad[1].texCoords = sf::Vector2f(texXfNext, 0);
    quad[2].texCoords = sf::Vector2f(texXfNext, static_cast<float>(this->game->texmgr.getRef("level1walls").getSize().y));
    quad[3].texCoords = sf::Vector2f(texXf, static_cast<float>(this->game->texmgr.getRef("level1walls").getSize().y));

    // Optionally shade the quad for side walls
    if (side == 1) {
        for (int i = 0; i < 4; i++) {
            quad[i].color = sf::Color(250, 250, 250);
        }
    } else {
        for (int i = 0; i < 4; i++) {
            quad[i].color = sf::Color::White;
        }
    }

    this->game->window.draw(quad, 4, sf::Quads, &this->game->texmgr.getRef("level1walls"));


            }

        // Setup minimap
        float mapSizePx = 200.f;
        float padding = 10.f;

        // Get window size
        float winW = (float)this->game->window.getSize().x;
        float winH = (float)this->game->window.getSize().y;
        sf::Vector2f minimapCenterPos = this->game->player.getPosition();

        // Minimap view
        sf::View minimapView;
        minimapView.setSize(400.f, 400.f);
        minimapView.setCenter(minimapCenterPos);

        // Initial rotation offset (since the player spawns looking east)
        float initialRotationDeg = 90.f;
        float playerAngleDeg = this->game->player.getAngle() * 180.f / 3.14159f;
        minimapView.setRotation(initialRotationDeg - playerAngleDeg);

        // Set viewport 
        sf::FloatRect vp(
            padding / winW,                    // left
            (winH - mapSizePx - padding) / winH, // top (distance from top of window)
            mapSizePx / winW,                  // width
            mapSizePx / winH                   // height
        );
        minimapView.setViewport(vp);

        // Draw minimap border
        sf::RectangleShape minimapBorder(sf::Vector2f(mapSizePx, mapSizePx));
        minimapBorder.setOrigin(mapSizePx/2, mapSizePx/2); // rotate around center
        minimapBorder.setPosition(padding + mapSizePx/2, winH - mapSizePx/2 - padding);
        minimapBorder.setFillColor(sf::Color(0,0,0,180));
        minimapBorder.setOutlineThickness(2.f);
        minimapBorder.setOutlineColor(sf::Color::White);
        minimapBorder.setRotation(initialRotationDeg - playerAngleDeg); // same rotation as minimap
        this->game->window.draw(minimapBorder);

        // Render the actual minimap
        this->game->map.renderMiniMap( this->game->window, minimapView, this->game->player.getPosition(), this->game->player.getAngle());

        // Draw player icon on minimap
        sf::CircleShape playerIcon(6, 3); // triangle with radius 6
        playerIcon.setOrigin(6, 6);       // rotate around center
        playerIcon.setPosition(this->game->player.getPosition());
        playerIcon.setFillColor(sf::Color::Red);

        // Player icon rotation relative to rotated minimap
        float totalRotationDeg = playerAngleDeg - initialRotationDeg;
        playerIcon.setRotation(totalRotationDeg);

        // Draw player icon in minimap view
        this->game->window.setView(minimapView);
        this->game->window.draw(playerIcon);

        // Reset view for HUD / other UI ---
        this->game->window.setView(this->game->window.getDefaultView());



        // draw HUD elements here
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