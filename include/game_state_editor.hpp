#ifndef GAME_STATE_EDITOR_HPP
#define GAME_STATE_EDITOR_HPP
 
#include <SFML/Graphics.hpp>
#include "Button.hpp"
#include "game_state.hpp"
#include "Map.hpp"

class GameStateEditor : public GameState
{
    private:
    int doorState = 0; //controls input reading while moving through a door
    int transparency;
    sf::RectangleShape fader;
    Game* game;
 
    sf::View gameView;
    sf::View guiView;
    
    bool isPaused = false; //controls input reading while pressing pause
    float moveSpeed;
    Map map;
    const float PI = 3.14159f;
    const float FOV = PI / 2.0f; 

    Button resumeButton;
    Button settingsButton;
    Button saveButton;
    Button quitButton;

    sf::Texture wallTexture;
    sf::Texture doorTexture;
    sf::Image doorImage;
    sf::Image wallImage;
    int textureWidth;
    int textureHeight;
    void enterDoor(int x, int y);

    bool requestQuitToMenu = false;
    

    public:
 
    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
 
    GameStateEditor(Game* game);
};
 
#endif