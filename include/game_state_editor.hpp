#ifndef GAME_STATE_EDITOR_HPP
#define GAME_STATE_EDITOR_HPP
 
#include <SFML/Graphics.hpp>
 
#include "game_state.hpp"
 #include "Map.hpp"
class GameStateEditor : public GameState
{
    private:
    int doorState = 0;
    int transparency;
    sf::RectangleShape fader;
    sf::View gameView;
    sf::View guiView;

    float moveSpeed;
    Map map;
    const float PI = 3.14159f;
    const float FOV = PI / 2.0f; 

    sf::Texture wallTexture;
    sf::Texture doorTexture;
    sf::Image doorImage;
    sf::Image wallImage;
    int textureWidth;
    int textureHeight;
    void enterDoor(int x, int y);

    public:
 
    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
 
    GameStateEditor(Game* game);
};
 
#endif