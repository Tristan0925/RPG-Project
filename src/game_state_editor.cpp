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

     sf::RectangleShape ceiling(sf::Vector2f(1920, 1080));
        ceiling.setFillColor(sf::Color(0, 250, 0));
        ceiling.setPosition(0, 0);
        this->game->window.draw(ceiling);
        
        // Draw floor
        sf::RectangleShape groundfloor(sf::Vector2f(1920, 1080));
        groundfloor.setFillColor(sf::Color(50, 50, 50));
        groundfloor.setPosition(0, 550);
        this->game->window.draw(groundfloor);

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
                if (this->game->map.isDoor(mapX,mapY)) hit = 1;
                if (this->game->map.isWall(mapX, mapY)) hit = 1;
                
            }
            
            if (side == 0) // Once a wall is hit, you compute the distance from the player to the wall
                perpWallDist = (sideDistX - deltaDistX) * cos(rayAngle - this->game->player.getAngle());
            else
                perpWallDist = (sideDistY - deltaDistY) * cos(rayAngle - this->game->player.getAngle());
            
            // Wall height
            // Near wall = small denominator = tall line; Far wall = big denominator = short line.
            int lineHeight = (int)(screenHeight / perpWallDist);
            
            int drawStart = -lineHeight / 2 + screenHeight / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + screenHeight / 2;
            if (drawEnd >= screenHeight) drawEnd = screenHeight - 1;
            
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, drawStart), this->game->map.isDoor(mapX,mapY) ? sf::Color(0,180,180) : sf::Color::Red),
                sf::Vertex(sf::Vector2f(x, drawEnd),   this->game->map.isDoor(mapX,mapY) ? sf::Color(0,180,180) : sf::Color::White)
                };
                 this->game->window.draw(line, 2, sf::Lines);
            }

        // Setup minimap
        float mapSizePx = 200.f;
        float padding = 100.f;
        float vertOffset = 50.0f;
        
        // Get window size
        float winW = (float)this->game->window.getSize().x;
        float winH = (float)this->game->window.getSize().y;
        sf::Vector2f minimapCenterPos = this->game->player.getPosition();

        // Minimap view
        sf::View minimapView;
        minimapView.setSize(400.f, 400.f);
        minimapView.setCenter(minimapCenterPos);

        // Initial rotation offset (since the player spawns looking East)
        float initialRotationDeg = 90.f;
        float playerAngleDeg = this->game->player.getAngle() * 180.f / 3.14159f;
        minimapView.setRotation(initialRotationDeg - playerAngleDeg);


       
        // Set viewport 
        sf::FloatRect vp(
            padding / winW,                    // left
            (winH - mapSizePx - padding - vertOffset) / winH, // top (distance from top of window)
            mapSizePx / winW,                  // width
            mapSizePx / winH               // height
        );
        minimapView.setViewport(vp);

        // Draw minimap border
        sf::RectangleShape minimapBorder(sf::Vector2f(mapSizePx, mapSizePx));
        minimapBorder.setOrigin(mapSizePx/2, mapSizePx/2); // rotate around center
        minimapBorder.setPosition(padding + mapSizePx/2, winH - mapSizePx/2 - padding - vertOffset);
        minimapBorder.setFillColor(sf::Color(0,0,0,180));
        minimapBorder.setOutlineThickness(2.f);
        minimapBorder.setOutlineColor(sf::Color::Red);
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



  


    return;
}

void GameStateEditor::update(const float dt) //If something needs to be updated based on dt, then go here
{

   moveSpeed = 128.f * dt;   // movement speed
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