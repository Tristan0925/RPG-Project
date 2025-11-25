#ifndef GAME_STATE_BATTLE_HPP
#define GAME_STATE_BATTLE_HPP

#include "game_state.hpp"
#include "Player.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Button.hpp"
#include <array>
#include <unordered_map>


class GameStateBattle : public GameState {
private:
    Player* player;  

    // Battle state
    bool isBossBattle = false;
    bool battleOver = false; 
    // Music
    sf::Music currentMusic;
    bool playResultsMusic; 
    
    
    // Text         
    sf::Font font;
    sf::Text battleText;
    
    // Text for results screen
    sf::Text topBarText;
    sf::Text totalEarnedExp;
    std::string totalEarnedExpMessage;
    sf::Text playerName;
    sf::Text pmember2Name;
    sf::Text pmember3Name;
    sf::Text pmember4Name;
    sf::Text playerLevel;
    sf::Text pmember2Level;
    sf::Text pmember3Level;
    sf::Text pmember4Level;
    sf::Text nextLevelPlayer;
    int nextLevelPlayerXp; // set the xps in the constructor
    sf::Text nextLevelPmember2; //set the fonts
    int nextLevelPmember2Xp;
    sf::Text nextLevelPmember3;
    int nextLevelPmember3Xp;
    sf::Text nextLevelPmember4;
    int nextLevelPmember4Xp;
    std::array<sf::Text, 4> levelUpTexts;

    //Text for Level Up!
    sf::Text nameOfCharacterForLevelUp;
    sf::Text levelUpHeaderText;

    sf::Text strength;
    int strengthVal = 0;
    float strengthValPercent = (float)strengthVal / 99;

    sf::Text vitality;
    int vitalityVal = 0;
    float vitalityValPercent = (float)vitalityVal / 99;

    sf::Text magic;
    int magicVal = 0;
    float magicValPercent = (float)magicVal / 99;

    sf::Text agility;
    int agilityVal = 0;
    float agilityValPercent = (float)agilityVal / 99;

    sf::Text luck;
    int luckVal = 0;
    float luckValPercent = (float)luckVal / 99;

    sf::Text maxHp;
    int maxHpVal = 0;
    int recalculatedMaxHp = maxHpVal;

    sf::Text maxMp;
    int maxMpVal = 0;
    int recalculatedMaxMp = recalculatedMaxMp;

    sf::Text distributionText;

    std::array<sf::Text, 8> skillNamesForResults;
    void backToGame();



    // Shapes
    sf::RectangleShape background;
    sf::RectangleShape textBox;
    sf::RectangleShape enemyBackground;
    std::vector<sf::Vector2f> basePositions;
    
    //Shapes for results screen 
    sf::VertexArray topBarTextBackground;
    sf::VertexArray thingsEarnedBackground;
    std::array<sf::RectangleShape, 4> portraitBackgrounds;
    std::array<sf::RectangleShape, 4> levelBackgrounds;
    sf::RectangleShape expBarPlayer;
    float playerXP;
    sf::RectangleShape expBarPmember2;
    float pmember2XP;
    sf::RectangleShape expBarPmember3;
    float pmember3XP;
    sf::RectangleShape expBarPmember4;
    float pmember4XP;
    std::array<sf::RectangleShape, 4> expBarBackgrounds;

    //Shapes for LEVEL UP! screen
    sf::RectangleShape nameplateBackground;
    sf::RectangleShape nameplate;
    std::array<sf::RectangleShape, 5> statBoxes;
    std::array<sf::RectangleShape,5> statBackgrounds;
    std::array<sf::RectangleShape, 2> maxStatBoxes;
    std::array<sf::RectangleShape,2> maxStatBackgrounds;
    sf::RectangleShape stBar;
    sf::RectangleShape viBar;
    sf::RectangleShape maBar;
    sf::RectangleShape agBar;
    sf::RectangleShape luBar;
    sf::RectangleShape pointsToDistributeTextbox;
    float maxBarSize = 99.0;
    
    //map of player + pmember which maps to whether or not they leveled up
    std::map<Player*, bool> levelUpBooleanMap;
    std::map<Player*, bool>::iterator levelUpIterator;
    bool levelupflags = false;

    //Flags that lets me reuse certain parts of the UI 
    bool reuseArrays = false;
    bool reuseTextforLevelUp = false;





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
    void buildSkillButtonsFor(Player* character);

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
    std::vector<NPC> loadEnemies(int count, bool isBossBattle, int bossIndex); 
    int bossIndex;
    std::vector<NPC> enemies;
    std::vector<sf::Texture> enemyTextures; 
    std::vector<sf::Sprite> enemySprites; 
    std::vector<sf::Text> turnEnemyNames; 
    std::vector<sf::RectangleShape> enemyNameBackgrounds;
    int currentEnemyIndex = 0;

    // Enemy helpers
    int getFirstLivingEnemy();
    int getNextLivingEnemy(int index);
    int getPrevLivingEnemy(int index);
    std::vector<sf::RectangleShape> enemyHealthBarsBack;
    std::vector<sf::RectangleShape> enemyHealthBarsFront;
    void cleanupDeadEnemies();
    int getEnemyIndex(NPC* e) const;
        
    // Submenu UI
    std::vector<Button> skillButtons;
    std::vector<Button> itemButtons;
    Button backButton;


    // Battle Menu State
    enum class BattleMenuState {
        Main,
        Skill,
        Item,
        EnemyTurn
    };
    BattleMenuState currentMenuState = BattleMenuState::Main;
    bool setPlayerMenuState = false;
    // Game over Menu State
    enum class GameOverMenuState {
        Main
    };

    bool gameOver = false;
    GameOverMenuState gameOverMenuState;
    sf::Text gameOverText;
    Button quitButton;
    Button loadButton;
    bool loadMenuActive = false;
    Button slot1;
    Button slot2;
    Button slot3;


    // UI tuning
    float ui_startX = 550.f;
    float ui_startY = 800.f;
    float ui_spacing = 220.f;

    //methods to progress the game_state after a battle
    void displayResultsScreen();
    void displayLevelUpScreen();
    
    //you must watch the level up screen before you can continue (sorry)
    bool levelUpTime = false;
    bool distributionFinished = false;
    bool printSkillNames = false;
    bool statsSet = false;

    int totalXpGained = 0;
    int XPdecrementer = 0; //counts how many times the loop has iterated so we can have a cool next exp counter.
    int skillPoints = 0;  //points actually distributed
    int tempSkillPoints = 0;  //holds the total points awarded
    int levelUpAttributeIndex = 0; 
    Player * character; 

    // BuffState
    struct BuffInstance {
        float damageAmp = 1.0f;
        float damageResist = 1.0f;
        float accBoost = 0.0f;
        float evadeBoost = 0.0f;
        int turnsRemaining = 0;
    };
    std::unordered_map<Player*, BuffInstance> activeBuffs;
    //void updateBuffTimers();
    float getElementMultiplier(const Player* target, const Skill* skill) const;


public:
    GameStateBattle(Game* game, bool isBossBattle, int bossIndex);
    virtual ~GameStateBattle() {}

    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();
};

#endif
