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

    // --- New rendering parameters ---
    const int rayCount = 1920;                    // fewer rays = big performance boost
    const float columnWidth = (float)screenWidth / rayCount;

    // Ceiling
    sf::RectangleShape ceiling(sf::Vector2f(screenWidth, screenHeight / 2));
    ceiling.setFillColor(sf::Color(30, 30, 30));
    ceiling.setPosition(0, 0);
    this->game->window.draw(ceiling);
        
    // Floor
    sf::RectangleShape floor(sf::Vector2f(screenWidth, screenHeight / 2));
    floor.setFillColor(sf::Color(50, 50, 50));
    floor.setPosition(0, screenHeight / 2);
    this->game->window.draw(floor);

    // Draw 3D world using raycasting
    for (int i = 0; i < rayCount; i++) {
        // Calculate ray angle relative to player's facing
        float rayAngle = (this->game->player.getAngle() - FOV / 2.0f) + ((float)i / rayCount) * FOV; 
        
        // Ray position and direction
        float rayX = this->game->player.getPosition().x / 64.f;
        float rayY = this->game->player.getPosition().y / 64.f;
        float rayDirX = cos(rayAngle);
        float rayDirY = sin(rayAngle);
        
        // DDA setup
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
        
        if (side == 0)
            perpWallDist = (sideDistX - deltaDistX) * cos(rayAngle - this->game->player.getAngle());
        else
            perpWallDist = (sideDistY - deltaDistY) * cos(rayAngle - this->game->player.getAngle());
        
        // Calculate where exactly the ray hit the wall (for texture sampling)
        float wallX;
        if (side == 0)
            wallX = this->game->player.getPosition().y / 64.f + perpWallDist * rayDirY;
        else
            wallX = this->game->player.getPosition().x / 64.f + perpWallDist * rayDirX;
        wallX -= std::floor(wallX);

        // texture column (X) in the wall image
        int texX = int(wallX * float(textureWidth));
        if (side == 0 && rayDirX > 0) texX = textureWidth - texX - 1;
        if (side == 1 && rayDirY < 0) texX = textureWidth - texX - 1;

        // Wall height
        int lineHeight = (int)(screenHeight / perpWallDist);
        int drawStart = -lineHeight / 2 + screenHeight / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + screenHeight / 2;
        if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

        // --- Draw the wall slice as a textured rectangle ---
        sf::RectangleShape wallColumn(sf::Vector2f(columnWidth + 1, drawEnd - drawStart));

        // Pick color from the texture at texX (sample mid Y for simplicity)
        int texY = textureHeight / 2;
        sf::Color color = wallImage.getPixel(texX, texY);

        // Make darker if side == 1 (for lighting)
        if (side == 1) {
            color.r *= 0.7;
            color.g *= 0.7;
            color.b *= 0.7;
        }

        wallColumn.setPosition(i * columnWidth, drawStart);
        wallColumn.setFillColor(color);
        this->game->window.draw(wallColumn);
    }

    // --- Minimap and HUD section (unchanged) ---
    float mapSizePx = 200.f;
    float padding = 10.f;

    float winW = (float)this->game->window.getSize().x;
    float winH = (float)this->game->window.getSize().y;
    sf::Vector2f minimapCenterPos = this->game->player.getPosition();

    sf::View minimapView;
    minimapView.setSize(400.f, 400.f);
    minimapView.setCenter(minimapCenterPos);

    float initialRotationDeg = 90.f;
    float playerAngleDeg = this->game->player.getAngle() * 180.f / 3.14159f;
    minimapView.setRotation(initialRotationDeg - playerAngleDeg);

    sf::FloatRect vp(
        padding / winW,
        (winH - mapSizePx - padding) / winH,
        mapSizePx / winW,
        mapSizePx / winH
    );
    minimapView.setViewport(vp);

    sf::RectangleShape minimapBorder(sf::Vector2f(mapSizePx, mapSizePx));
    minimapBorder.setOrigin(mapSizePx / 2, mapSizePx / 2);
    minimapBorder.setPosition(padding + mapSizePx / 2, winH - mapSizePx / 2 - padding);
    minimapBorder.setFillColor(sf::Color(0, 0, 0, 180));
    minimapBorder.setOutlineThickness(2.f);
    minimapBorder.setOutlineColor(sf::Color::White);
    minimapBorder.setRotation(initialRotationDeg - playerAngleDeg);
    this->game->window.draw(minimapBorder);

    this->game->map.renderMiniMap(this->game->window, minimapView, this->game->player.getPosition(), this->game->player.getAngle());

    sf::CircleShape playerIcon(6, 3);
    playerIcon.setOrigin(6, 6);
    playerIcon.setPosition(this->game->player.getPosition());
    playerIcon.setFillColor(sf::Color::Red);
    float totalRotationDeg = playerAngleDeg - initialRotationDeg;
    playerIcon.setRotation(totalRotationDeg);
    this->game->window.setView(minimapView);
    this->game->window.draw(playerIcon);

    this->game->window.setView(this->game->window.getDefaultView());
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
 
    while (this->game->window.pollEvent(event))
    {
        switch (event.type)
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
        this->game->player.moveForward(moveSpeed, this->game->map);
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

    // Load texture once
    wallTexture.loadFromFile("assets/wall_texture.jpg");
    wallImage = wallTexture.copyToImage(); // for pixel-level access
    textureWidth = wallImage.getSize().x;
    textureHeight = wallImage.getSize().y;
}
