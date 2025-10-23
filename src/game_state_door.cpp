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
  //create rooms based on x and y coords. 
   fader.setFillColor(sf::Color::Blue);
   fader.setSize(sf::Vector2f(1920,1080));
}

void GameStateDoor::backToGame()
{
    this->game->popState();
    return;
}


