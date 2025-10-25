#ifndef GAME_STATE_EDITOR_HPP
#define GAME_STATE_EDITOR_HPP
 
#include <SFML/Graphics.hpp>
#include "Button.hpp"
#include "game_state.hpp"
#include "Map.hpp"

class GameStateEditor : public GameState
{
    private:
    Game* game;
 
    sf::View gameView;
    sf::View guiView;


    sf::Text saveText;
    sf::Clock saveClock;
    bool showSaveText = false;
    bool isPaused = false;
    float moveSpeed;
    Map map;
    const float PI = 3.14159f;
    const float FOV = PI / 2.0f; 

    Button resumeButton;
    Button settingsButton;
    Button saveButton;
    Button loadButton;
    Button quitButton;

    sf::Texture wallTexture;
    sf::Texture doorTexture;
    sf::Image doorImage;
    sf::Image wallImage;
    int textureWidth;
    int textureHeight;

    bool requestQuitToMenu = false;
    

    public:
 
    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
 
    GameStateEditor(Game* game);
};
 
#endif