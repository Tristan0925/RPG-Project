#ifndef GAME_STATE_START_HPP
#define GAME_STATE_START_HPP

#include <SFML/Graphics.hpp>
#include "Button.hpp"
#include "game_state.hpp"

class GameStateStart : public GameState
{
    private:

    sf::View view;
    sf::Text title;
    
    Button startgame;
    Button loadButton;
    Button settings;
    Button endgame;
    sf::RectangleShape underline;

    sf::Vector2f defaultTitlePos;
    sf::Vector2f defaultTitleScale;


    void loadgame();

    public:

    virtual void draw (const float dt);
    virtual void update(const float dt);
    virtual void handleInput();

    GameStateStart(Game* game);
};

#endif