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
#include <string>
#include <iostream>

void Game::loadTextures() // load textures used everywhere
{
    texmgr.loadTexture("background", "./assets/mainmenu.jpeg");
    texmgr.loadTexture("playerSprite", "./assets/player.png");
    texmgr.loadTexture("pmember2Sprite", "./assets/partymember2.png");
    texmgr.loadTexture("pmember3Sprite", "./assets/partymember3.png");
    texmgr.loadTexture("pmember4Sprite", "./assets/partymember4.png");
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

// requestPush/requestPop/requestChange implementations
void Game::requestPush(std::unique_ptr<GameState> state) {
    pendingState.action = StateAction::Push;
    pendingState.state = std::move(state);
}

void Game::requestPop() {
    pendingState.action = StateAction::Pop;
}

void Game::requestChange(std::unique_ptr<GameState> state) {
    pendingState.action = StateAction::Change;
    pendingState.state = std::move(state);
}

void Game::applyPendingState() {
    switch (pendingState.action) {
        case StateAction::Push:
            if (pendingState.state) {
                pushState(std::move(pendingState.state));
            }
            break;
        case StateAction::Pop:
            popState();
            break;
        case StateAction::Change:
            if (pendingState.state) {
                changeState(std::move(pendingState.state));
            } else {
                // If state is null but action is Change, just pop then do nothing
                popState();
            }
            break;
        case StateAction::None:
        default:
            break;
    }
    // reset pendingState
    pendingState.action = StateAction::None;
    pendingState.state.reset();
}



void Game::gameLoop() //handles the gameloop
{
   sf::Clock clock;

   while(this-> window.isOpen())
   {
    sf::Time elapsed = clock.restart();
    float dt = elapsed.asSeconds();

    if(peekState() == nullptr)
    {
        // maybe sleep a bit here or continue
        continue;
    }
    peekState() -> handleInput();
    peekState() -> update(dt);
    this ->window.clear(sf::Color::Black);
    peekState()->draw(dt);
    this->window.display();

    // <-- apply pending state changes safely after current state finished
    applyPendingState();
   }
}

Game::Game() : hpItem("Dragon Morsel", "Makes you feel like something, but you can't put your finger on it. Heals 100HP.", 100, 0, 0),  //when saves work, conditionally create these constructors
manaItem("Energizing Moss", "Some moss you found in a chest. Not safe for human consumption, but somehow restores 100MP.", 0, 100, 0),
pmember2("Maya",75, 75, 120, 120, 10, 20, 20, 20, 0, 11), //magic-y gal
pmember3("Lisa", 125, 125, 75, 75, 15, 25, 15, 15, 0, 12), // punchy guy
pmember4("Eikichi", 100, 100, 90, 90, 10, 10, 10, 10, 0, 10 ) //healer guy
// init all variables which are used throughout the game states here 
{
   this->loadTextures();
   this->window.create(sf::VideoMode(1920, 1080), "Untitled RPG Project"); // create a window
   this->window.setFramerateLimit(60); // set frame rate
   this->background.setTexture(this->texmgr.getRef("background"));
   this->playerSprite.setTexture(this->texmgr.getRef("playerSprite"));
   this->pmember2Sprite.setTexture(this->texmgr.getRef("pmember2Sprite"));
   this->pmember3Sprite.setTexture(this->texmgr.getRef("pmember3Sprite"));
   this->pmember4Sprite.setTexture(this->texmgr.getRef("pmember4Sprite"));
   if (!map.loadFromFile("assets/map1.txt")) {
    throw std::runtime_error("failed to load");
   }
    if (!font.loadFromFile("assets/Birch.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        std::exit(1);
    } else {
        std::cout << "Font loaded: " << font.getInfo().family << std::endl;
    }


    sf::Vector2f spawn(map.getSpawnX(), map.getSpawnY());
    this->player.setPosition(spawn * 64.f); // scale by tile size

    this->doorCoordinates = map.getDoorCoordinates();
    for (const auto& coord : doorCoordinates) {
        std::cout << coord << " ";
    }
    std::cout << std::endl;
    for (const auto& coord : doorCoordinates) {
        doorCoordinatesToHasLoot[coord] = 1;
    }
   
  
    
    
}

Game::~Game() //get rid of all the things on the stack
{
while(!this->states.empty())
{
    popState();
}

}
