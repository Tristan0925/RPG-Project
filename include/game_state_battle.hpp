#ifndef GAME_STATE_BATTLE_HPP
#define GAME_STATE_BATTLE_HPP

#include "game_state.hpp"
#include "Player.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Button.hpp"

class GameStateBattle : public GameState {
private:
    Player* player;  

    // Battle state
    bool isBossBattle = false;

    // Music
    sf::Music battleMusic;

    // Text         
    sf::Font font;
    sf::Text battleText;

    // Shapes
    sf::RectangleShape background;
    sf::RectangleShape textBox;
    sf::RectangleShape enemyBackground;
    std::vector<sf::Vector2f> basePositions;


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

    // Player UI
    std::vector<sf::RectangleShape> hpBars;
    std::vector<sf::RectangleShape> mpBars;
    std::vector<sf::Text> hpTexts;
    std::vector<sf::Text> mpTexts;
    std::vector<sf::RectangleShape> playerBackgrounds;
    std::vector<sf::Sprite> playerIcons;
    std::vector<sf::Texture> playerIconTextures;
    std::vector<float> portraitBaseScales;
    std::vector<float> turnPortraitBaseScales;
    
    std::vector<Player*> party;

    // Turn UI
    std::deque<Player*> turnQueue;
    size_t currentTurnIndex = 0;
    std::vector<sf::RectangleShape> turnPortraitBoxes;
    std::vector<sf::Sprite> turnPortraitSprites;
    sf::RectangleShape turnPanelBackground;
    void updateTurnPanel();

    // Battle Buttons
    bool skillMenuActive = false;
    Button attackButton;
    Button skillButton;
    Button itemButton;
    Button guardButton;
    Button escapeButton;

    // Enemies

    void spawnEnemies(bool isBossBattle);
    void setupTurnOrder();
    void enemy(NPC& enemy);
    std::vector<NPC> loadRandomEnemies(int count); 
    std::vector<NPC> enemies;
    std::vector<sf::Texture> enemyTextures; 
    std::vector<sf::Sprite> enemySprites; 
    std::vector<sf::Text> turnEnemyNames; 
        
    // Submenu UI
    std::vector<Button> skillButtons;
    std::vector<Button> itemButtons;
    Button backButton;


    // Battle Menu State
    enum class BattleMenuState {
        Main,
        Skill,
        Item
    };

    BattleMenuState currentMenuState = BattleMenuState::Main;

    // UI tuning
    float ui_startX = 550.f;
    float ui_startY = 800.f;
    float ui_spacing = 220.f;

public:
    GameStateBattle(Game* game, bool isBossBattle);
    virtual ~GameStateBattle() {}

    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
};

#endif
