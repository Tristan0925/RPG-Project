//This represents the main menu screen.

#include <SFML/Graphics.hpp>
#include "game_state_door.hpp"
#include "game_state.hpp"
#include <iostream>

void GameStateDoor::draw(const float dt)
{
   
 
   this->game->window.draw(fader);
  
  
    
}

void GameStateDoor::update(const float dt)
{
transparency -= static_cast<int>(100 * dt);
if (transparency < 0) transparency = 0;
   fader.setFillColor(sf::Color(255,0,0,static_cast<sf::Uint8>(transparency)));
}

void GameStateDoor::handleInput()
{
    sf::Event event;
    while(this->game->window.pollEvent(event))
    {}
    return;
}

GameStateDoor::GameStateDoor(Game* game, int x, int y)

{
    this->game = game;
    transparency = 255;
  //create rooms based on x and y coords. 
   fader.setSize(sf::Vector2f(1920,1080));
   fader.setFillColor(sf::Color(255,0,0,static_cast<sf::Uint8>(transparency)));
  
}

void GameStateDoor::backToGame()
{
    this->game->popState();
    return;
}


