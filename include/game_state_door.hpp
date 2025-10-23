#ifndef GAME_STATE_DOOR_HPP
#define GAME_STATE_DOOR_HPP

#include <SFML/Graphics.hpp>
#include "game_state.hpp"
#include "game.hpp"
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
    int transparency;
    //place item here

    void drawTreasureRoom(); //i guess put item in the args
    void drawBossBattle(); //WIP
    void backToGame();
    
    public:

    virtual void draw (const float dt);
    virtual void update(const float dt);
    virtual void handleInput();

    GameStateDoor(Game* game, int x, int y);
};

#endif