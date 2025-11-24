

#include <SFML/Graphics.hpp>
#include "game_state_door.hpp"
#include "game_state.hpp"
#include "game_state_battle.hpp"
#include <iostream>
#include <string>
#include "item.hpp"
#include "Player.hpp"
#include <iostream>
#include <array>
#include <random>

void GameStateDoor::draw(const float dt)
{
    this->game->window.setView(this->view);
 if (isItemRoom){
    
   this->game->window.draw(treasureSprite);
   std::array<Item, 2> playerinv = this->game->player.getInventory();
   for (const auto& item : playerinv) {
    std::cout << item.showName() << " ";
    std::cout << item.getQuantity() << " ";
}
std::cout << std::endl;
   textInTextbox.setString("- You entered the room and found a chest. In the chest contained x" + std::to_string(quantity) + " " + itemName +".");
   this->game->window.draw(Textbox);
   this->game->window.draw(textInTextbox);
 }
 else if (isEmptyRoom){
      std::cout << coordinatePair << std::endl;
   this->game->window.draw(treasureSprite);
   textInTextbox.setString("- You entered the room and found a chest. In the chest contained NOTHING.");
   this->game->window.draw(Textbox);
   this->game->window.draw(textInTextbox);
 }

 else if(isBossRoom){
    textInTextbox.setString("You sense a terrifying presence ahead. After you face it, you cannot return. Proceed?\n Space - Head Back.     Enter - Proceed.");
    this->game->window.draw(Textbox);
    this->game->window.draw(textInTextbox);
    if (floorNumber == 1){
        if (!preludeTrack.openFromFile("./assets/music/boss1prelude.mp3")) {
        std::cout << "Could not load music file" << std::endl;
    } else {
        preludeTrack.setLoop(true);
        preludeTrack.play();
    }
    }
    else if (floorNumber == 2){
        if (!preludeTrack.openFromFile("./assets/music/boss2prelude.mp3")) {
        std::cout << "Could not load music file" << std::endl;
    } else {
        preludeTrack.setLoop(true);
        preludeTrack.play();
    }
    }
 }

 else{
    std::cout << "Something went horribly wrong if you are seeing this" << std::endl;
    std::cout << coordinatePair << std::endl;
    
    std::exit(99);
 }
   this->game->window.draw(fader);  
}

void GameStateDoor::update(const float dt)
{
transparency -= static_cast<int>(100 * 2 * dt);
if (transparency < 0) transparency = 0;
   fader.setFillColor(sf::Color(0,0,0,static_cast<sf::Uint8>(transparency)));
}

void GameStateDoor::handleInput()
{
    sf::Event event;
  while (this->game->window.pollEvent(event)) //REMEMBER TO FIX THIS WHEN MERGING PAUSE MENU
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
                this->view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height)); //resize window to new window size
                this->view.setCenter(this->view.getSize() / 2.f); //center view 
                this->game->window.setView(this->view); //updates view
                break;
            }
            case sf::Event::KeyPressed:{
                if (event.key.code == sf::Keyboard::Space){ // && !isBossRoom
                    this->game->requestPop();
                    return;
                }
                else if (event.key.code == sf::Keyboard::Enter && isBossRoom){
                    this->game->requestChange(std::make_unique<GameStateBattle>(this->game, true));
                    return;
                }
            }
            default: break;
        }
    }
}

GameStateDoor::GameStateDoor(Game* game, int x, int y)

{
    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->view.setSize(pos);
    pos *= 0.5f;
    this->view.setCenter(pos);
    
    this->game = game;
    transparency = 255;
    fader.setSize(sf::Vector2f(1920,1080));
    fader.setFillColor(sf::Color(0,0,0,static_cast<sf::Uint8>(transparency)));
    if (!treasure.loadFromFile("./assets/treasure.jpg")){
        printf("Error loading treasure.jpg");
    }
    treasureSprite.setTexture(treasure);
    treasureSprite.setPosition(960,540);
    textInTextbox.setFont(this->game->font);
    textInTextbox.setCharacterSize(40);
    Textbox.setOutlineColor(sf::Color::Red);
    Textbox.setOutlineThickness(2.0f);
    Textbox.setFillColor(sf::Color::Black);
    Textbox.setSize(sf::Vector2(1720.0f,200.0f));
    Textbox.setPosition(100,830);
    textInTextbox.setPosition(120,830);

    hpItem = this->game->hpItem;
    mpItem = this->game->manaItem;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> chanceOfItem(1,2);
    std::uniform_int_distribution<> quantityOfItem(1,3);
    quantity = quantityOfItem(gen);
      std::cout << x << y << std::endl;
    coordinatePair = ("(" + std::to_string(x) + ", " + std::to_string(y+1) + ")"); //adding 1 is a band-aid to the real problem (idk what it is)
    player = this->game->player;
    std::array<Item, 2> playerinv = this->game->player.getInventory(); 



 for (const auto& pair : this->game->doorCoordinatesToHasLoot) {
       if (pair.first == coordinatePair && coordinatePair == "(34, 4)" && pair.second == true){ //replace with bossCoordinates
            isBossRoom = true;
            isEmptyRoom = false;
            isItemRoom = false;
            floorNumber = 1;
            this->game->doorCoordinatesToHasLoot[pair.first] = false;
        }
        else if (pair.first == coordinatePair && pair.second == true){
            isItemRoom = true;
            isEmptyRoom = false;
            isBossRoom = false;
            this->game->doorCoordinatesToHasLoot[pair.first] = false;
            if (chanceOfItem(gen) == 1) {
                itemName = hpItem.showName();
                this->game->player.addToInventory(hpItem, quantity); 

            }
            else {
                itemName = mpItem.showName();
            this->game->player.addToInventory(mpItem, quantity);
            }
        } 
        else if (pair.first == coordinatePair && pair.second == false){
            isEmptyRoom = true;
            isItemRoom = false;
            isBossRoom = false;
        }
       
    }



   
  
}



GameStateDoor::~GameStateDoor() {
    std::cout << "GameStateDoor destroyed" << std::endl;
}


