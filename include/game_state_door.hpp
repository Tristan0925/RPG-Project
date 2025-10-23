#ifndef GAME_STATE_DOOR_HPP
#define GAME_STATE_DOOR_HPP

#include <SFML/Graphics.hpp>
#include "game_state.hpp"
#include "game.hpp"
//import item here
class GameStateDoor : public GameState
{
    private: 
    // this should have 3 sprites for funny characters, textbox, text, 
    sf::RectangleShape fader;
    sf::Image doorRoomImage;
    sf::Text speaker;
    sf::Text textInTextbox;
    sf::RectangleShape Textbox;
    sf::RectangleShape speakerBackground;
    int transparency;
    //place item here


    void backToGame();
    
    public:

    virtual void draw (const float dt);
    virtual void update(const float dt);
    virtual void handleInput();

    GameStateDoor(Game* game, int x, int y);
};

#endif