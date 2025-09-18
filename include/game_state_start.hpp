#ifndef GAME_STATE_START_HPP
#define GAME_STATE_START_HPP

#include <SFML/Graphics.hpp>

#include "game_state.hpp"

class GameStateStart : public GameState
{
    private:

    sf::View view;
<<<<<<< HEAD
      sf::Text title;
=======
>>>>>>> dd81b5daa3557755b002ee232ac7a88cd6cbdf04
    void loadgame();

    public:

    virtual void draw (const float dt);
    virtual void update(const float dt);
    virtual void handleInput();

    GameStateStart(Game* game);
};

#endif