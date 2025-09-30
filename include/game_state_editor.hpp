#ifndef GAME_STATE_EDITOR_HPP
#define GAME_STATE_EDITOR_HPP
 
#include <SFML/Graphics.hpp>
 
#include "game_state.hpp"
 #include "Map.hpp"
class GameStateEditor : public GameState
{
    private:
 
    sf::View gameView;
    sf::View guiView;

    float moveSpeed;
    Map map;
    const float PI = 3.14159f;
    const float FOV = PI / 3.0f; 
    sf::Vector2i lastTile;  // track last tile player was on

    public:
 
    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
 
    GameStateEditor(Game* game);
};


 
#endif