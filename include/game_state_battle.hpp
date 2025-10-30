#ifndef GAME_STATE_BATTLE_HPP
#define GAME_STATE_BATTLE_HPP

#include "game_state.hpp"
#include "Player.hpp"
#include <SFML/Graphics.hpp>

class GameStateBattle : public GameState {
private:
    Player* player;           
    sf::Font font;
    sf::Text battleText;
    sf::RectangleShape background;

public:
    GameStateBattle(Game* game);
    virtual ~GameStateBattle() {}

    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
};

#endif
