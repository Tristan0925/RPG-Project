#ifndef GAME_STATE_DOOR_HPP
#define GAME_STATE_DOOR_HPP

#include <SFML/Graphics.hpp>
#include "game_state.hpp"
#include "game.hpp"
#include "item.hpp"
#include <unordered_map>
//import item here
class GameStateDoor : public GameState
{
    private: 
    // this should have textbox, text, etc.
    sf::View view;
    sf::RectangleShape fader;
    sf::Sprite treasureSprite;
    sf::Texture treasure; 
    sf::Text textInTextbox;
    sf::RectangleShape Textbox;
    int transparency, quantity, gridX, gridY;
    Item hpItem;
    std::string hpItemName;
    Item mpItem;
    std::string mpItemName;
    Player player;
    std::unordered_map<std::string, bool> doorCoordsToHasLoot;
    

    void drawTreasureRoom(); 
    void drawBossBattle(); //WIP
    void backToGame();
    
    public:

    virtual void draw (const float dt);
    virtual void update(const float dt);
    virtual void handleInput();

    GameStateDoor(Game* game, int x, int y);
};

#endif