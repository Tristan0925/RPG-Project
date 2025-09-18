//This file represents the game stack. Essentially, all the parts of the game(main menu, main game, etc.) exist as separate states and all exist on this stack. The separate files(game_state_editor, game_state_start) create these separate states and let the game stack manage them. The current one being used exists at the top of the stack and will then get popped off when not in use. 
#include <SFML/Graphics.hpp> 
#include <SFML/System.hpp>
#include <cmath>
#include <stack>
#include "game.hpp"
#include "game_state.hpp"
#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"

<<<<<<< HEAD
void Game::loadTextures()
{
    texmgr.loadTexture("background", "./assets/mainmenu.jpeg");
=======

void Game::loadTextures()
{
    texmgr.loadTexture("background", "./assets/mainmenu.jpg");
>>>>>>> dd81b5daa3557755b002ee232ac7a88cd6cbdf04
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


void Game::gameLoop() //things that need to exist across gamestates go here
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

Game::Game() //i'm not sure what these things do just yet
{
   this->loadTextures();
   this->window.create(sf::VideoMode(1920, 1080), "Untitled RPG Project"); // create a window
   this->window.setFramerateLimit(60); // set frame rate
   this->background.setTexture(this->texmgr.getRef("background"));
    if (!map.loadFromFile("assets/map1.txt")) {
        throw std::runtime_error("failed to load");
    }
    if (!font.loadFromFile("assets/Birch.ttf")){
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
