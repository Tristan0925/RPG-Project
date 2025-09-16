/*
Entry point for the RPG game.
Sets up the game window, handles the main loop, and manages player movement via camera.

-- This version simulates a first-person view by moving the camera instead of a visible player sprite.

*/

// sf is the SFML version of a namespace, std being the standard one. Namespaces are basically like containers that hold variables, functions, and the like. Kinda similar to modules in python (i think).

#include <SFML/Graphics.hpp> 
#include <SFML/System.hpp>
#include <cmath>
#include <stack>
#include "game.hpp"
#include "game_state.hpp"
#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"

void Game::loadTextures()
{
    texmgr.loadTexture("background", "./assets/peripeteia.jpg");
}
void Game::pushState(std::unique_ptr<GameState> state) //place game state onto stack
{
    this->states.push(std::move(state));
    return;
}

void Game::popState() //remove the game state off the stack as to save on memory
{
    if(!this->states.empty())
    this->states.pop();
}

void Game::changeState(std::unique_ptr<GameState> state) //change game state
{
if (!this->states.empty())
{
    popState();
}
pushState(std::move(state));
return;
}

GameState* Game::peekState() //check game state
{
    if(this->states.empty())
    {
        return nullptr;
    }
    return this->states.top().get();
}


void Game::gameLoop() //place game logic in here
{
   sf::Clock clock;

   while(this-> window.isOpen())
   {
    sf::Time elapsed = clock.restart();
    float dt = elapsed.asSeconds();

    if(peekState() == nullptr)
    {
        continue;
    }
    peekState() -> handleInput();
    peekState() -> update(dt);
    this ->window.clear(sf::Color::Black);
    peekState()->draw(dt);
    this->window.display();
   }
}

Game::Game() //place constants in here
{
   this->loadTextures();
   this->window.create(sf::VideoMode(800, 600), "Untitled RPG Project"); // create a window
   this->window.setFramerateLimit(60); // set frame rate
   this->background.setTexture(this->texmgr.getRef("background"));
    if (!map.loadFromFile("assets/map1.txt")) {
        throw std::runtime_error("failed to load");
    }

    sf::Vector2f spawn(99.0f,99.0f);
    this->player.setPosition(spawn);
}

Game::~Game() //get rid of all the things on the stack
{
while(!this->states.empty())
{
    popState();
}

}
