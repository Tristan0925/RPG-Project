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
#include <string>
#include <vector>
#include <unordered_map>

class GameState;

class Game
{
    private:

    void loadTextures();


    public:
    Item hpItem;
    Item manaItem;
    Player player;
    NPC pmember2;
    NPC pmember3;
    NPC pmember4;
  


    Map map;
    std::vector<std::string> doorCoordinates;
    std::unordered_map<std::string, bool> doorCoordinatesToHasLoot;
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