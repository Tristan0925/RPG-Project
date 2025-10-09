//This is the main game file

#include <SFML/Graphics.hpp>
#include <cmath>
#include "game_state.hpp"
#include "game_state_editor.hpp"
#include "iostream"
#include "algorithm"

void GameStateEditor::draw(const float dt) //If you draw things, put them here
{
    
    // Camera rotation angles
    const float PI = 3.14159f;
    const float FOV = PI / 2.0f;  // 90 degree field of view

    // Screen size for calculations
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    // --- New rendering parameters ---
    const int rayCount = 640; // fewer rays = big performance boost
    const float columnWidth = (float)screenWidth / rayCount;
    int mapWidth = this->game->map.getWidth();
    int mapHeight = this->game->map.getHeight();


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
        // Ray angle relative to player
        float rayAngle = this->game->player.getAngle() - FOV / 2.f + ((float)i / rayCount) * FOV;
    
        // Ray position in tile coordinates
        float rayX = this->game->player.getPosition().x / 64.f;
        float rayY = this->game->player.getPosition().y / 64.f;
    
        // Ray direction
        float rayDirX = cos(rayAngle);
        float rayDirY = sin(rayAngle);
    
        // DDA setup
        int mapX = (int)rayX;
        int mapY = (int)rayY;
        float sideDistX, sideDistY;
        float deltaDistX = (rayDirX == 0) ? 1e30f : fabs(1 / rayDirX);
        float deltaDistY = (rayDirY == 0) ? 1e30f : fabs(1 / rayDirY);
        int stepX = (rayDirX < 0) ? -1 : 1;
        int stepY = (rayDirY < 0) ? -1 : 1;
        sideDistX = (rayDirX < 0) ? (rayX - mapX) * deltaDistX : (mapX + 1.0f - rayX) * deltaDistX;
        sideDistY = (rayDirY < 0) ? (rayY - mapY) * deltaDistY : (mapY + 1.0f - rayY) * deltaDistY;
    
        int hit = 0;
        int side = 0;
    
        // DDA loop — march ray
        int maxSteps = std::max(mapWidth, mapHeight);
        // prevent infinite loops
        int stepsTaken = 0;
    
        while (hit == 0 && stepsTaken < maxSteps) {
            stepsTaken++;
    
            // Bounds check
            if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
                hit = 1;
                break;
            }
    
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
    
            if (this->game->map.isWall(mapX, mapY)) {
                hit = 1;
            }
        }
    
        // Calculate perpendicular wall distance (fish-eye correction)
        float perpWallDist = (side == 0)
            ? (sideDistX - deltaDistX)
            : (sideDistY - deltaDistY);

        float correctedAngle = rayAngle - this->game->player.getAngle();
        perpWallDist *= std::max(std::cos(correctedAngle), 0.0f);

        // Avoid division by zero for very close walls
        if (perpWallDist <= 0.0001f) continue;

        // Wall height in pixels
        int lineHeight = static_cast<int>(screenHeight / perpWallDist);
        int drawStart = -lineHeight / 2 + screenHeight / 2;
        int drawEnd = lineHeight / 2 + screenHeight / 2;
        if (drawStart < 0) drawStart = 0;
        if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;

        // Texture X coordinate
        float wallX;
        if (side == 0)
            wallX = rayY + perpWallDist * rayDirY;
        else
            wallX = rayX + perpWallDist * rayDirX;
        wallX -= std::floor(wallX);

        int texX = static_cast<int>(wallX * static_cast<float>(textureWidth));
        if (side == 0 && rayDirX > 0) texX = textureWidth - texX - 1;
        if (side == 1 && rayDirY < 0) texX = textureWidth - texX - 1;

        // Texture step per screen pixel (for mapping)
        float step = 1.0f * textureHeight / lineHeight;
        float texStart = (drawStart - screenHeight / 2 + lineHeight / 2) * step;
        float texEnd = texStart + (drawEnd - drawStart) * step;

        // Use a single quad for the wall column
        sf::VertexArray column(sf::Quads, 4);

        float x = i * columnWidth;
        column[0].position = sf::Vector2f(x, drawStart);
        column[1].position = sf::Vector2f(x + columnWidth, drawStart);
        column[2].position = sf::Vector2f(x + columnWidth, drawEnd);
        column[3].position = sf::Vector2f(x, drawEnd);

        // Correct vertical mapping (no stretching)
        column[0].texCoords = sf::Vector2f(texX, texStart);
        column[1].texCoords = sf::Vector2f(texX + 1, texStart);
        column[2].texCoords = sf::Vector2f(texX + 1, texEnd);
        column[3].texCoords = sf::Vector2f(texX, texEnd);

        // Shading for Y-sides
        sf::Color tint(255, 255, 255);
        if (side == 1) {
            tint.r = static_cast<sf::Uint8>(tint.r * 0.7f);
            tint.g = static_cast<sf::Uint8>(tint.g * 0.7f);
            tint.b = static_cast<sf::Uint8>(tint.b * 0.7f);
        }
        for (int v = 0; v < 4; ++v)
            column[v].color = tint;

        // Draw this vertical slice
        this->game->window.draw(column, &wallTexture);

    }

    // Minimap and HUD section
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
            // Close the window
            case sf::Event::Closed:
            {
                this->game->window.close();
                break;
            }
            // Resize the window 
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
    wallTexture.setSmooth(false);   // prevents blur
    wallTexture.setRepeated(true);
    wallTexture.generateMipmap();   // improves close-up detail
    wallImage = wallTexture.copyToImage(); // for pixel-level access
    textureWidth = wallImage.getSize().x;
    textureHeight = wallImage.getSize().y;
}
