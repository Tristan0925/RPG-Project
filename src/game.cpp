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

void Game::loadTextures() // load textures used in game_state_start and game_state_editor (for the most part anyway)
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
pmember2("Maya", 1, 2, 3, 5, 3, 3, 0, {{"Fire", 1.0}, {"Ice", 0.5}, {"Physical", 1.0}, {"Force", 1.5}, {"Electric", 1.0}}),
    //  {{ 
    //     Skill("Attack", "Basic attack made with your fists.", "Physical", 1, true, 32, 0.0f, 100.0f, 3.0f, 13.0f), 
    //     Skill("Zio","Low electric damage to a single foe. High accuracy and medium crit rate.", "Electric", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 
    //     Skill("Tarunda", "Raises attack power for a single ally by 25%.", "Damage Amp", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 
    //     Skill("Agi", "Low fire damage to all foes.", "Fire", 4, false, 30, 7, 100.0f, 0.0f, 85, 8),
    //     Skill("Bufula", "Moderate ice damage to a single enemy.", "Ice", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), 
    //     Skill("Mediarama", "Medium ice damage to one foe.", "Healing", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
    //     Skill("Omni-Dimension", "Multiplies the damage of next physical attack by 2.5x.","Almighty", 10, true, 0.0f, 2.5f, 0.0f, 0.0f), 
    //     Skill(), //empty
    //     Skill() //empty 
    // }}) //magic-y guy, SKILLS: attack, low elec, damage debuff to all, low phys, medium ice, mediarama, omni dimension (20% instant kill (does not factor resistances)) 
pmember3("Lisa", 1, 3, 3, 4, 3, 2, 0, {{"Fire", 1.5}, {"Ice", 1.0}, {"Physical", 1.0}, {"Force", 1.0}, {"Electric", 1.5}}), 
    //  Skill("Attack", "Basic attack made with your fists.", "Physical", 1, true, 32, 0.0f, 100.0f, 3.0f, 13.0f), 
    //     Skill("Zanma","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Force", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 
    //     Skill("Diarama", "High physical damage to one foe. Medium crit rate.", "Healing", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 
    //     Skill("Rakunda", "Reduces defense of all enemies by 25%.", "Damage Resist", 4, false, 30, 7, 100.0f, 0.0f, 85, 8), 
    //     Skill("Megido", "High force damage to all foes.", "Magic-Almighty", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), 
    //     Skill("Sinful Shell", "40% ", "Physical", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
    //     Skill("Mediarama", "Multiplies the damage of next physical attack by 2.5x.","Healing", 10, true, 0.0f, 2.5f, 0.0f, 0.0f), 
    //     Skill(), //empty
    //     Skill() //empty
// healer guy, SKILLS: attack, medium force single, diarama, damage resist down, medium almighty all, sharpshoot(40% Instant Kill (factors in phys resistance)), mediarama
pmember4("Eikichi", 1, 5, 2, 3, 2, 3, 0, {{"Fire", 1.0}, {"Ice", 1.5}, {"Physical", 0.5}, {"Force", 1.0}, {"Electric", 1.0}}) //punchy guy, SKILLS: attack,medium ice single, low phys all, medium ice all, attack boost all, medium force one, high phys single
//    Skill("Attack", "Basic attack made with your fists.", "Physical", 1, true, 32, 0.0f, 100.0f, 3.0f, 13.0f), 
//         Skill("Bufula","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Ice", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 
//         Skill("Andalucia", "High physical damage to one foe. Medium crit rate.", "Physical", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 
//         Skill("Mabufula", "Low fire damage to all foes.", "Ice", 4, false, 30, 7, 100.0f, 0.0f, 85, 8), 
//         Skill("Matarukaja", "High force damage to all foes.", "Damage Amp", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), 
//         Skill("Zanma", "Medium ice damage to one foe.", "Force", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
//         Skill("Stasis Blade", "Multiplies the damage of next physical attack by 2.5x.","Physical", 10, true, 0.0f, 2.5f, 0.0f, 0.0f), 
//         Skill(), //empty
//         Skill() //empty

    // init all variables which are used throughout the game states here 
   
{
    //remember to initialize their skills
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
