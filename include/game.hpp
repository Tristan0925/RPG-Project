#ifndef GAME_HPP
#define GAME_HPP

#include <stack>
#include <SFML/Graphics.hpp>
#include <memory>
#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"

class GameState;

class Game
{
    private:

    void loadTextures();


    public:
    Player player;
    Map map;
    sf::Font font;
    

    std::stack<std::unique_ptr<GameState>> states;

    sf::RenderWindow window;
    TextureManager texmgr;
    sf::Sprite background;
    sf::Texture level1floor;
    sf::Texture level1walls;
    sf::Image level1wallsimage;
    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void changeState(std::unique_ptr<GameState> state);
    GameState* peekState();

    void gameLoop();

    Game();
    ~Game();
};

#endif 