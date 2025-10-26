//This represents the main menu screen.

#include <SFML/Graphics.hpp>
#include "game_state_door.hpp"
#include "game_state.hpp"
#include <iostream>
#include <string>
#include "item.hpp"
#include "Player.hpp"
#include <iostream>
#include <array>
void GameStateDoor::draw(const float dt)
{
    this->game->window.setView(this->view);
 // long list of if elses which tell what to put in each room
//  if (x,y){
//     textInTextbox.setString("You sense a terrifying presence ahead. Proceed?");
//     this->game->window.draw(Textbox);
//    this->game->window.draw(textInTextbox);
//  }
//  else
 // basic treasure thing
 //IDEA: HAVE AN ARRAY OF ALL THE KNOWN DOOR SPOTS (EXCEPT THE BOSS ROOM), 
 //IDEA: have a hashmap of all the locations of doors, (location -> 1), if 1, give an item with a 50/50 of being hp or mana. If 0, say you've been there already.
 //str-ify the coords: (1,7), (1,0), (34,8), (16,5), 
 if (gridX == 1 && gridY == 0){
   this->game->window.draw(treasureSprite);
   std::array<Item, 2> playerinv = this->game->player.getInventory();
   for (const auto& item : playerinv) {
    std::cout << item.showName() << " ";
    std::cout << item.getQuantity() << " ";
}
std::cout << std::endl;
   textInTextbox.setString("- You entered the room and found a chest. In the chest contained x" + std::to_string(quantity) + " " + hpItemName +".");
   this->game->window.draw(Textbox);
   this->game->window.draw(textInTextbox);
 }
 
 else{
    std::cout << "door position: (" << gridX << ", " << gridY << ")" << std::endl;
   this->game->window.draw(treasureSprite);
   textInTextbox.setString("- You entered the room and found a chest. In the chest contained NOTHING.");
   this->game->window.draw(Textbox);
   this->game->window.draw(textInTextbox);
 }

 









   this->game->window.draw(fader);
   
  
    
}

void GameStateDoor::update(const float dt)
{
transparency -= static_cast<int>(100 * dt);
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
                if (event.key.code == sf::Keyboard::Space){
                    backToGame();
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
    hpItemName = hpItem.showName();
    mpItem = this->game->manaItem;
    mpItemName = mpItem.showName();
    quantity = 1;
    gridX = x;
    gridY = y;
    player = this->game->player;
    std::array<Item, 2> playerinv = this->game->player.getInventory(); 
    for (const auto& item : playerinv) {
    std::cout << item.showName() << " ";
}
std::cout << std::endl;
 //take coords and stringify. compare to list. 
if (gridX == 1 && gridY == 0) this->game->player.addToInventory(hpItem, quantity); 

   
  
}

void GameStateDoor::backToGame()
{
    this->game->popState();
    return;
}


