#ifndef GAME_STATE_BATTLE_HPP
#define GAME_STATE_BATTLE_HPP

#include "game_state.hpp"
#include "Player.hpp"
#include <SFML/Graphics.hpp>

class GameStateBattle : public GameState {
private:
    Player* player;  

    // Text         
    sf::Font font;
    sf::Text battleText;

    // Shapes
    sf::RectangleShape background;
    sf::RectangleShape textBox;
    sf::RectangleShape enemyBackground;

    // Textures
    sf::Texture enemyBackgroundTex;

    // Player status
    sf::Text playerHP;
    sf::Text playerMP;

    sf::RectangleShape playerHPBar;
    sf::RectangleShape playerHPBarBackground;

    sf::RectangleShape playerMPBar;
    sf::RectangleShape playerMPBarBackground;

    sf::VertexArray playerBackground;

    sf::Sprite playerSprite;

public:
    GameStateBattle(Game* game);
    virtual ~GameStateBattle() {}

    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
};

#endif
