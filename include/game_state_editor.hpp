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

    bool slotMenuActive = false;

    enum class SlotMenuMode {
        None,
        Save,
        Load
    } slotMenuMode = SlotMenuMode::None;

    Button slot1;
    Button slot2;
    Button slot3;
    Button backButton;

    bool showSaveText = false;
    sf::Text saveText;
    sf::Clock saveClock;
    

    public:
 
    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
 
    GameStateEditor(Game* game);
};
 
#endif