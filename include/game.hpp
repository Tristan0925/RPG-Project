#ifndef GAME_HPP
#define GAME_HPP

#include <stack>
#include <SFML/Graphics.hpp>
#include <memory>
#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"
#include "NPC.hpp"
#include "item.hpp"

class GameState;

class Game
{
    private:

    void loadTextures();


    public:
    Player player;
    NPC pmember2;
    NPC pmember3;
    NPC pmember4;
    Item hpItem;
    Item manaItem;


    Map map;
    sf::Font font;
      

    std::stack<std::unique_ptr<GameState>> states;

    sf::RenderWindow window;
    TextureManager texmgr;
    sf::Sprite background;
    sf::Sprite playerSprite;
    sf::Sprite pmember2Sprite;
    sf::Sprite pmember3Sprite;
    sf::Sprite pmember4Sprite;

    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void changeState(std::unique_ptr<GameState> state);
    GameState* peekState();

    void gameLoop();

    Game();
    ~Game();
};

#endif 