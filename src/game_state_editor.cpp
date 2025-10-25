//This is the main game file

#include <SFML/Graphics.hpp>
#include <cmath>
#include "game_state.hpp"
#include "game_state_editor.hpp"
#include "iostream"
#include "algorithm"
#include "Button.hpp"
#include "game_state_start.hpp"

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
        
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
        
            // Check for wall hit
            if (this->game->map.isWall(mapX, mapY) || this->game->map.isDoor(mapX, mapY))
                hit = 1;
        
            // Bounds check
            if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
                hit = 1;
                break;
            }
        }
        
    
        // Calculate perpendicular wall distance (fish-eye correction)
        float perpWallDist = (side == 0)
            ? (sideDistX - deltaDistX)
            : (sideDistY - deltaDistY);

        
        // Get window size
       
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

        // Draw this vertical slice, tex is dependent on door or not door
     if (this->game->map.isDoor(mapX,mapY) == 0){
        this->game->window.draw(column, &wallTexture);
     }
     else this->game->window.draw(column, &doorTexture); 

    }
       // Initial rotation offset (since the player spawns looking East)
        float initialRotationDeg = 90.f;
        float playerAngleDeg = this->game->player.getAngle() * 180.f / 3.14159f;
       sf::View minimapView;

    // Setup minimap
        float mapSizePx = 200.f;
        float padding = 100.f;
        float vertOffset = 50.0f;
        float winW = (float)this->game->window.getSize().x;
        float winH = (float)this->game->window.getSize().y;
        sf::Vector2f minimapCenterPos = this->game->player.getPosition();
       
        // Set viewport 
        sf::FloatRect vp(
            padding / winW,                    // left
            (winH - mapSizePx - padding - vertOffset) / winH, // top (distance from top of window)
            mapSizePx / winW,                  // width
            mapSizePx / winH               // height
        );
        minimapView.setViewport(vp);
        minimapView.setRotation(initialRotationDeg - playerAngleDeg);
        minimapView.setCenter(minimapCenterPos);
        // Draw minimap border
        sf::RectangleShape minimapBorder(sf::Vector2f(mapSizePx, mapSizePx));
        minimapBorder.setOrigin(mapSizePx/2, mapSizePx/2); // rotate around center
        minimapBorder.setPosition(padding + mapSizePx/2, winH - mapSizePx/2 - padding - vertOffset);
        minimapBorder.setFillColor(sf::Color(0,0,0,180));
        minimapBorder.setOutlineThickness(2.f);
        minimapBorder.setOutlineColor(sf::Color::Red);
        minimapBorder.setRotation(initialRotationDeg - playerAngleDeg); // same rotation as minimap
        this->game->window.draw(minimapBorder);


        // draw HUD elements here

        //Background for locationText
        sf::RectangleShape locationBackground(sf::Vector2f(250.0f,45.0f));
        locationBackground.setFillColor(sf::Color(0,0,0,100));
        locationBackground.setPosition(15.0f, 100.0f);
        locationBackground.setOutlineColor(sf::Color::Red);
        locationBackground.setOutlineThickness(3.0f);

        //locationText
        sf::Text locationText;
        locationText.setFont(this->game->font);
        locationText.setString("Spooky Scary Dungeon 1F");
        locationText.setCharacterSize(24);
        locationText.setPosition(60.5f, 110.0f);
        this->game->window.draw(locationBackground);
        this->game->window.draw(locationText);

        //North Indicator for minimap
         sf::ConvexShape triangleN;
       
       
         triangleN.setPointCount(3);
         triangleN.setPoint(0,sf::Vector2f(mapSizePx/2.0f - 50.0f,-3.0f));
         triangleN.setPoint(1, sf::Vector2f(mapSizePx/2.0f, - 50.0f));
         triangleN.setPoint(2, sf::Vector2f(mapSizePx/2.0f + 50.0f, -3.0f));  

         triangleN.setFillColor(sf::Color(0,0,0,230));
         triangleN.setOutlineColor(sf::Color::Red);
         triangleN.setOutlineThickness(3.0f);
         triangleN.setRotation(-90.0f + playerAngleDeg);
         triangleN.setOrigin(mapSizePx/2, mapSizePx/2);
         triangleN.setPosition(padding + mapSizePx/2, winH - mapSizePx/2 - padding - vertOffset);


         this->game->window.draw(triangleN);

        //Player + Party Stats
         sf::Text playerHP; 
         float playerHPpercentage = ((float)this->game->player.getHP() / this->game->player.getmaxHP());
         sf::Text playerMP;
         float playerMPpercentage = ((float)this->game->player.getMP() / this->game->player.getmaxMP());



        playerHP.setString(std::to_string(this->game->player.getHP()) + "/" + std::to_string(this->game->player.getmaxHP()));
        playerHP.setFont(this->game->font);
        playerHP.setCharacterSize(18);

        playerMP.setString(std::to_string(this->game->player.getMP()) + "/" + std::to_string(this->game->player.getmaxMP()));
        playerMP.setFont(this->game->font);
        playerMP.setCharacterSize(18);

        sf::RectangleShape playerHPBar(sf::Vector2f(100.0f * playerHPpercentage, 10.0f));
        sf::RectangleShape playerHPBarBackground(sf::Vector2f(100.0f, 10.0f));
        playerHPBar.setPosition(1770.0f,310.0f);
        playerHPBar.setFillColor(sf::Color(127,255,0));
        playerHPBarBackground.setFillColor(sf::Color(51,51,51));
        playerHPBarBackground.setPosition(1770.0f,310.0f);
        playerHPBarBackground.setOutlineThickness(1.2f);
        playerHPBarBackground.setOutlineColor(sf::Color::Black);
       

        
        sf::RectangleShape playerMPBar(sf::Vector2f(100.0f * playerMPpercentage, 10.0f));
        sf::RectangleShape playerMPBarBackground(sf::Vector2f(100.0f, 10.0f));
        playerMPBarBackground.setPosition(1770.0f,330.0f);
        playerMPBar.setPosition(1770.0f,330.0f);
        playerMPBarBackground.setFillColor(sf::Color(51,51,51));
        playerMPBarBackground.setOutlineThickness(1.2f);
        playerMPBarBackground.setOutlineColor(sf::Color::Black);
        playerMPBar.setFillColor(sf::Color(0,0,255));


        sf::VertexArray playerBackground(sf::Quads, 4);
        playerBackground[0].position = sf::Vector2f(1960.0f, 260.0f);
        playerBackground[1].position = sf::Vector2f(1960.0f, 350.0f);
        playerBackground[2].position = sf::Vector2f(1730.0f, 350.0f);
        playerBackground[3].position = sf::Vector2f(1730.0f, 260.0f);

        playerBackground[0].color = sf::Color(255,0,0,200);
        playerBackground[1].color = sf::Color(255,0,0,200);
        playerBackground[2].color = sf::Color(0,0,0,200);
        playerBackground[3].color = sf::Color(0,0,0,200);

        sf::Sprite playerSprite = this->game->playerSprite;
        playerSprite.setPosition(1770.0f, 260.0f);


        playerHP.setPosition(1735.0f, 305.0f);

        playerMP.setPosition(1735.0f,323.0f);


        this->game->window.draw(playerBackground);
        this->game->window.draw(playerHPBarBackground);
        this->game->window.draw(playerHPBar);
        this->game->window.draw(playerMPBarBackground);
        this->game->window.draw(playerMPBar);
        this->game->window.draw(playerSprite);
        this->game->window.draw(playerHP);
        this->game->window.draw(playerMP);
        
        //Party Member 2 HUD element

        sf::Text pmember2HP; 
        float pmember2HPpercentage = ((float)this->game->pmember2.getHP() / this->game->pmember2.getmaxHP());
        sf::Text pmember2MP;
        float pmember2MPpercentage = ((float)this->game->pmember2.getMP() / this->game->pmember2.getmaxMP());

        pmember2HP.setString(std::to_string(this->game->pmember2.getHP()) + "/" + std::to_string(this->game->pmember2.getmaxHP()));
        pmember2HP.setFont(this->game->font);
        pmember2HP.setCharacterSize(18);

        pmember2MP.setString(std::to_string(this->game->pmember2.getMP()) + "/" + std::to_string(this->game->pmember2.getmaxMP()));
        pmember2MP.setFont(this->game->font);
        pmember2MP.setCharacterSize(18);

        sf::RectangleShape pmember2HPBar(sf::Vector2f(100.0f * pmember2HPpercentage, 10.0f));
        sf::RectangleShape pmember2HPBarBackground(sf::Vector2f(100.0f, 10.0f));
        pmember2HPBar.setPosition(1770.0f,417.0f);
        pmember2HPBar.setFillColor(sf::Color(127,255,0));
        pmember2HPBarBackground.setFillColor(sf::Color(51,51,51));
        pmember2HPBarBackground.setPosition(1770.0f,417.0f);
        pmember2HPBarBackground.setOutlineThickness(1.2f);
        pmember2HPBarBackground.setOutlineColor(sf::Color::Black);
       

        
        sf::RectangleShape pmember2MPBar(sf::Vector2f(100.0f * pmember2MPpercentage, 10.0f));
        sf::RectangleShape pmember2MPBarBackground(sf::Vector2f(100.0f, 10.0f));
        pmember2MPBarBackground.setPosition(1770.0f,437.0f);
        pmember2MPBar.setPosition(1770.0f,437.0f);
        pmember2MPBarBackground.setFillColor(sf::Color(51,51,51));
        pmember2MPBarBackground.setOutlineThickness(1.2f);
        pmember2MPBarBackground.setOutlineColor(sf::Color::Black);
        pmember2MPBar.setFillColor(sf::Color(0,0,255));


        sf::VertexArray pmember2Background(sf::Quads, 4);
        pmember2Background[0].position = sf::Vector2f(1960.0f, 370.0f);
        pmember2Background[1].position = sf::Vector2f(1960.0f, 460.0f); // 20px spacing, 90px size
        pmember2Background[2].position = sf::Vector2f(1730.0f, 460.0f);
        pmember2Background[3].position = sf::Vector2f(1730.0f, 370.0f);

        pmember2Background[0].color = sf::Color(255,0,0,200);
        pmember2Background[1].color = sf::Color(255,0,0,200);
        pmember2Background[2].color = sf::Color(0,0,0,200);
        pmember2Background[3].color = sf::Color(0,0,0,200);

        sf::Sprite pmember2Sprite = this->game->pmember2Sprite;
        pmember2Sprite.setPosition(1770.0f, 370.0f);
        pmember2Sprite.setScale(0.5f,0.5f);


        pmember2HP.setPosition(1735.0f, 412.0f);

        pmember2MP.setPosition(1735.0f,432.0f);

        this->game->window.draw(pmember2Background);
        this->game->window.draw(pmember2HPBarBackground);
        this->game->window.draw(pmember2HPBar);
        this->game->window.draw(pmember2MPBarBackground);
        this->game->window.draw(pmember2MPBar);
        this->game->window.draw(pmember2Sprite);
        this->game->window.draw(pmember2HP);
        this->game->window.draw(pmember2MP);


        //Party Member 3 HUD element

      

        sf::Text pmember3HP; 
        float pmember3HPpercentage = ((float)this->game->pmember3.getHP() / this->game->pmember3.getmaxHP());
        sf::Text pmember3MP;
        float pmember3MPpercentage = ((float)this->game->pmember3.getMP() / this->game->pmember3.getmaxMP());

        pmember3HP.setString(std::to_string(this->game->pmember3.getHP()) + "/" + std::to_string(this->game->pmember3.getmaxHP()));
        pmember3HP.setFont(this->game->font);
        pmember3HP.setCharacterSize(18);

        pmember3MP.setString(std::to_string(this->game->pmember3.getMP()) + "/" + std::to_string(this->game->pmember3.getmaxMP()));
        pmember3MP.setFont(this->game->font);
        pmember3MP.setCharacterSize(18);

        sf::RectangleShape pmember3HPBar(sf::Vector2f(100.0f * pmember3HPpercentage, 10.0f));
        sf::RectangleShape pmember3HPBarBackground(sf::Vector2f(100.0f, 10.0f));
        pmember3HPBar.setPosition(1770.0f,520.0f);
        pmember3HPBar.setFillColor(sf::Color(127,255,0));
        pmember3HPBarBackground.setFillColor(sf::Color(51,51,51));
        pmember3HPBarBackground.setPosition(1770.0f,520.0f);
        pmember3HPBarBackground.setOutlineThickness(1.2f);
        pmember3HPBarBackground.setOutlineColor(sf::Color::Black);
       

        
        sf::RectangleShape pmember3MPBar(sf::Vector2f(100.0f * pmember3MPpercentage, 10.0f));
        sf::RectangleShape pmember3MPBarBackground(sf::Vector2f(100.0f, 10.0f));
        pmember3MPBarBackground.setPosition(1770.0f,540.0f);
        pmember3MPBar.setPosition(1770.0f,540.0f);
        pmember3MPBarBackground.setFillColor(sf::Color(51,51,51));
        pmember3MPBarBackground.setOutlineThickness(1.2f);
        pmember3MPBarBackground.setOutlineColor(sf::Color::Black);
        pmember3MPBar.setFillColor(sf::Color(0,0,255));


        sf::VertexArray pmember3Background(sf::Quads, 4);
        pmember3Background[0].position = sf::Vector2f(1960.0f, 480.0f);
        pmember3Background[1].position = sf::Vector2f(1960.0f, 570.0f); // 20px spacing, 90px size
        pmember3Background[2].position = sf::Vector2f(1730.0f, 570.0f);
        pmember3Background[3].position = sf::Vector2f(1730.0f, 480.0f);

        pmember3Background[0].color = sf::Color(255,0,0,200);
        pmember3Background[1].color = sf::Color(255,0,0,200);
        pmember3Background[2].color = sf::Color(0,0,0,200);
        pmember3Background[3].color = sf::Color(0,0,0,200);

        sf::Sprite pmember3Sprite = this->game->pmember3Sprite;
        pmember3Sprite.setPosition(1770.0f, 480.0f);
        

        pmember3HP.setPosition(1735.0f, 515.0f);

        pmember3MP.setPosition(1735.0f,535.0f);

        this->game->window.draw(pmember3Background);
        this->game->window.draw(pmember3HPBarBackground);
        this->game->window.draw(pmember3HPBar);
        this->game->window.draw(pmember3MPBarBackground);
        this->game->window.draw(pmember3MPBar);
        this->game->window.draw(pmember3Sprite);
        this->game->window.draw(pmember3HP);
        this->game->window.draw(pmember3MP);

        //Party Member 4 HUD element


          sf::Text pmember4HP; 
        float pmember4HPpercentage = ((float)this->game->pmember3.getHP() / this->game->pmember3.getmaxHP());
        sf::Text pmember4MP;
        float pmember4MPpercentage = ((float)this->game->pmember3.getMP() / this->game->pmember3.getmaxMP());

        pmember4HP.setString(std::to_string(this->game->pmember4.getHP()) + "/" + std::to_string(this->game->pmember4.getmaxHP()));
        pmember4HP.setFont(this->game->font);
        pmember4HP.setCharacterSize(18);

        pmember4MP.setString(std::to_string(this->game->pmember4.getMP()) + "/" + std::to_string(this->game->pmember4.getmaxMP()));
        pmember4MP.setFont(this->game->font);
        pmember4MP.setCharacterSize(18);

        sf::RectangleShape pmember4HPBar(sf::Vector2f(100.0f * pmember4HPpercentage, 10.0f));
        sf::RectangleShape pmember4HPBarBackground(sf::Vector2f(100.0f, 10.0f));
        pmember4HPBar.setPosition(1770.0f,627.0f);
        pmember4HPBar.setFillColor(sf::Color(127,255,0));
        pmember4HPBarBackground.setFillColor(sf::Color(51,51,51));
        pmember4HPBarBackground.setPosition(1770.0f,627.0f);
        pmember4HPBarBackground.setOutlineThickness(1.2f);
        pmember4HPBarBackground.setOutlineColor(sf::Color::Black);
       

        
        sf::RectangleShape pmember4MPBar(sf::Vector2f(100.0f * pmember4MPpercentage, 10.0f));
        sf::RectangleShape pmember4MPBarBackground(sf::Vector2f(100.0f, 10.0f));
        pmember4MPBarBackground.setPosition(1770.0f,650.0f);
        pmember4MPBar.setPosition(1770.0f,650.0f);
        pmember4MPBarBackground.setFillColor(sf::Color(51,51,51));
        pmember4MPBarBackground.setOutlineThickness(1.2f);
        pmember4MPBarBackground.setOutlineColor(sf::Color::Black);
        pmember4MPBar.setFillColor(sf::Color(0,0,255));


        sf::VertexArray pmember4Background(sf::Quads, 4);
        pmember4Background[0].position = sf::Vector2f(1960.0f, 590.0f);
        pmember4Background[1].position = sf::Vector2f(1960.0f, 680.0f); // 20px spacing, 90px size
        pmember4Background[2].position = sf::Vector2f(1730.0f, 680.0f);
        pmember4Background[3].position = sf::Vector2f(1730.0f, 590.0f);

        pmember4Background[0].color = sf::Color(255,0,0,200);
        pmember4Background[1].color = sf::Color(255,0,0,200);
        pmember4Background[2].color = sf::Color(0,0,0,200);
        pmember4Background[3].color = sf::Color(0,0,0,200);

        sf::Sprite pmember4Sprite = this->game->pmember4Sprite;
        pmember4Sprite.setPosition(1785.0f, 590.0f);
        

        pmember4HP.setPosition(1735.0f, 622.0f);

        pmember4MP.setPosition(1735.0f,642.0f);

        this->game->window.draw(pmember4Background);
        this->game->window.draw(pmember4HPBarBackground);
        this->game->window.draw(pmember4HPBar);
        this->game->window.draw(pmember4MPBarBackground);
        this->game->window.draw(pmember4MPBar);
        this->game->window.draw(pmember4Sprite);
        this->game->window.draw(pmember4HP);
        this->game->window.draw(pmember4MP);



  


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

    // Pause menu
    if (isPaused) {
        this->game->window.setView(this->game->window.getDefaultView());

        sf::RectangleShape overlay(sf::Vector2f(this->game->window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 150)); // semi-transparent black
        this->game->window.draw(overlay);
    
        sf::Text pauseText;
        pauseText.setFont(this->game->font);
        pauseText.setCharacterSize(48);
        pauseText.setFillColor(sf::Color::White);
    
        // Center the text
        sf::FloatRect bounds = pauseText.getLocalBounds();
        pauseText.setOrigin(bounds.width / 2, bounds.height / 2);
        pauseText.setPosition(
            this->game->window.getSize().x / 2.f,
            this->game->window.getSize().y / 2.f
        );
    
        this->game->window.draw(pauseText);

        // Draw buttons
        resumeButton.draw(this->game->window);
        settingsButton.draw(this->game->window);
        saveButton.draw(this->game->window);
        quitButton.draw(this->game->window);

        // Hover underline effect
        if (resumeButton.isHovered(this->game->window))
            this->game->window.draw(resumeButton.getUnderline());
        if (settingsButton.isHovered(this->game->window))
            this->game->window.draw(settingsButton.getUnderline());
        if (saveButton.isHovered(this->game->window))
            this->game->window.draw(saveButton.getUnderline());
        if (quitButton.isHovered(this->game->window))
            this->game->window.draw(quitButton.getUnderline());
    }    
    return;
}


void GameStateEditor::update(const float dt) //If something needs to be updated based on dt, then go here
{
    if (isPaused)
        return;
    moveSpeed = 100.f * dt;   // movement speed
    this->game->player.update(dt);
    return;
}

void GameStateEditor::handleInput() // Inputs go here
{
    sf::Event event;

    // Poll all events from the window
    while (this->game->window.pollEvent(event))
    {
        switch (event.type)
        {
            // Close the window
            case sf::Event::Closed:
                this->game->window.close();
                break;

            // Resize the window 
            case sf::Event::Resized:
                gameView.setSize(event.size.width, event.size.height);
                guiView.setSize(event.size.width, event.size.height);
                this->game->background.setPosition(
                    this->game->window.mapPixelToCoords(sf::Vector2i(0, 0), this->guiView)
                );
                this->game->background.setScale(
                    float(event.size.width) / float(this->game->background.getTexture()->getSize().x),
                    float(event.size.height) / float(this->game->background.getTexture()->getSize().y)
                );
                break;

            // Toggle pause
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape)
                {
                    isPaused = !isPaused;
                    std::cout << "Paused: " << std::boolalpha << isPaused << std::endl;
                }
                break;

            default:
                break;
        } // end switch

        if (isPaused)
        {
            // Only handle mouse clicks while paused
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                std::cout << "Mouse click while paused\n";

                if (resumeButton.wasClicked(this->game->window)) {
                    std::cout << "Resume clicked\n";
                    isPaused = false; // resume game
                }
                else if (settingsButton.wasClicked(this->game->window)) {
                    std::cout << "Settings clicked (placeholder)\n";
                }
                else if (saveButton.wasClicked(this->game->window)) {
                    std::cout << "Save clicked (placeholder)\n";
                }
                else if (quitButton.wasClicked(this->game->window)) {
                    std::cout << "Quit clicked\n";
                    requestQuitToMenu = true; // just mark it, don’t change state here
                    return; // stop further input for this frame
                }
            }

            // Skip gameplay input while paused
            // just let the while loop finish naturally
        }
    }

    if (requestQuitToMenu) {
        std::cout << "Changing to main menu...\n";
        this->game->changeState(std::make_unique<GameStateStart>(this->game));
        return; // stop further input for this frame
    }

    // Forward movement
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
: game(game),
  resumeButton("Resume", sf::Vector2f(0.f, 0.f), 40, game),
  settingsButton("Settings", sf::Vector2f(0.f, 0.f), 40, game),
  saveButton("Save", sf::Vector2f(0.f, 0.f), 40, game),
  quitButton("Quit to Menu", sf::Vector2f(0.f, 0.f), 40, game)
{
    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->guiView.setSize(pos);
    this->gameView.setSize(pos);
    pos *= 0.5f;
    this->guiView.setCenter(pos);
    this->gameView.setCenter(pos);

    moveSpeed = 0.f;

    // Load textures once
    doorTexture.loadFromFile("assets/door_texture.png");
    doorTexture.setSmooth(false);
    doorTexture.generateMipmap();
    doorImage = wallTexture.copyToImage();
    wallTexture.loadFromFile("assets/wall_texture.jpg");
    wallTexture.setSmooth(false);   // prevents blur
    wallTexture.setRepeated(true);
    wallTexture.generateMipmap();   // improves close-up detail
    wallImage = wallTexture.copyToImage(); // for pixel-level access
    textureWidth = wallImage.getSize().x;
    textureHeight = wallImage.getSize().y; //dont change these since tex are the same size

    
    resumeButton.changePosition(50.f, 190.f);
    settingsButton.changePosition(50.f, 230.f);
    saveButton.changePosition(50.f, 270.f);
    quitButton.changePosition(50.f, 310.f);
}
