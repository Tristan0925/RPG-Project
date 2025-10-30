#ifndef GAME_STATE_EDITOR_HPP
#define GAME_STATE_EDITOR_HPP
 
#include <SFML/Graphics.hpp>
#include "Button.hpp"
#include "game_state.hpp"
#include "Map.hpp"

class GameStateEditor : public GameState
{
    private:
    bool controlInputReadingPaused = false; //controls input reading 
    int transparency;
    sf::RectangleShape fader;
    Game* game;
 
    sf::View gameView;
    sf::View guiView;
    
    bool isPaused = false; //controls input reading while pressing pause
    float moveSpeed;
    Map map;
    const float PI = 3.14159f;
    sf::Vector2i lastTile;  // track last tile player was on
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
    void enterDoor(int x, int y);

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
    bool exitingDoor, enteringDoor = false;
    float exitTimer = 0.0f;
    const float exitDuration = 0.25f;
    

    public:
 
    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
 
    GameStateEditor(Game* game);
};


 
#endif