#include "game_state_battle.hpp"
#include "game_state_editor.hpp"
#include <iostream>
#include <algorithm>
#include "Button.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include <random>
#include <array>

using json = nlohmann::json;

namespace {
    // File-local helper state and types (keeps header unchanged)
    struct DamagePopup {
        sf::Text text;
        sf::Vector2f velocity;
        float life = 1.0f; // seconds
    };

    // Persistent file-scope containers used by the cpp only
    std::vector<DamagePopup> damagePopups;
    std::vector<float> enemyBaseScales; // keep base scales for each enemy sprite
    std::mt19937& globalRng() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }

    // small helper to find skill by name in the game's skill master list
    const Skill* findSkillByName(const Game* g, const std::string& name) {
        if (!g) return nullptr;
        for (const auto& s : g->skillMasterList) {
            if (s.getName() == name) return &s;
        }
        return nullptr;
    }
}

// static std::string getSkillElement(const Skill* s) {
//     std::string t = s->getType();

//     if (t == "Fire") return "Fire";
//     if (t == "Ice") return "Ice";
//     if (t == "Electric") return "Electric";
//     if (t == "Force") return "Force";
//     if (t == "Magic-Almighty") return "Almighty";
//     if (t == "Almighty") return "Almighty";
    
//     return ""; // physical/heal/buffs etc.
// }

// ---- Enemy Turn Timing State ----
static bool enemyTurnPending = false;
static float enemyTurnTimer = 0.f;
static const float ENEMY_TURN_DELAY = 1.5f; // delay before enemy acts
static Player* pendingEnemy = nullptr;


// Constructor and setup 

GameStateBattle::GameStateBattle(Game* game, bool isBossBattle)
:   topBarTextBackground(sf::Quads,4), 
    thingsEarnedBackground(sf::Quads,4),
    attackButton("Attack", {150.f, 800.f}, 30, game, sf::Color::White),
    skillButton("Skill",  {150.f, 840.f}, 30, game, sf::Color::White),
    itemButton("Item",   {150.f, 880.f}, 30, game, sf::Color::White),
    guardButton("Guard", {350.f, 800.f}, 30, game, sf::Color::White),
    escapeButton("Escape", {350.f, 840.f}, 30, game, sf::Color::White),
    backButton("Back", {350.f, 1000.f}, 30, game, sf::Color::White),
    quitButton("Quit", {400.f, 400.f}, 32, game, sf::Color::White),
    loadButton("Load Save", {400.f, 500.f}, 32, game, sf::Color::White),
    slot1("Slot 1", {450.f, 400.f}, 34, game, sf::Color::White),
    slot2("Slot 2", {450.f, 450.f}, 34, game, sf::Color::White),
    slot3("Slot 3", {450.f, 500.f}, 34, game, sf::Color::White)
{
    this->game = game;
    this->player = &game->player;
    font = this->game->font;

    // Setup battle text
    battleText.setFont(font);
    battleText.setString("* You feel like pressing enter to leave the battle state.");
    battleText.setCharacterSize(36);
    battleText.setFillColor(sf::Color::White);
    battleText.setPosition(850.f, 620.f);
    
    // Text box
    textBox.setSize({1750.f, 100.f});
    textBox.setOutlineThickness(2.f);
    textBox.setOutlineColor(sf::Color::Red);
    textBox.setFillColor(sf::Color::Black);
    textBox.setPosition(85.f, 620.f);
    
    // Enemy Background
    enemyBackground.setSize({1907.f, 550.f});
    enemyBackground.setOutlineThickness(2.f);
    enemyBackground.setOutlineColor(sf::Color::Red);
    enemyBackground.setPosition(5.f, 50.f);

    // Background 
    background.setSize(sf::Vector2f(game->window.getSize()));
    background.setFillColor(sf::Color::Black);

    // Texture loading (use texture manager)
    this->game->texmgr.loadTexture("enemy_bg", "./assets/backgrounds/border_dw_titan_base_0.png");
    enemyBackground.setTexture(&this->game->texmgr.getRef("enemy_bg"));
    auto ebSize = enemyBackground.getSize();
    enemyBackground.setTextureRect(sf::IntRect(0, 0, static_cast<int>(ebSize.x), static_cast<int>(ebSize.y)));

    // Player UI setup --------------------
    // UI position controls (change this)  
    float startX = 550.f;   // move left/right
    float startY = 890.f;   // move up/down
    float spacing = 220.f;  // horizontal spacing between characters
    
    party = {
        &this->game->player,
        &this->game->pmember2,
        &this->game->pmember3,
        &this->game->pmember4
    };

    // Prepare containers with party size to avoid UB / realloc issues
    size_t pcount = party.size();
    turnPortraitSprites.resize(pcount);
    turnPortraitBoxes.resize(pcount);

    playerBackgrounds.reserve(pcount);
    playerIcons.reserve(pcount);
    hpBars.reserve(pcount);
    mpBars.reserve(pcount);
    hpTexts.reserve(pcount);
    mpTexts.reserve(pcount);

    // initialize scale arrays to safe defaults
    portraitBaseScales.assign(pcount, 1.0f);
    turnPortraitBaseScales.assign(pcount, 1.0f);

    // Party setup
    for (size_t i = 0; i < party.size(); ++i) {
        sf::RectangleShape hpBar(sf::Vector2f(100.f, 10.f));
        sf::RectangleShape mpBar(sf::Vector2f(100.f, 10.f));
        sf::Text hpText;
        sf::Text mpText;
        sf::RectangleShape bgBox;
        sf::Sprite icon;

        hpText.setFont(font);
        mpText.setFont(font);
        hpText.setCharacterSize(24);
        mpText.setCharacterSize(24);

        // Red bg box
        bgBox.setFillColor(sf::Color(128, 0, 0, 200));
        bgBox.setOutlineColor(sf::Color::Red);
        bgBox.setOutlineThickness(2.f);

        // Bars
        hpBar.setFillColor(sf::Color::Green);
        mpBar.setFillColor(sf::Color::Blue);
    
        // UI positions
        float xOffset = startX + i * spacing;
        float yOffset = startY - 50.f;
        bgBox.setSize({210.f, 150.f}); // adjust the size of the red background
        bgBox.setPosition(xOffset - 20.f, yOffset - 20.f);
        basePositions.push_back(bgBox.getPosition());

        // Load and position the icon
        std::string texName, path;
        if (i == 0) {
            texName = "player_icon";
            path = "assets/player.png";
        } else {
            texName = "partymember" + std::to_string(i + 1) + "_icon";
            path = "assets/partymember" + std::to_string(i + 1) + ".png";
        }

        // Load it once through the texture manager (idempotent ideally)
        this->game->texmgr.loadTexture(texName, path);
        icon.setTexture(this->game->texmgr.getRef(texName), true);

        // Position and scale icon inside the red box
        float targetHeight = 70.f;
        float textureHeight = icon.getLocalBounds().height;
        float scale = (textureHeight > 0.f) ? (targetHeight / textureHeight) : 1.0f;
        icon.setScale(scale, scale);
        icon.setPosition(xOffset - 15.f, yOffset - 10.f);

        // store base scales consistently
        portraitBaseScales[i] = scale;

        // Store a copy for battle area icons
        playerBackgrounds.push_back(bgBox);
        playerIcons.push_back(icon);

        // Place bars below the icon
        hpBar.setPosition(xOffset, yOffset + 80.f);
        mpBar.setPosition(xOffset, yOffset + 100.f);

        // Place HP/MP text next to bars
        hpText.setPosition(xOffset + 110.f, yOffset + 70.f);
        mpText.setPosition(xOffset + 110.f, yOffset + 90.f);

        hpBars.push_back(hpBar);
        mpBars.push_back(mpBar);
        hpTexts.push_back(hpText);
        mpTexts.push_back(mpText);

        // Now set the portrait sprite for the turn panel 
        if (i < turnPortraitSprites.size()) {
            turnPortraitSprites[i].setTexture(this->game->texmgr.getRef(texName), true);
            turnPortraitSprites[i].setPosition(xOffset, yOffset);
        }
    }   

    // Turn Order UI ------------
    float panelW = 220.f;
    float panelH = 310.f;
    turnPanelBackground.setSize({panelW, panelH});
    turnPanelBackground.setFillColor(sf::Color(20,20,20, 200));
    turnPanelBackground.setOutlineColor(sf::Color::Red);
    turnPanelBackground.setOutlineThickness(2.f);

    // panel position
    float marginRight = 40.f;
    turnPanelBackground.setPosition(static_cast<float>(this->game->window.getSize().x) - panelW - marginRight, 735.f);

    // --- Compute initial turn order by AGI
    {
        std::vector<std::pair<int, Player*>> tmp;
        for (auto* p : party)
            tmp.emplace_back(p->getAGI(), p);

        std::sort(tmp.begin(), tmp.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });

        turnQueue.clear();
        for (auto& pr : tmp)
            turnQueue.push_back(pr.second);
    }

    // highlight first actor
    currentTurnIndex = 0;

    // enemy index
    currentEnemyIndex = getFirstLivingEnemy();

    // Music
    if (!currentMusic.openFromFile("./assets/music/normalbattle.mp3")) {
        std::cout << "Could not load music file" << std::endl;
    } else {
        currentMusic.setLoop(true);
        currentMusic.play();
    }
    

    buildSkillButtonsFor(&this->game->player);

    // Populate Items dynamically from inventory 
    itemButtons.clear();
    float itemY = 800.f;
    for (const auto& itemName : this->game->player.getInventory()) { 
        itemButtons.emplace_back(itemName.showName(), sf::Vector2f(150.f, itemY), 30, this->game, sf::Color::White);
        itemY += 70.f;
    }

    attackButton.enableHexBackground(true);
    skillButton.enableHexBackground(true);
    itemButton.enableHexBackground(true);
    guardButton.enableHexBackground(true);
    escapeButton.enableHexBackground(true);

    //Set Up results screen text
    topBarText.setFont(font);
    topBarText.setCharacterSize(75);
    topBarText.setString("Results \t\t\t\t\t\t\t\t\t\t\t\t\t\t Obtained the following:"); //the \t don't do anything these are just there now
    topBarText.setFillColor(sf::Color::Black);
    topBarText.setPosition(15.0f, 20.0f);

    topBarTextBackground[0].position = sf::Vector2f(0.f,0.f);
    topBarTextBackground[0].color= sf::Color(130,25,13);
    topBarTextBackground[1].position = sf::Vector2f(1920.f,0.f);
    topBarTextBackground[1].color= sf::Color(55,11,4);
    topBarTextBackground[2].position = sf::Vector2f(1920.f,150.f);
    topBarTextBackground[2].color= sf::Color(55,11,4);
    topBarTextBackground[3].position = sf::Vector2f(0,150.f);
    topBarTextBackground[3].color= sf::Color(130,25,13);

    thingsEarnedBackground[0].position = sf::Vector2f(0.f,200.f);
    thingsEarnedBackground[0].color= sf::Color(55,11,4);
    thingsEarnedBackground[1].position = sf::Vector2f(1920.f,200.f);
    thingsEarnedBackground[1].color= sf::Color(130,25,13);
    thingsEarnedBackground[2].position = sf::Vector2f(1920.f,400.f);
    thingsEarnedBackground[2].color= sf::Color(130,25,13);
    thingsEarnedBackground[3].position = sf::Vector2f(0,400.f);
    thingsEarnedBackground[3].color= sf::Color(55,11,4);

    totalEarnedExp.setFont(font);
    totalEarnedExpMessage = "EXP                                                                                                                                                                " + std::to_string(totalXpGained);  // + std::to_string(totalXpGained) or whatever it is
    totalEarnedExp.setString(totalEarnedExpMessage);
    totalEarnedExp.setCharacterSize(75);
    totalEarnedExp.setFillColor(sf::Color::Red);
    totalEarnedExp.setPosition(25.0f,250.0f);

    //set up all the necessary variables for displaying the player + party for results screen
    playerName.setFont(font);
    playerName.setString(this->game->player.getName());
    playerName.setCharacterSize(40);
    playerName.setFillColor(sf::Color(126, 17, 5));
    playerName.setPosition(200.0f, 490.f);

    playerLevel.setFont(font);
    playerLevel.setString("LV.  " + std::to_string(this->game->player.getLVL()));
    playerLevel.setCharacterSize(40);
    playerLevel.setFillColor(sf::Color(126, 17, 5));
    playerLevel.setPosition(850.0f, 490.f);

    nextLevelPlayerXp = this->game->player.getXpForNextLevel();
    nextLevelPlayer.setFont(font);
    nextLevelPlayer.setString("Next Exp:                                " + std::to_string(nextLevelPlayerXp));
    nextLevelPlayer.setCharacterSize(40);
    nextLevelPlayer.setPosition(1100.0f, 490.0f);

     
    playerXP = (float)this->game->player.getXp();
    expBarPlayer.setSize({1460.0f * ((float)playerXP/(float)nextLevelPlayerXp),10.0f});
    expBarPlayer.setFillColor(sf::Color(255,165,0));
    expBarPlayer.setPosition(175.0f, 545.0f);

    pmember2Name.setFont(font);
    pmember2Name.setString(this->game->pmember2.getName());
    pmember2Name.setCharacterSize(40);
    pmember2Name.setFillColor(sf::Color(126, 17, 5));
    pmember2Name.setPosition(200.0f, 590.f);

    pmember2Level.setFont(font);
    pmember2Level.setString("LV.  " + std::to_string(this->game->player.getLVL()));
    pmember2Level.setCharacterSize(40);
    pmember2Level.setFillColor(sf::Color(126, 17, 5));
    pmember2Level.setPosition(850.0f, 590.f);

    nextLevelPmember2Xp = this->game->pmember2.getXpForNextLevel();
    nextLevelPmember2.setFont(font);
    nextLevelPmember2.setString("Next Exp:                                " + std::to_string(nextLevelPmember2Xp));
    nextLevelPmember2.setCharacterSize(40);
    nextLevelPmember2.setPosition(1100.0f, 590.0f);

    pmember2XP = (float)this->game->pmember2.getXp();
    expBarPmember2.setSize({1460.0f * ((float)pmember2XP/(float)nextLevelPmember2Xp),10.0f});
    expBarPmember2.setFillColor(sf::Color(255,165,0));
    expBarPmember2.setPosition(175.0f, 645.0f);

    pmember3Name.setFont(font);
    pmember3Name.setString(this->game->pmember3.getName());
    pmember3Name.setCharacterSize(40);
    pmember3Name.setFillColor(sf::Color(126, 17, 5));
    pmember3Name.setPosition(200.0f, 690.f);

    pmember3Level.setFont(font);
    pmember3Level.setString("LV.  " + std::to_string(this->game->player.getLVL()));
    pmember3Level.setCharacterSize(40);
    pmember3Level.setFillColor(sf::Color(126, 17, 5));
    pmember3Level.setPosition(850.0f, 690.f);

    nextLevelPmember3Xp = this->game->pmember3.getXpForNextLevel();
    nextLevelPmember3.setFont(font);
    nextLevelPmember3.setString("Next Exp:                                " + std::to_string(nextLevelPmember3Xp));
    nextLevelPmember3.setCharacterSize(40);
    nextLevelPmember3.setPosition(1100.0f, 690.0f);

    pmember3XP = (float)this->game->pmember3.getXp();
    expBarPmember3.setSize({1460.0f * ((float)pmember3XP/(float)nextLevelPmember3Xp),10.0f});
    expBarPmember3.setFillColor(sf::Color(255,165,0));
    expBarPmember3.setPosition(175.0f, 745.0f);

    pmember4Name.setFont(font);
    pmember4Name.setString(this->game->pmember4.getName());
    pmember4Name.setCharacterSize(40);
    pmember4Name.setFillColor(sf::Color(126, 17, 5));
    pmember4Name.setPosition(200.0f, 790.f);

    pmember4Level.setFont(font);
    pmember4Level.setString("LV.  " + std::to_string(this->game->player.getLVL()));
    pmember4Level.setCharacterSize(40);
    pmember4Level.setFillColor(sf::Color(126, 17, 5));
    pmember4Level.setPosition(850.0f, 790.f);

    nextLevelPmember4Xp = this->game->pmember4.getXpForNextLevel();
    nextLevelPmember4.setFont(font);
    nextLevelPmember4.setString("Next Exp:                                " + std::to_string(nextLevelPmember4Xp));
    nextLevelPmember4.setCharacterSize(40);
    nextLevelPmember4.setPosition(1100.0f, 790.0f);

    pmember4XP = (float)this->game->pmember4.getXp();
    expBarPmember4.setSize({1460.0f * ((float)pmember4XP/(float)nextLevelPmember4Xp),10.0f});
    expBarPmember4.setFillColor(sf::Color(255,165,0));
    expBarPmember4.setPosition(175.0f, 845.0f);

    float BackgroundsOffsetY = 490.0f;
    float levelUpOffsetY = 455.0f;
    float expOffsetY = 545.0f;
    for (size_t i = 0; i < 4; i++){
        portraitBackgrounds[i].setSize({630.f,55.f});
        portraitBackgrounds[i].setFillColor(sf::Color(184, 62, 48));
        portraitBackgrounds[i].setPosition(175.0f, BackgroundsOffsetY + 100.0f*i);
        levelBackgrounds[i].setSize({830.f,55.f});
        levelBackgrounds[i].setFillColor(sf::Color(255,0,0,100));
        levelBackgrounds[i].setPosition(805.f, BackgroundsOffsetY + 100.0f*i);
        levelUpTexts[i].setFont(font);
        levelUpTexts[i].setFillColor(sf::Color(0,0,0,0));
        levelUpTexts[i].setCharacterSize(30);
        levelUpTexts[i].setString("Level Up!");
        levelUpTexts[i].setPosition(200.0f, levelUpOffsetY + 100.0f*i);
        expBarBackgrounds[i].setSize({1460,10.0f});
        expBarBackgrounds[i].setFillColor(sf::Color(51,51,51));
        expBarBackgrounds[i].setPosition(175.0f, expOffsetY + 100.0f*i);
    }
    // set up variables for the LEVEL UP! screen
    nameplateBackground.setSize({900.0f, 60.0f});
    nameplateBackground.setFillColor(sf::Color(184, 62, 48));
    nameplateBackground.setPosition(100.0f, 300.0f);

    nameplate.setSize({300.0f,58.0f});
    nameplate.setFillColor(sf::Color::Black);
    nameplate.setPosition(400.0f, 301.0f);

    nameOfCharacterForLevelUp.setFont(font);
    nameOfCharacterForLevelUp.setString("Placeholder Placeholder"); //adjust for who is leveling up in update
    nameOfCharacterForLevelUp.setCharacterSize(40);
    nameOfCharacterForLevelUp.setPosition(500.0f, 302.0f);
    
    for (size_t x = 0; x < 5; x++){
        statBoxes[x].setSize({100.0f, 40.0f});
        statBoxes[x].setFillColor(sf::Color(184,62,48));
        statBoxes[x].setPosition(100.0f, 365.0f + 43.0f*x );
        statBackgrounds[x].setSize({500.0f,40.0f});
        statBackgrounds[x].setFillColor(sf::Color::Transparent);
        statBackgrounds[x].setOutlineColor(sf::Color(255,0,0,50));
        statBackgrounds[x].setOutlineThickness(1.0f);
        statBackgrounds[x].setPosition(200.0f, 365.0f + 43.0f*x);
    }

    for (size_t x = 0; x < 2; x++){
        maxStatBoxes[x].setSize({100.0f, 40.0f});
        maxStatBoxes[x].setFillColor(sf::Color(184,62,48));
        maxStatBoxes[x].setPosition(710.0f, 365.0f + 43.0f*x);
        maxStatBackgrounds[x].setSize({190.0f,40.0f});
        maxStatBackgrounds[x].setFillColor(sf::Color::Transparent);
        maxStatBackgrounds[x].setOutlineColor(sf::Color(255,0,0,50));
        maxStatBackgrounds[x].setOutlineThickness(1.0f);
        maxStatBackgrounds[x].setPosition(810.0f, 365.0f + 43.0f*x);
    }
    
    strength.setFont(font);
    strength.setString("ST             " + std::to_string(strengthVal));
    strength.setFillColor(sf::Color::Green);
    strength.setPosition(100.0f, 365.0f);

    stBar.setSize({500.0f * strengthValPercent, 10.0f});
    stBar.setFillColor(sf::Color(255,165,0));
    stBar.setPosition(200.0f, 380.0f);

    vitality.setFont(font);
    vitality.setString("VI             " + std::to_string(vitalityVal));
    vitality.setPosition(100.0f, 408.0f);

    viBar.setSize({500.0f * vitalityValPercent, 10.0f});
    viBar.setFillColor(sf::Color(255,165,0));
    viBar.setPosition(200.0f, 423.0f);

    magic.setFont(font);
    magic.setString("MA             " + std::to_string(magicVal));
    magic.setFillColor(sf::Color::Green);
    magic.setPosition(100.0f, 451.0f);

    maBar.setSize({500.0f * magicValPercent, 10.0f});
    maBar.setFillColor(sf::Color(255,165,0));
    maBar.setPosition(200.0f, 466.0f);

    agility.setFont(font);
    agility.setString("AG             " + std::to_string(agilityVal));
    agility.setPosition(100.0f, 494.0f);

    agBar.setSize({500.0f * agilityValPercent, 10.0f});
    agBar.setFillColor(sf::Color(255,165,0));
    agBar.setPosition(200.0f, 509.0f);

    luck.setFont(font);
    luck.setString("LU             " + std::to_string(luckVal));
    luck.setPosition(100.0f, 537.0f);

    luBar.setSize({500.0f * luckValPercent, 10.0f});
    luBar.setFillColor(sf::Color(255,165,0));
    luBar.setPosition(200.0f, 552.0f);

    maxHp.setFont(font);
    maxHp.setString("Max HP                 " + std::to_string(maxHpVal) + "  ==>  " + std::to_string(recalculatedMaxHp));
    maxHp.setPosition(730.0f, 365.0f);

    maxMp.setFont(font);
    maxMp.setString("Max MP                 " + std::to_string(maxMpVal) + "  ==>  " + std::to_string(recalculatedMaxMp));
    maxMp.setPosition(730.0f,408.0f);

    pointsToDistributeTextbox.setSize({290.0f,122.0f});
    pointsToDistributeTextbox.setFillColor(sf::Color::Transparent);
    pointsToDistributeTextbox.setOutlineColor(sf::Color(255,0,0,50));
    pointsToDistributeTextbox.setOutlineThickness(1.0f);
    pointsToDistributeTextbox.setPosition(710.0f, 455.0f);

    distributionText.setFont(font);
    distributionText.setPosition(710.0f, 455.0f);
    distributionText.setString("Distribute points.\n" + std::to_string(skillPoints) + " points remaining.");

    for (size_t x = 1; x < 9; x++){
        skillNamesForResults[x].setFont(font);
        skillNamesForResults[x].setPosition(200.0f, 540.0f + (50.0f * x-1));
    }

    levelUpBooleanMap[&this->game->player] = false;
    levelUpBooleanMap[&this->game->pmember2] = false;
    levelUpBooleanMap[&this->game->pmember3] = false;
    levelUpBooleanMap[&this->game->pmember4] = false;
    levelUpIterator = levelUpBooleanMap.begin();
    
    // Game over initialization
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(400.f, 200.f); // adjust for center
    gameOver = false;
    gameOverMenuState = GameOverMenuState::Main;

    enemyTurnPending = false;
    enemyTurnTimer = 0.f;
    pendingEnemy = nullptr;
}

void GameStateBattle::displayResultsScreen(){
    this->game->window.clear();
    if (!reuseArrays){ //doing it like this so we reset positions one time.
        float iconOffsetY = 494.0f;
        float backgroundOffsetY = 493.0f;
        playerIcons[0].setScale(1.22,1.22);
        playerIcons[1].setScale(0.65,0.65);
        playerIcons[2].setScale(1.6,1.6);
        playerIcons[3].setScale(1.72,1.72);
        for (size_t i = 0; i < 4; i++){ //probably want to make this based off party size
        playerBackgrounds[i].setSize({231.0f,52.0f});
        playerBackgrounds[i].setFillColor(sf::Color(58, 7, 1));
        playerBackgrounds[i].setOutlineThickness(0.0f);
        playerBackgrounds[i].setPosition(570.0f,backgroundOffsetY + 100.0f * i);
        playerIcons[i].setPosition(620.0f, iconOffsetY + 100.0f * i);
        }
        reuseArrays = true;
    }
    for (auto& background : portraitBackgrounds){
        this->game->window.draw(background);
    }
    for (auto& background : levelBackgrounds){
        this->game->window.draw(background);
    }
    for (auto& background : expBarBackgrounds){
        this->game->window.draw(background);
    }
    for (auto& text : levelUpTexts){
        this->game->window.draw(text);
    }
    for (auto&  background : playerBackgrounds){
        this->game->window.draw(background);
    }
    for (auto& icon : playerIcons){
        this->game->window.draw(icon);
    }
 
     
    this->game->window.draw(topBarTextBackground);
    this->game->window.draw(topBarText);
    this->game->window.draw(thingsEarnedBackground);
    this->game->window.draw(totalEarnedExp);
    this->game->window.draw(playerName);
    this->game->window.draw(pmember2Name);
    this->game->window.draw(pmember3Name);
    this->game->window.draw(pmember4Name);
    this->game->window.draw(playerLevel);
    this->game->window.draw(nextLevelPlayer);
    this->game->window.draw(pmember2Level);
    this->game->window.draw(nextLevelPmember2);
    this->game->window.draw(pmember3Level);
    this->game->window.draw(nextLevelPmember3);
    this->game->window.draw(pmember4Level);
    this->game->window.draw(nextLevelPmember4);
    this->game->window.draw(expBarPlayer);
    this->game->window.draw(expBarPmember2);
    this->game->window.draw(expBarPmember3);
    this->game->window.draw(expBarPmember4);
  
}


void GameStateBattle::displayLevelUpScreen(){
    this->game->window.clear();
    if (!reuseTextforLevelUp){
        topBarText.setString("LEVEL UP!");
        topBarText.setFillColor(sf::Color::White);
        topBarText.setPosition(850.0f, 20.0f);
    }
    
    this->game->window.clear(sf::Color(0,0,0));
    this->game->window.draw(topBarTextBackground);
    this->game->window.draw(topBarText);
    this->game->window.draw(nameplateBackground);
    this->game->window.draw(nameplate);
    this->game->window.draw(nameOfCharacterForLevelUp);
    for (auto& statbox : statBoxes){
        this->game->window.draw(statbox);
    }
    for (auto& backgrounds : statBackgrounds){
        this->game->window.draw(backgrounds);
    }
    for (auto& statbox : maxStatBoxes){
        this->game->window.draw(statbox);
    }
    for (auto& backgrounds : maxStatBackgrounds){
        this->game->window.draw(backgrounds);
    }
  
    this->game->window.draw(pointsToDistributeTextbox);
    this->game->window.draw(distributionText);
    this->game->window.draw(strength);
    this->game->window.draw(vitality);
    this->game->window.draw(magic);
    this->game->window.draw(agility);
    this->game->window.draw(luck);
    this->game->window.draw(maxHp);
    this->game->window.draw(maxMp);
    this->game->window.draw(stBar);
    this->game->window.draw(viBar);
    this->game->window.draw(maBar);
    this->game->window.draw(agBar);
    this->game->window.draw(luBar);
    
      if (printSkillNames){
        for (auto& skillName : skillNamesForResults){
            this->game->window.draw(skillName);
        }
    }
}

// Draw 
void GameStateBattle::draw(const float dt) {
    // Always clear the window first
    this->game->window.clear(sf::Color::Black);

    // Battle finished / results screen
    if (battleOver && !levelUpTime) {
        if (!playResultsMusic) {
            currentMusic.stop();
            if (!currentMusic.openFromFile("./assets/music/battleresults.mp3"))
                std::cout << "Could not load music file" << std::endl;
            else {
                currentMusic.setLoop(true);
                currentMusic.play();
                playResultsMusic = true;
                }
            }
         
         displayResultsScreen();
         this->game->window.display();     
         return;
    }
    else if (battleOver && levelUpTime){
        displayLevelUpScreen();
        this->game->window.display(); // commit the frame
        return;
    } 
    else{

    // --- Draw normal battle scene
    this->game->window.draw(background);
    this->game->window.draw(enemyBackground);
    this->game->window.draw(textBox);
    this->game->window.draw(battleText);

    // Draw party
    for (size_t i = 0; i < party.size(); ++i) {
        if (i >= playerBackgrounds.size() || i >= playerIcons.size() || 
            i >= hpBars.size() || i >= mpBars.size())
            continue;

        this->game->window.draw(playerBackgrounds[i]);
        this->game->window.draw(playerIcons[i]);
        this->game->window.draw(hpBars[i]);
        this->game->window.draw(mpBars[i]);
        this->game->window.draw(hpTexts[i]);
        this->game->window.draw(mpTexts[i]);
    }

    // Draw enemies
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (i < enemyBaseScales.size()) {
            float base = enemyBaseScales[i];
            enemySprites[i].setScale((static_cast<int>(i) == currentEnemyIndex) ? base * 1.15f : base,
                                     (static_cast<int>(i) == currentEnemyIndex) ? base * 1.15f : base);
        }

        this->game->window.draw(enemySprites[i]);

        // Highlight active enemy
        if (!turnQueue.empty() && turnQueue.front() == &enemies[i]) {
            sf::RectangleShape highlightBox;
            sf::FloatRect bounds = enemySprites[i].getGlobalBounds();
            highlightBox.setSize(sf::Vector2f(bounds.width + 10.f, bounds.height + 10.f));
            highlightBox.setPosition(bounds.left - 5.f, bounds.top - 5.f);
            highlightBox.setFillColor(sf::Color(255, 255, 255, 50));
            highlightBox.setOutlineColor(sf::Color::Red);
            highlightBox.setOutlineThickness(2.f);
            this->game->window.draw(highlightBox);
        }

        // Enemy HP bar
        if (!enemies[i].isDead()) {
            float hpPerc = (float)enemies[i].getHP() / (float)enemies[i].getmaxHP();
            sf::FloatRect eBounds = enemySprites[i].getGlobalBounds();

            sf::RectangleShape bg(sf::Vector2f(eBounds.width, 6.f));
            bg.setFillColor(sf::Color(0, 0, 0, 180));
            bg.setPosition(eBounds.left, eBounds.top - 12.f);

            sf::RectangleShape fg(sf::Vector2f(eBounds.width * hpPerc, 6.f));
            fg.setFillColor(sf::Color(200, 40, 40));
            fg.setPosition(eBounds.left, eBounds.top - 12.f);

            this->game->window.draw(bg);
            this->game->window.draw(fg);
        }
    }

    // Game Over buttons
    if (gameOver) {
        this->game->window.clear(sf::Color::Black);
        this->game->window.draw(gameOverText);
        if (!loadMenuActive) {
            quitButton.draw(this->game->window);
            loadButton.draw(this->game->window);

            if (quitButton.isHovered(this->game->window))
                this->game->window.draw(quitButton.getUnderline());
            if (loadButton.isHovered(this->game->window))
                this->game->window.draw(loadButton.getUnderline());
        } else {
            // Load slots
            slot1.draw(this->game->window);
            slot2.draw(this->game->window);
            slot3.draw(this->game->window);
            backButton.draw(this->game->window);

            if (slot1.isHovered(this->game->window)) this->game->window.draw(slot1.getUnderline());
            if (slot2.isHovered(this->game->window)) this->game->window.draw(slot2.getUnderline());
            if (slot3.isHovered(this->game->window)) this->game->window.draw(slot3.getUnderline());
            if (backButton.isHovered(this->game->window)) this->game->window.draw(backButton.getUnderline());
        }
        this->game->window.display(); // update the window
        return; // skip the rest of battle drawing
    }

    // Turn panel and portraits
    this->game->window.draw(turnPanelBackground);

    for (size_t i = 0; i < turnPortraitBoxes.size(); ++i) {
        if (turnPortraitBoxes[i].getSize().x > 0.f) {
            this->game->window.draw(turnPortraitBoxes[i]);
            if (i < turnPortraitSprites.size() && turnPortraitSprites[i].getTexture() != nullptr)
                this->game->window.draw(turnPortraitSprites[i]);
        }
    }

    // --- Enemy names
    for (size_t i = 0; i < enemyNameBackgrounds.size(); ++i) {
        this->game->window.draw(enemyNameBackgrounds[i]);
        if (i < turnEnemyNames.size())
            this->game->window.draw(turnEnemyNames[i]);
    }

    // Floating damage popups
    for (auto& dp : damagePopups) {
        this->game->window.draw(dp.text);
    }

    // Battle menu buttons
    if (currentMenuState == BattleMenuState::Main) {
        attackButton.draw(this->game->window);
        skillButton.draw(this->game->window);
        itemButton.draw(this->game->window);
        guardButton.draw(this->game->window);
        escapeButton.draw(this->game->window);
    } else if (currentMenuState == BattleMenuState::Skill) {
        for (auto& b : skillButtons) b.draw(this->game->window);
        backButton.draw(this->game->window);
    } else if (currentMenuState == BattleMenuState::Item) {
        for (auto& b : itemButtons) b.draw(this->game->window);
        backButton.draw(this->game->window);
    }

    // Commit the frame
    this->game->window.display();
}
}


// Update
void GameStateBattle::update(const float dt) {
    if (battleOver && !levelUpTime){
        if (totalXpGained > 0){
            XPdecrementer++;
            float addedXP = 1;
            playerXP += addedXP;
            if (playerXP >= nextLevelPlayerXp){ //since everyone gets the same amount of xp, we are just using the player's xp to see if everyone levels up or not (we can change this later if we want)
                if (!levelupflags){
                    levelUpTexts[0].setFillColor(sf::Color(0,255,0,255));
                    levelUpBooleanMap[&this->game->player] = true;

                    levelUpTexts[1].setFillColor(sf::Color(0,255,0,255));
                    levelUpBooleanMap[&this->game->pmember2] = true;

                    levelUpTexts[2].setFillColor(sf::Color(0,255,0,255));
                    levelUpBooleanMap[&this->game->pmember3] = true;

                    levelUpTexts[3].setFillColor(sf::Color(0,255,0,255));
                    levelUpBooleanMap[&this->game->pmember4] = true;
                    levelupflags = true;
                }
                this->game->player.levelUp();
                playerLevel.setString("LV.  " + std::to_string(this->game->player.getLVL()));
                playerXP = 0;
                nextLevelPlayerXp = this->game->player.getXpForNextLevel();
                nextLevelPlayer.setString("Next Exp:                                " + std::to_string(nextLevelPlayerXp));
            
                this->game->pmember2.levelUp();
                pmember2XP = 0;
                pmember2Level.setString("LV.  " + std::to_string(this->game->pmember2.getLVL()));
                nextLevelPmember2Xp =  this->game->pmember2.getXpForNextLevel();
                nextLevelPmember2.setString("Next Exp:                                " + std::to_string(nextLevelPmember2Xp));

                this->game->pmember3.levelUp();
                pmember3Level.setString("LV.  " + std::to_string(this->game->pmember3.getLVL()));
                pmember3XP = 0;
                nextLevelPmember3Xp =  this->game->pmember3.getXpForNextLevel();
                nextLevelPmember3.setString("Next Exp:                                " + std::to_string(nextLevelPmember3Xp));

                this->game->pmember4.levelUp();
                pmember4Level.setString("LV.  " + std::to_string(this->game->pmember4.getLVL()));
                pmember4XP = 0;
                nextLevelPmember4Xp = this->game->pmember4.getXpForNextLevel();
                nextLevelPmember4.setString("Next Exp:                                " + std::to_string(nextLevelPmember4Xp));
                XPdecrementer = 0; //very bandaid fix but we reset the decrementer whenever we level up so it will correctly decrement the exp.
                tempSkillPoints += 8; // on every level up, you gain 8 points you can distribute freely (this is probably not the best for balance)
                skillPoints = tempSkillPoints;
                distributionText.setString("Distribute points.\n" + std::to_string(skillPoints) + " points remaining.");
            }
            float xpPercent = (float)playerXP/(float)nextLevelPlayerXp;
            expBarPlayer.setSize({1460.0f * xpPercent,10.0f});
            nextLevelPlayer.setString("Next Exp:                                " + std::to_string(nextLevelPlayerXp - XPdecrementer));

            expBarPmember2.setSize({1460.0f * xpPercent,10.0f});
            nextLevelPmember2.setString("Next Exp:                                " + std::to_string(nextLevelPmember2Xp - XPdecrementer));

            expBarPmember3.setSize({1460.0f * xpPercent,10.0f});
            nextLevelPmember3.setString("Next Exp:                                " + std::to_string(nextLevelPmember3Xp - XPdecrementer));

            expBarPmember4.setSize({1460.0f * xpPercent,10.0f});
            nextLevelPmember4.setString("Next Exp:                                " + std::to_string(nextLevelPmember4Xp - XPdecrementer));

            totalXpGained -= addedXP; //for simplicity, everyone gets the same amount of xp every battle
        }
        else distributionFinished = true;
    } 
    else if (battleOver && levelUpTime){
        if (levelUpIterator != levelUpBooleanMap.end()){
            character = levelUpIterator->first;
            bool leveledUp = levelUpIterator->second;
            if (leveledUp && !statsSet){
                strengthVal = character->getSTR();
                strengthValPercent = (float)strengthVal / 99;

                vitalityVal = character->getVIT();
                vitalityValPercent = (float)vitalityVal / 99;

                magicVal = character->getMAG();
                magicValPercent = (float)magicVal / 99;

                agilityVal = character->getAGI();
                agilityValPercent = (float)agilityVal / 99;

                luckVal = character->getLU();
                luckValPercent = (float)luckVal / 99;

                maxHpVal = character->getmaxHP();
                recalculatedMaxHp = maxHpVal;

                maxMpVal = character->getmaxMP();
                recalculatedMaxMp = maxMpVal;

                nameOfCharacterForLevelUp.setString(character->getName());
                strength.setString("ST             " + std::to_string(strengthVal));
                stBar.setSize({500.0f * strengthValPercent, 10.0f});    
                vitality.setString("VI             " + std::to_string(vitalityVal));
                viBar.setSize({500.0f * vitalityValPercent, 10.0f});
                magic.setString("MA             " + std::to_string(magicVal));
                maBar.setSize({500.0f * magicValPercent, 10.0f});
                agility.setString("AG             " + std::to_string(agilityVal));
                agBar.setSize({500.0f * agilityValPercent, 10.0f});
                luck.setString("LU             " + std::to_string(luckVal));
                luBar.setSize({500.0f * luckValPercent, 10.0f});
                maxHp.setString("Max HP                 " + std::to_string(maxHpVal) + "  ==>  " + std::to_string(recalculatedMaxHp));
                maxMp.setString("Max MP                 " + std::to_string(maxMpVal) + "  ==>  " + std::to_string(recalculatedMaxMp));

                 for (size_t x = 1; x < 9; x++){
                    auto* skill = character->getSkillsList()[x];
                    if(skill && skill->getName() != "EMPTY SLOT" && skill->getUnlockLevel() <= character->getLVL()){
                        skillNamesForResults[x-1].setString(skill->getName());
                    }
                statsSet = true;

            }}
            strength.setFillColor(sf::Color::White);
            vitality.setFillColor(sf::Color::White);
            magic.setFillColor(sf::Color::White);
            agility.setFillColor(sf::Color::White);
            luck.setFillColor(sf::Color::White);
              switch ((levelUpAttributeIndex % 5 + 5) % 5){
                        case (0):
                            strength.setFillColor(sf::Color::Green);
                            break;
                        case (1):
                            vitality.setFillColor(sf::Color::Green);
                            break;
                        case (2):
                            magic.setFillColor(sf::Color::Green);
                            break;
                        case (3):
                            agility.setFillColor(sf::Color::Green);
                            break;
                        case (4):
                            luck.setFillColor(sf::Color::Green);
                            break;
              }
              std::cout << (levelUpAttributeIndex % 5 + 5) % 5 << std::endl;
            printSkillNames = true;
        }
    }

    else{
    // update damage popups (position + life)
    for (auto it = damagePopups.begin(); it != damagePopups.end();) {
        it->life -= dt;
        it->text.move(it->velocity * dt);
        if (it->life <= 0.f) it = damagePopups.erase(it);
        else ++it;
    }

    for (size_t i = 0; i < party.size(); ++i) {
        auto* p = party[i];
        if (!p) continue;
        int maxHP = p->getmaxHP();
        int maxMP = p->getmaxMP();

        float hpPercent = (maxHP > 0) ? static_cast<float>(p->getHP()) / static_cast<float>(maxHP) : 0.f;
        float mpPercent = (maxMP > 0) ? static_cast<float>(p->getMP()) / static_cast<float>(maxMP) : 0.f;
    
        if (i < hpBars.size()) hpBars[i].setSize({100.f * hpPercent, 10.f});
        if (i < mpBars.size()) mpBars[i].setSize({100.f * mpPercent, 10.f});
    
        if (i < hpTexts.size()) hpTexts[i].setString(std::to_string(p->getHP()) + "/" + std::to_string(maxHP));
        if (i < mpTexts.size()) mpTexts[i].setString(std::to_string(p->getMP()) + "/" + std::to_string(maxMP));
    }   

    // Spawn enemies if empty
    if (enemies.empty()) {
        // use mt19937 for everything consistent
        std::random_device rd;
        std::mt19937 gen(rd());
        enemyBaseScales.clear();
        int count = std::uniform_int_distribution<>(1, 4)(gen); // 1–4 enemies
        enemies = loadRandomEnemies(count);

        // Reserve to avoid reallocation invalidating addresses
        enemySprites.clear();
        enemyTextures.clear();
        enemySprites.reserve(enemies.size());
        enemyBaseScales.reserve(enemies.size());

        // Load textures via texmgr and position sprites
        for (size_t idx = 0; idx < enemies.size(); ++idx) {
            const auto& e = enemies[idx];
            std::string path = e.getSpriteLocation();
            std::string texName = "enemy_" + std::to_string(idx) + "_" + std::to_string(std::hash<std::string>{}(path));

            this->game->texmgr.loadTexture(texName, path);
            sf::Sprite spr;
            spr.setTexture(this->game->texmgr.getRef(texName), true);

            // randomize positions using mt19937
            int xpos = std::uniform_int_distribution<>(600, 1000)(gen);
            int ypos = std::uniform_int_distribution<>(200, 300)(gen);
            spr.setPosition(static_cast<float>(xpos), static_cast<float>(ypos));

            // compute and store base scale to keep sprite sizes stable
            float textureHeight = spr.getLocalBounds().height;
            float targetHeight = 150.f; // chosen target height for enemy images
            float baseScale = (textureHeight > 0.f) ? (targetHeight / textureHeight) : 1.0f;
            spr.setScale(baseScale, baseScale);
            enemyBaseScales.push_back(baseScale);

            enemySprites.push_back(spr);
        }

        // Populate turn queue by AGI: players then enemies (store pointers to elements)
        turnQueue.clear();
        for (auto* p : party) turnQueue.push_back(p);

        // ensure enemies vector will not be reallocated later unexpectedly
        for (auto& e : enemies) turnQueue.push_back(&e);

        std::sort(turnQueue.begin(), turnQueue.end(),
        [](Player* a, Player* b) { return a->getAGI() > b->getAGI(); });

        // ensure currentEnemyIndex is valid
        currentEnemyIndex = getFirstLivingEnemy();
    }

    // Highlight the active player's UI box
    Player* active = nullptr;
    if (!turnQueue.empty()) active = turnQueue.front();

    updateTurnPanel();

    for (size_t i = 0; i < party.size(); ++i) {
        float raise = 0.f;
        if (active == party[i]) {
            raise = -16.f;
            if (i < playerBackgrounds.size()) {
                playerBackgrounds[i].setOutlineColor(sf::Color::Green);
                playerBackgrounds[i].setOutlineThickness(3.f);
            }
        } else {
            if (i < playerBackgrounds.size()) {
                playerBackgrounds[i].setOutlineColor(sf::Color::Red);
                playerBackgrounds[i].setOutlineThickness(2.f);
            }           
        }

        if (i < basePositions.size()) {
            sf::Vector2f base = basePositions[i];
            if (i < playerBackgrounds.size()) playerBackgrounds[i].setPosition(base.x, base.y + raise);
            if (i < playerIcons.size()) playerIcons[i].setPosition(playerIcons[i].getPosition().x, base.y - 10.f + raise);
            if (i < hpBars.size()) hpBars[i].setPosition(hpBars[i].getPosition().x, base.y + 80.f + raise);
            if (i < mpBars.size()) mpBars[i].setPosition(mpBars[i].getPosition().x, base.y + 100.f + raise);
            if (i < hpTexts.size()) hpTexts[i].setPosition(hpTexts[i].getPosition().x, base.y + 70.f + raise);
            if (i < mpTexts.size()) mpTexts[i].setPosition(mpTexts[i].getPosition().x, base.y + 90.f + raise);
        }
    }

    // Button Highlights
    attackButton.setHighlight(attackButton.isHovered(this->game->window));
    skillButton.setHighlight(skillButton.isHovered(this->game->window));
    itemButton.setHighlight(itemButton.isHovered(this->game->window));
    guardButton.setHighlight(guardButton.isHovered(this->game->window));
    escapeButton.setHighlight(escapeButton.isHovered(this->game->window));

    // dynamically update skill descriptions
    if (currentMenuState == BattleMenuState::Skill) {
        for (size_t i = 0; i < skillButtons.size(); ++i) {
            if (skillButtons[i].isHovered(this->game->window)) {
                const Skill* s = this->game->player.getSkillPtr(
                    skillButtons[i].getText(),
                    this->game->skillMasterList
                );
                if (s) {
                    battleText.setString(
                        s->getName() + "\n" +
                        s->getDescription() + "\nMP Cost: " +
                        std::to_string(s->getMpCost())
                    );
                }
            }
        }
    }

    // --- Skip dead actors at the start of their turn ---
    while (!turnQueue.empty()) {
        Player* front = turnQueue.front();
        bool isEnemy = (std::find(party.begin(), party.end(), front) == party.end());
        bool isDead = isEnemy ? static_cast<NPC*>(front)->isDead() : (front->getHP() <= 0);

        if (isDead) {
            turnQueue.pop_front();
            turnQueue.push_back(front);
            if (front) front->decrementBuffTurns();
        } else break; // stop at first living actor
    }

    // ---- Enemy AI turns ----
    if (!turnQueue.empty()) {
        Player* actor = turnQueue.front();
        bool actorIsEnemy = (std::find(party.begin(), party.end(), actor) == party.end());

        if (actorIsEnemy) {
            if (!enemyTurnPending) {
                pendingEnemy = actor;
                enemyTurnPending = true;
                enemyTurnTimer = ENEMY_TURN_DELAY;
            } else {
                enemyTurnTimer -= dt;
                if (enemyTurnTimer <= 0.f && pendingEnemy != nullptr) {
                    NPC* actingEnemy = nullptr;

                    // Find actual NPC* in enemies vector
                    for (auto& e : enemies) {
                        if (&e == pendingEnemy) { actingEnemy = &e; break; }
                    }

                    if (actingEnemy && !actingEnemy->isDead()) {
                        // --- Build list of living targets once ---
                        std::vector<Player*> livingTargets;
                        for (auto* p : party) if (p && p->getHP() > 0) livingTargets.push_back(p);

                        if (!livingTargets.empty()) {
                            std::uniform_int_distribution<> dis(0, livingTargets.size() - 1);
                            Player* target = livingTargets[dis(globalRng())];

                            // --- Enemy skill selection ---
                            const std::vector<std::string>& enemySkillNames = actingEnemy->getSkillNames();
                            const Skill* chosenSkill = nullptr;
                            std::vector<const Skill*> buffSkills;
                            std::vector<const Skill*> damageSkills;
                            std::vector<const Skill*> healSkills;

                            for (const auto& sname : enemySkillNames) {
                                const Skill* sc = actingEnemy->getSkillPtr(sname, this->game->skillMasterList);
                                if (!sc) continue;

                                std::string t = sc->getType();
                                bool isBuff =
                                    (t == "Damage Amp" || t == "Damage Resist" ||
                                    t == "Hit Evade Boost" || t == "Hit Evade Reduction");

                                if (isBuff) buffSkills.push_back(sc);
                                else if (t == "Healing") healSkills.push_back(sc);
                                else damageSkills.push_back(sc);
                            }

                            std::uniform_real_distribution<float> roll(0.f, 1.f);
                            bool usedBuff = false;

                            // 25% chance to use buff
                            if (!buffSkills.empty() && roll(globalRng()) < 0.25f) {
                                chosenSkill = buffSkills[rand() % buffSkills.size()];
                                usedBuff = true;

                                // Apply buff effects
                                if (chosenSkill->getType() == "Damage Amp")
                                    actingEnemy->addBuff("Damage Amp", 1.25f, 3, true, false);
                                else if (chosenSkill->getType() == "Damage Resist")
                                    actingEnemy->addBuff("Damage Resist", 0.8f, 3, false, true);
                                else if (chosenSkill->getType() == "Hit Evade Boost") {
                                    actingEnemy->addBuff("Hit Boost", 1.15f, 3, true, false);
                                    actingEnemy->addBuff("Evade Boost", 1.15f, 3, false, false);
                                }

                                battleText.setString(actingEnemy->getName() + " used " + chosenSkill->getName() + "!");

                                int idx = getEnemyIndex(actingEnemy);
                                sf::Vector2f pos = enemySprites[idx].getPosition();
                                DamagePopup dp;
                                dp.text.setFont(font);
                                dp.text.setCharacterSize(24);
                                dp.text.setString("BUFF!");
                                dp.text.setFillColor(sf::Color(255,200,50));
                                dp.text.setPosition(pos - sf::Vector2f(0.f, 40.f));
                                dp.velocity = sf::Vector2f(0.f, -25.f);
                                dp.life = 1.2f;
                                damagePopups.push_back(dp);
                            }

                            // If not buffing, pick damaging skill
                            if (!usedBuff) {
                                if (!damageSkills.empty()) chosenSkill = damageSkills[rand() % damageSkills.size()];
                                else chosenSkill = findSkillByName(this->game, "Attack"); // fallback
                            }

                            // --- Apply damage skill ---
                            if (!usedBuff && chosenSkill) {
                                int totalDmg = 0;
                                for (auto* t : (chosenSkill->getIsSingleTarget() ? std::vector<Player*>{target} : livingTargets)) {
                                    int damage = 0;
                                    bool crit = false;
                                    float critChance = chosenSkill->getCritRate();
                                    if (critChance <= 0.f) critChance = 0.05f;

                                    std::uniform_real_distribution<float> cr(0.f,1.f);
                                    crit = (cr(globalRng()) < critChance);

                                    float elementMul = getElementMultiplier(t, chosenSkill);
                                    bool isPhysical = (chosenSkill->getType().find("Physical") != std::string::npos);

                                    if (isPhysical) {
                                        int baseAtk = chosenSkill->getBaseAtk();
                                        if (baseAtk <= 0) baseAtk = 10 + actingEnemy->getSTR()*2;
                                        float scalar = (chosenSkill->getDamageAmp() > 0.f) ? chosenSkill->getDamageAmp() : 1.0f;
                                        damage = actingEnemy->physATK(scalar, baseAtk, crit);
                                        damage = static_cast<int>(std::round(damage * t->getIncomingDamageMultiplier()));
                                    } else {
                                        int baseAtk = chosenSkill->getBaseAtk();
                                        int limit = chosenSkill->getLimit();
                                        int corr  = chosenSkill->getCorrection();
                                        bool isWeak = (elementMul > 1.0f);
                                        damage = actingEnemy->magATK(1.0f, baseAtk, limit, corr, isWeak);
                                        damage = static_cast<int>(std::round(damage * elementMul * t->getIncomingDamageMultiplier()));
                                    }

                                    t->takeDamage(damage);
                                    totalDmg += damage;

                                    int pIdx = std::distance(party.begin(), std::find(party.begin(), party.end(), t));
                                    DamagePopup dp;
                                    dp.text.setFont(font);
                                    dp.text.setCharacterSize(28);
                                    std::string label = std::string(crit ? "CRIT " : "")
                                    + (elementMul > 1 ? "WEAK " : "")
                                    + (elementMul < 1 ? "RESIST " : "")
                                    + std::to_string(damage);                
                                    dp.text.setString(label);
                                    sf::Vector2f pos = playerIcons[pIdx].getPosition();
                                    dp.text.setPosition(pos - sf::Vector2f(0.f,40.f));
                                    dp.velocity = sf::Vector2f(0.f, -30.f);
                                    dp.life = 1.f;
                                    damagePopups.push_back(dp);
                                }
                                battleText.setString(actingEnemy->getName() + " used " + chosenSkill->getName() + " for " + std::to_string(totalDmg) + " total damage!");
                            }

                            cleanupDeadEnemies();
                        }
                    }

                    // --- END OF ACTOR TURN ---
                    turnQueue.pop_front();
                    turnQueue.push_back(actor);  // rotate
                    if (actor) actor->decrementBuffTurns();

                    enemyTurnPending = false;
                    pendingEnemy = nullptr;
                }
            }
        }
    }

    if (player->getHP() <= 0 && !gameOver) {
        gameOver = true;
        currentMusic.stop();
    }
}
}

// Input Handling
void GameStateBattle::handleInput() {
    sf::Event event;
    while (this->game->window.pollEvent(event)) {

        // --- Close window
        if (event.type == sf::Event::Closed) { 
            this->game->window.close();
            return;
        }

        // --- Game Over input
        if (gameOver) {

            // --- Mouse click events for Game Over
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                if (!loadMenuActive) { 
                    // Main Game Over buttons
                    if (quitButton.wasClicked(this->game->window)) {
                        this->game->window.close();
                        return;
                    }
                    else if (loadButton.wasClicked(this->game->window)) {
                        loadMenuActive = true; // show slot menu
                    }
                } 
                else { 
                    // Slot menu active
                    if (slot1.wasClicked(this->game->window)) {
                        this->game->player.loadFromFile("save1.json", this->game->skillMasterList);
                        loadMenuActive = false;
                        gameOver = false; // optionally reset game over state
                        this->game->requestChange(std::make_unique<GameStateEditor>(this->game, false));
                        return;
                    }
                    else if (slot2.wasClicked(this->game->window)) {
                        this->game->player.loadFromFile("save2.json", this->game->skillMasterList);
                        loadMenuActive = false;
                        gameOver = false;
                        this->game->requestChange(std::make_unique<GameStateEditor>(this->game, false));
                        return;
                    }
                    else if (slot3.wasClicked(this->game->window)) {
                        this->game->player.loadFromFile("save3.json", this->game->skillMasterList);
                        loadMenuActive = false;
                        gameOver = false;
                        this->game->requestChange(std::make_unique<GameStateEditor>(this->game, false));
                        return;
                    }
                    else if (backButton.wasClicked(this->game->window)) {
                        loadMenuActive = false; // return to main Game Over buttons
                    }
                }

            }

            continue; // skip normal battle input while gameOver
        }

        // --- Key press events
        if (event.type == sf::Event::KeyPressed) {
             if (event.key.code == sf::Keyboard::Space) {
                 if (battleOver && distributionFinished && levelUpTime){
                    if (levelUpIterator == levelUpBooleanMap.end()){
                        this->game->requestPop();
                        return;
                    }
                    if (levelUpIterator != levelUpBooleanMap.end() && skillPoints == 0){
                        skillPoints = tempSkillPoints;
                        character->statUp(strengthVal, vitalityVal, magicVal, agilityVal, luckVal);
                        ++levelUpIterator; 
                        statsSet = false;
                    }
                }
                 if (battleOver && distributionFinished){ //use this to check if anyone leveld up then pop.
                    levelUpTime = true;
                   }
                if (!turnQueue.empty()) {
                    Player* front = turnQueue.front();
                    turnQueue.pop_front();
                    turnQueue.push_back(front);
                    if (front) front->decrementBuffTurns();
                }
            }
            else if (event.key.code == sf::Keyboard::Right) {
                if (!enemies.empty()) {
                    int next = getNextLivingEnemy(currentEnemyIndex);
                    if (next >= 0) currentEnemyIndex = next;
                }
            }
            else if (event.key.code == sf::Keyboard::Left) {
                if (!enemies.empty()) {
                    int prev = getPrevLivingEnemy(currentEnemyIndex);
                    if (prev >= 0) currentEnemyIndex = prev;
                }
               
                if (!turnQueue.empty()) {
                    Player* front = turnQueue.front();
                    turnQueue.pop_front();
                    turnQueue.push_back(front);
                    if (front) front->decrementBuffTurns();
                }
            }
            else if (event.key.code == sf::Keyboard::Right) {
                if (!enemies.empty()) {
                    int next = getNextLivingEnemy(currentEnemyIndex);
                    if (next >= 0) currentEnemyIndex = next;
                }
            }
            else if (event.key.code == sf::Keyboard::Left) {
                if (!enemies.empty()) {
                    int prev = getPrevLivingEnemy(currentEnemyIndex);
                    if (prev >= 0) currentEnemyIndex = prev;
                }
              
            }
            else if (event.key.code == sf::Keyboard::W){
                if (levelUpTime){
                    levelUpAttributeIndex--;
                    } 
                }
            
            else if (event.key.code == sf::Keyboard::S){
                if (levelUpTime){
                      levelUpAttributeIndex++;
            }
        }
            else if (event.key.code == sf::Keyboard::D){
                if (levelUpTime){
                    switch ((levelUpAttributeIndex % 5 + 5) % 5){
                        case (0):
                            if (skillPoints != 0 && strengthVal != 99){
                                strengthVal++;
                                strength.setString("ST             " + std::to_string(strengthVal));
                                strengthValPercent = (float)strengthVal / 99;
                                stBar.setSize({500.0f * strengthValPercent, 10.0f});
                                skillPoints--;
                            }
                            break;
                        case (1):
                            if (skillPoints != 0 && vitalityVal != 99){
                            vitalityVal++;
                            vitality.setString("VI             " + std::to_string(vitalityVal));
                            vitalityValPercent = (float)vitalityVal / 99;
                            viBar.setSize({500.0f * vitalityValPercent, 10.0f});
                            recalculatedMaxHp = (levelUpIterator->first->getLVL() + vitalityVal) * 6;
                            maxHp.setString("Max HP                 " + std::to_string(maxHpVal) + "  ==>  " + std::to_string(recalculatedMaxHp));
                            skillPoints--;
                        }
                            break;
                        case (2):
                            if (skillPoints != 0 && magicVal != 99){
                            magicVal++;
                            magic.setString("MA             " + std::to_string(magicVal));
                            magicValPercent = (float)magicVal / 99;
                            maBar.setSize({500.0f * magicValPercent, 10.0f});
                            recalculatedMaxMp = (levelUpIterator->first->getLVL() + magicVal) * 3;
                            maxMp.setString("Max MP                 " + std::to_string(maxMpVal) + "  ==>  " + std::to_string(recalculatedMaxMp));
                            skillPoints--;
                            break;
                        }
                        case (3):
                             if (skillPoints != 0 && agilityVal!= 99){
                            agilityVal++;
                            agility.setString("AG             " + std::to_string(agilityVal));
                            agilityValPercent= (float)agilityVal / 99;
                            agBar.setSize({500.0f * agilityValPercent, 10.0f});
                            skillPoints--;
                            break;
                        }
                        case (4):
                            if (skillPoints != 0 && luckVal!= 99){
                            luckVal++;
                            luck.setString("LU             " + std::to_string(luckVal));
                            luckValPercent= (float) luckVal / 99;
                            luBar.setSize({500.0f * luckValPercent, 10.0f});
                            skillPoints--;
                            break;
                        }
                    }
                    distributionText.setString("Distribute points.\n" + std::to_string(skillPoints) + " points remaining."); 
            }
        }
            else if (event.key.code == sf::Keyboard::A){
                if (levelUpTime){
                 switch ((levelUpAttributeIndex % 5 + 5) % 5){
                        case (0):
                            if (strengthVal != character->getSTR()){
                                strengthVal--;
                                strength.setString("ST             " + std::to_string(strengthVal));
                                strengthValPercent = (float)strengthVal / 99;
                                stBar.setSize({500.0f * strengthValPercent, 10.0f});
                                skillPoints++;
                            }
                            break;
                        case (1):
                            if (vitalityVal != character->getVIT()){
                            vitalityVal--;
                            vitality.setString("VI             " + std::to_string(vitalityVal));
                            vitalityValPercent = (float)vitalityVal / 99;
                            viBar.setSize({500.0f * vitalityValPercent, 10.0f});
                            recalculatedMaxHp = (levelUpIterator->first->getLVL() + vitalityVal) * 6;
                            maxHp.setString("Max HP                 " + std::to_string(maxHpVal) + "  ==>  " + std::to_string(recalculatedMaxHp));
                            skillPoints++;
                        }
                            break;
                        case (2):
                            if (magicVal != character->getMAG()){
                            magicVal--;
                            magic.setString("MA             " + std::to_string(magicVal));
                            magicValPercent = (float)magicVal / 99;
                            maBar.setSize({500.0f * magicValPercent, 10.0f});
                            recalculatedMaxMp = (levelUpIterator->first->getLVL() + magicVal) * 3;
                            maxMp.setString("Max MP                 " + std::to_string(maxMpVal) + "  ==>  " + std::to_string(recalculatedMaxMp));
                            skillPoints++;
                            break;
                        }
                        case (3):
                             if (agilityVal != character->getAGI()){
                            agilityVal--;
                            agility.setString("AG             " + std::to_string(agilityVal));
                            agilityValPercent= (float)agilityVal / 99;
                            agBar.setSize({500.0f * agilityValPercent, 10.0f});
                            skillPoints++;
                            break;
                        }
                        case (4):
                            if (luckVal != character->getLU()){
                            luckVal--;
                            luck.setString("LU             " + std::to_string(luckVal));
                            luckValPercent= (float)luckVal / 99;
                            luBar.setSize({500.0f * luckValPercent, 10.0f});
                            skillPoints++;
                            break;
                        }
                    }
                    distributionText.setString("Distribute points.\n" + std::to_string(skillPoints) + " points remaining.");
                }
        }
    }

        // --- Mouse click events for battle input
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (currentMenuState == BattleMenuState::Main) {
                if (attackButton.wasClicked(this->game->window)) {
                    if (turnQueue.empty()) {
                        battleText.setString("No one can act right now.");
                        return;
                    }

                    Player* attacker = turnQueue.front();     // the actor whose turn it is
                    
                    // ensure the actor is one of the party members (players)
                    bool isPartyActor = (std::find(party.begin(), party.end(), attacker) != party.end());
                    if (!isPartyActor) {
                        battleText.setString("It's not your turn!");
                        return;
                    }

                    // find valid target (selected enemy)
                    if (enemies.empty()) {
                        battleText.setString("No enemies to target.");
                        return;
                    }

                    int targetIdx = currentEnemyIndex;
                    if (targetIdx < 0 || targetIdx >= static_cast<int>(enemies.size())) {
                        targetIdx = getFirstLivingEnemy();
                        if (targetIdx < 0) {
                            battleText.setString("No enemies left.");
                            return;
                        }
                    }

                    NPC* target = &enemies[targetIdx];
                    if (target->isDead()) {
                        battleText.setString(battleText.getString() + "\n" + target->getName() + " was defeated!");
                        cleanupDeadEnemies();
                        int newIdx = getFirstLivingEnemy();
                        if (newIdx < 0) {
                            battleText.setString("All enemies defeated.");
                            return;
                        }
                        currentEnemyIndex = newIdx;
                        target = &enemies[currentEnemyIndex];
                    }

                    // Use the "Attack" skill if present; fallback baseAtk/crit defaults
                    const Skill* atkSkill = findSkillByName(this->game, "Attack");

                    int baseAtk = 15;          // default fallback
                    float critChance = 0.05f;  // default fallback
                    
                    if (atkSkill) {
                        if (atkSkill->getBaseAtk() > 0)
                            baseAtk = atkSkill->getBaseAtk();
                    
                        if (atkSkill->getCritRate() > 0)
                            critChance = atkSkill->getCritRate();
                    }

                    // compute damage using attacking player's physATK (player inherits Player)
                    int damage = attacker->physATK(1.0f, baseAtk, false);
                    damage = static_cast<int>(std::round(damage * target->getIncomingDamageMultiplier()));

                    // crit roll
                    std::uniform_real_distribution<float> cr(0.f, 1.f);
                    bool isCrit = (cr(globalRng()) < critChance);
                    if (isCrit) damage = static_cast<int>(damage * 1.5f);

                    target->takeDamage(damage);

                    // spawn damage popup above enemy
                    DamagePopup dp;
                    dp.text.setFont(font);
                    dp.text.setCharacterSize(32);
                    dp.text.setString((isCrit ? std::string("CRIT ") : std::string("")) + std::to_string(damage));
                    dp.text.setFillColor(isCrit ? sf::Color::Red : sf::Color::White);

                    // place popup near enemy sprite center
                    sf::FloatRect eb = enemySprites[targetIdx].getGlobalBounds();
                    dp.text.setPosition(eb.left + eb.width / 2.f - dp.text.getGlobalBounds().width / 2.f,
                                        eb.top - 10.f);
                    dp.velocity = sf::Vector2f(0.f, -30.f);
                    dp.life = 1.0f;
                    damagePopups.push_back(dp);

                    // update battle text
                    battleText.setString(
                        attacker->getName() + " attacked " +
                        target->getName() + " dealing " +
                        std::to_string(damage) + " damage!"
                    );

                    // if enemy died, remove them from turnQueue
                    if (target->isDead()) {
                        battleText.setString(battleText.getString() + "\n" + target->getName() + " was defeated!");
                        for (auto it = turnQueue.begin(); it != turnQueue.end(); ) {
                            if (*it == target) it = turnQueue.erase(it);
                            else ++it;
                        }

                        // check if all enemies dead
                        bool anyAlive = false;
                        for (auto& e : enemies) {
                            if (!e.isDead()) { anyAlive = true; break; }
                        }
                        if (!anyAlive) {
                            battleOver = true;
                            return;
                        } else {
                            int first = getFirstLivingEnemy();
                            if (first >= 0) currentEnemyIndex = first;
                        }
                    }

                    // end turn (rotate queue)
                    if (!turnQueue.empty()) {
                        Player* front = turnQueue.front();
                        turnQueue.pop_front();
                        turnQueue.push_back(front);
                        if (front) front->decrementBuffTurns();
                    }

                    currentMenuState = BattleMenuState::Main; // end click event
                    return;
                }                           
                else if (skillButton.wasClicked(this->game->window)) {
                    if (!turnQueue.empty()) {
                        Player* active = turnQueue.front();
                        buildSkillButtonsFor(active);
                    }
                    currentMenuState = BattleMenuState::Skill;
                }
                else if (itemButton.wasClicked(this->game->window)) {
                    currentMenuState = BattleMenuState::Item;
                }
            }

            else if (currentMenuState == BattleMenuState::Skill) {

                for (auto& b : skillButtons) {
                    if (!b.wasClicked(this->game->window)) continue;

                    // It's assumed the front of turnQueue is the actor using the skill.
                    if (turnQueue.empty()) {
                        battleText.setString("No one can act right now.");
                        break;
                    }
                    Player* attacker = turnQueue.front();

                    // safety: ensure actor is a player
                    bool isPartyActor = (std::find(party.begin(), party.end(), attacker) != party.end());
                    if (!isPartyActor) {
                        battleText.setString("It's not your turn!");
                        break;
                    }

                    // find the Skill* from attacker's known skills
                    const std::string skillName = b.getText();
                    const Skill* s = attacker->getSkillPtr(skillName, this->game->skillMasterList);
                    if (!s) {
                        battleText.setString("Skill not found.");
                        break;
                    }

                    // Check MP (and HP if you use HP costs)
                    int mpCost = s->getMpCost();
                    if (mpCost > 0 && attacker->getMP() < mpCost) {
                        battleText.setString("Not enough MP to use " + s->getName() + ".");
                        break;
                    }
                    // If skill has HP cost (getHpCost() > 0) - optional behavior
                    float hpCostPct = s->getHpCost();
                    if (hpCostPct > 0.0f) {
                        int hpCost = static_cast<int>(std::round(attacker->getmaxHP() * hpCostPct));
                        if (attacker->getHP() <= hpCost) {
                            battleText.setString("Not enough HP to use " + s->getName() + ".");
                            break;
                        }
                        // apply HP cost (we use takeDamage to subtract HP)
                        attacker->takeDamage(hpCost);
                    }

                    // Spend MP
                    if (mpCost > 0) attacker->spendMP(mpCost);

                    // Determine whether this is healing or damaging skill by type string
                    const std::string type = s->getType(); // e.g. "Physical", "Fire", "Healing", "Almighty", "Damage Amp", etc.
                    bool isHealing = (type == "Healing");
                    bool isPhysical = (type.find("Physical") != std::string::npos);
                    //bool isAlmighty = (type.find("Almighty") != std::string::npos);
                    bool isDamageAmpSkill = (type == "Damage Amp"); // buff-like
                    bool isHitEvadeBoost = (type == "Hit Evade Boost" || type == "Hit Evade Reduction");
                    bool isDamageResistSkill = (type == "Damage Resist");

                    // Basic scalar: use damage amp if present, otherwise 1.0
                    float scalar = (s->getDamageAmp() > 0.0f) ? s->getDamageAmp() : 1.0f;

                    // Determine targets: single vs all
                    bool singleTarget = s->getIsSingleTarget();

                    // Convenience lambdas for popups + text update (uses your font & sprites)
                    auto spawnPopupAtEnemyIndex = [&](int enemyIdx, int dmg, bool crit, float elementMul) {
                        DamagePopup dp;
                        dp.text.setFont(font);
                        dp.text.setCharacterSize(32);
                    
                        // Build label
                        std::string label = "";
                        if (crit) label += "CRIT ";
                        if (elementMul > 1.0f)      label += "WEAK ";
                        else if (elementMul < 1.0f) label += "RESIST ";
                    
                        dp.text.setString(label + std::to_string(dmg));
                    
                        // Color based on affinity
                        sf::Color popupColor = sf::Color::White;
                        if (elementMul > 1.0f)       popupColor = sf::Color(255, 255, 0);        // Yellow
                        else if (elementMul < 1.0f)  popupColor = sf::Color(100, 149, 255);      // Blue
                        else if (crit)               popupColor = sf::Color::Red;                // Red
                    
                        dp.text.setFillColor(popupColor);
                    
                        // Position at enemy sprite center
                        sf::FloatRect eb = (enemyIdx >= 0 && enemyIdx < (int)enemySprites.size()) ? enemySprites[enemyIdx].getGlobalBounds() : sf::FloatRect(800.f, 300.f, 0.f, 0.f);
                    
                        dp.text.setPosition(
                            eb.left + eb.width / 2.f - dp.text.getGlobalBounds().width / 2.f,
                            eb.top - 10.f
                        );
                    
                        dp.velocity = sf::Vector2f(0.f, -30.f);
                        dp.life = 1.0f;
                    
                        damagePopups.push_back(dp);
                    };                    

                    auto spawnPopupAtPlayerIndex = [&](int pIdx, int dmg, bool crit) {
                        DamagePopup dp;
                        dp.text.setFont(font);
                        dp.text.setCharacterSize(28);
                        dp.text.setString((crit ? std::string("CRIT ") : std::string("")) + std::to_string(dmg));
                        dp.text.setFillColor(sf::Color::Red);
                        sf::Vector2f pos(200.f, 700.f);
                        if (pIdx >= 0 && pIdx < static_cast<int>(playerIcons.size())) pos = playerIcons[pIdx].getPosition() - sf::Vector2f(0.f, 40.f);
                        dp.text.setPosition(pos);
                        dp.velocity = sf::Vector2f(0.f, -30.f);
                        dp.life = 1.0f;
                        damagePopups.push_back(dp);
                    };

                    // handle healing skills
                    if (isHealing) {
                        // healing percent stored in skill
                        float healPct = s->getHealthRestorePercent(); // e.g. 1.25 => 125%
                        if (singleTarget) {
                            // heal the active party member (front of queue) or first alive ally
                            int idx = 0;
                            Player* tgt = nullptr;
                            for (size_t i = 0; i < party.size(); ++i) {
                                if (party[i] && party[i]->getHP() > 0) { idx = static_cast<int>(i); tgt = party[i]; break; }
                            }
                            if (tgt) {
                                int healAmount = static_cast<int>(std::round(tgt->getmaxHP() * healPct));
                                tgt->heal(healAmount);
                                battleText.setString(attacker->getName() + " used " + s->getName() + " and healed " + tgt->getName() + " for " + std::to_string(healAmount) + " HP!");
                                // spawn a small popup near player
                                spawnPopupAtPlayerIndex(idx, healAmount, false);
                            }
                        } else {
                            // heal all allies
                            int totalHealed = 0;
                            for (size_t i = 0; i < party.size(); ++i) {
                                Player* tgt = party[i];
                                if (!tgt) continue;
                                int healAmount = static_cast<int>(std::round(tgt->getmaxHP() * healPct));
                                tgt->heal(healAmount);
                                totalHealed += healAmount;
                                spawnPopupAtPlayerIndex(static_cast<int>(i), healAmount, false);
                            }
                            battleText.setString(attacker->getName() + " used " + s->getName() + " and healed the party!");
                        }

                        // end turn: rotate
                        if (!turnQueue.empty()) {
                            Player* front = turnQueue.front(); turnQueue.pop_front(); turnQueue.push_back(front);
                            //updateBuffTimers();
                        }
                        break; // processed skill click
                    }

                    // handle buff/utility skills (Damage Amp, Damage Resist, Hit/Evade)
                    if (isDamageAmpSkill) {
                        // +25% outgoing damage for 3 turns on attacker
                        attacker->addBuff("Damage Amp", 1.25f, 3, /*affectsOutgoing=*/true, /*affectsIncoming=*/false);
                        battleText.setString(attacker->getName() + " used " + s->getName() + ". Damage increased!");
                        // popup
                        DamagePopup dp;
                        dp.text.setFont(font);
                        dp.text.setCharacterSize(24);
                        dp.text.setString("DMG UP!");
                        dp.text.setFillColor(sf::Color(255,200,50)); // gold-ish
                        dp.text.setPosition(playerIcons[0].getPosition() - sf::Vector2f(0.f, 40.f)); // adjust actor pos
                        dp.velocity = sf::Vector2f(0.f, -25.f);
                        dp.life = 1.2f;
                        damagePopups.push_back(dp);
                    }
                    else if (isDamageResistSkill) {
                        // 20% incoming damage reduction (multiplier 0.8) for 3 turns
                        attacker->addBuff("Damage Resist", 0.80f, 3, /*affectsOutgoing=*/false, /*affectsIncoming=*/true);
                        battleText.setString(attacker->getName() + " used " + s->getName() + ". Damage taken reduced!");
                        // popup (similar)
                    }
                    else if (isHitEvadeBoost) {
                        // add both hit & evade buffs (use standardized names)
                        attacker->addBuff("Hit Boost", 1.15f, 3, true, false);
                        attacker->addBuff("Evade Boost", 1.15f, 3, false, false);
                        battleText.setString(attacker->getName() + " used " + s->getName() + ". Accuracy & Evasion up!");
                    }
                    

                    // else: damage skill (physical, magic, almighty, etc.)
                    // Determine targets: single or all
                    std::vector<NPC*> enemyTargets;
                    if (singleTarget) {
                        int tidx = currentEnemyIndex;
                        if (tidx < 0 || tidx >= static_cast<int>(enemies.size()) || enemies[tidx].isDead()) {
                            tidx = getFirstLivingEnemy();
                        }
                        if (tidx >= 0) enemyTargets.push_back(&enemies[tidx]);
                    } else {
                        for (auto& en : enemies) if (!en.isDead()) enemyTargets.push_back(&en);
                    }

                    if (enemyTargets.empty()) {
                        battleText.setString("No valid targets for " + s->getName() + ".");
                        break;
                    }

                    // compute damage for each target (respecting physical vs magic vs almighty)
                    int totalDamage = 0;
                    for (size_t ei = 0; ei < enemyTargets.size(); ++ei) {
                        NPC* target = enemyTargets[ei];

                        int damage = 0;
                        bool crit = false;
                        float critChance = s->getCritRate();
                        if (critChance <= 0.f) critChance = 0.05f; // fallback
                        float elementMul = getElementMultiplier(target, s);

                        // roll crit
                        std::uniform_real_distribution<float> cr(0.f, 1.f);
                        crit = (cr(globalRng()) < critChance);

                        if (isPhysical) {
                            // Use physATK. Use baseAtk and scalar if provided.
                            int baseAtk = s->getBaseAtk();
                            float usedScalar = scalar;
                            damage = attacker->physATK(usedScalar, baseAtk, crit);
                        } else {
                            // magic / almighty / element
                            int baseAtk = s->getBaseAtk();
                            int limit   = s->getLimit();
                            int corr    = s->getCorrection();

                            // Determine weakness from target affinities via helper: getElementMultiplier
                            // getElementMultiplier should return e.g. 1.5 for weak, 0.5 for resist, 1.0 for neutral.
                            float elementMul = 1.0f;
                            // If you named your helper differently, change this call accordingly
                            elementMul = getElementMultiplier(target, s);

                            // magATK uses its own scalar behaviour; pass isWeak (true if multiplier > 1.0)
                            bool isWeak = (elementMul > 1.0f);

                            damage = attacker->magATK(1.0f /* scalar for mag formula */, baseAtk, limit, corr, isWeak);
                            damage = static_cast<int>(std::round(damage * elementMul * target->getIncomingDamageMultiplier()));

                            // apply element multiplier
                            damage = static_cast<int>(std::round(damage * elementMul));
                        }

                        // apply damage to target
                        target->takeDamage(damage);
                        totalDamage += damage;

                        // spawn popup: find enemy index in enemies vector
                        int enemyIndexInVector = getEnemyIndex(target);
                        spawnPopupAtEnemyIndex(enemyIndexInVector, damage, crit, elementMul);
                    } // end for each enemy target

                    // update battle text
                    if (enemyTargets.size() == 1) {
                        battleText.setString(attacker->getName() + " used " + s->getName() + " on " + enemyTargets.front()->getName() + " for " + std::to_string(totalDamage) + " damage!");
                    } else {
                        battleText.setString(attacker->getName() + " used " + s->getName() + " and dealt " + std::to_string(totalDamage) + " total damage to the enemies!");
                    }

                    // Remove dead enemies from turnQueue and check battle over
                    cleanupDeadEnemies();
                    bool anyAlive = false;
                    for (auto& e : enemies) if (!e.isDead()) { anyAlive = true; break; }
                    if (!anyAlive) {
                        battleOver = true;
                        return;
                    } else {
                        int first = getFirstLivingEnemy();
                        if (first >= 0) currentEnemyIndex = first;
                    }

                    // end turn: rotate queue
                    if (!turnQueue.empty()) {
                        Player* front = turnQueue.front();
                        turnQueue.pop_front();
                        turnQueue.push_back(front);
                        if (front) front->decrementBuffTurns();
                    }

                    currentMenuState = BattleMenuState::Main;
                    return; // handled this skill click
                } // end for skillButtons

                // Back button (return to main menu)
                if (backButton.wasClicked(this->game->window)) {
                    currentMenuState = BattleMenuState::Main;
                }
            } // end Skill branch
        }
    }
}

  
// load random enemies
std::vector<NPC> GameStateBattle::loadRandomEnemies(int count) {
    std::ifstream file("assets/enemies/enemies.json");
    if (!file.is_open()) {
        std::cerr << "Could not open enemies.json\n";
        return {};
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (std::exception& ex) {
        std::cerr << "Failed to parse enemies.json: " << ex.what() << "\n";
        return {};
    }

    if (!j.is_array() || j.empty()) {
        std::cerr << "enemies.json is empty or invalid\n";
        return {};
    }

    std::vector<NPC> selectedEnemies;
    selectedEnemies.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(j.size()) - 1);

    for (int i = 0; i < count; ++i) {
        auto& e = j[dis(gen)];

        // --- Load affinities from JSON ---
        std::map<std::string, float> aff;

        if (e.contains("affinities")) {
            for (auto& [key, val] : e["affinities"].items()) {
                aff[key] = val.get<float>();
            }
        }

        // Ensure all expected elements exist (fallback = neutral)
        static const std::vector<std::string> elems = {
            "Physical", "Fire", "Ice", "Electric", "Force", "Almighty"
        };

        for (const std::string& elem : elems) {
            if (aff.find(elem) == aff.end())
                aff[elem] = 1.0f;
        }

        // Load skill list from JSON and attach to NPC
        std::vector<std::string> sks;
        if (e.contains("skills") && e["skills"].is_array()) {
            for (auto &it : e["skills"]) {
                try {
                    sks.push_back(it.get<std::string>());
                    } catch (...) { /* ignore malformed entries */ }
                }
            }

        // --- Create the NPC
        selectedEnemies.emplace_back(
            e.value("name", "Unknown"),
            e.value("sprite", "default.png"),
            e.value("level", 1),
            e.value("STR", 1),
            e.value("VIT", 1),
            e.value("MAG", 1),
            e.value("AGI", 1),
            e.value("LU", 1),
            e.value("baseXP", 0),
            aff,
            e.value("isBoss", false),
            sks
       );
    }    
    return selectedEnemies;
}


// Dynamically update the turn panel
void GameStateBattle::updateTurnPanel() {
    // --- Build combined sorted list by AGI ---
    struct TurnEntry {
        bool isPlayer;
        Player* playerPtr;
        NPC* enemyPtr;
        int agi;
    };

    std::vector<TurnEntry> turnList;
    turnList.reserve(party.size() + enemies.size());

    // Add players
    for (auto* p : party) {
        turnList.push_back(TurnEntry{ true, p, nullptr, p->getAGI() });
    }

    // Add enemies
    for (auto& e : enemies) {
        turnList.push_back(TurnEntry{ false, nullptr, &e, e.getAGI() });
    }

    // Sort by AGI descending
    std::sort(turnList.begin(), turnList.end(),
        [](const TurnEntry& a, const TurnEntry& b) {
            return a.agi > b.agi;
        });

    turnPortraitBoxes.clear();
    turnPortraitSprites.clear();
    enemyNameBackgrounds.clear();
    turnEnemyNames.clear();

    // We will "blank" the ones that are enemies
    turnPortraitBoxes.resize(turnList.size());
    turnPortraitSprites.resize(turnList.size());

    // --- Layout ---
    float panelX = turnPanelBackground.getPosition().x;
    float panelY = turnPanelBackground.getPosition().y;
    float padding = 16.f;
    float portraitSize = 25.f;       // size
    float entrySpacingY = 4.f;       // spacing between


    float currentY = panelY + padding;

    for (size_t i = 0; i < turnList.size(); ++i) {
        const auto& entry = turnList[i];
        float x = panelX + 70.f;
        float y = currentY; // Use the running Y position


        float entryHeight = portraitSize; 

        bool isCurrentTurn = (!turnQueue.empty() && turnQueue.front() == (entry.isPlayer ? (Player*)entry.playerPtr : (Player*)entry.enemyPtr));

        if (entry.isPlayer) {
            // Find the player's original index to get the correct texture
            auto it = std::find(party.begin(), party.end(), entry.playerPtr);
            if (it == party.end()) continue; // Should not happen
            size_t static_party_index = std::distance(party.begin(), it);
            
            std::string texName = (static_party_index == 0) ? "player_icon" : "partymember" + std::to_string(static_party_index + 1) + "_icon";
            
            sf::RectangleShape& box = turnPortraitBoxes[i];
            sf::Sprite& spr = turnPortraitSprites[i];

            box.setPosition(x, y);
            box.setSize({ portraitSize, portraitSize }); // Use the fixed height
            box.setFillColor(sf::Color(40, 40, 40, 160));
            box.setOutlineColor(sf::Color(150, 0, 0));
            box.setOutlineThickness(1.5f);

            spr.setTexture(this->game->texmgr.getRef(texName), true);

            float textureHeight = spr.getLocalBounds().height;
            float baseScale = (textureHeight > 0.f) ? (portraitSize / textureHeight) : 1.0f;
            float finalScale = isCurrentTurn ? baseScale * 1.1f : baseScale;

            spr.setScale(finalScale, finalScale);
            if (spr.getTexture() != nullptr) {
                sf::FloatRect lb = spr.getLocalBounds();
                spr.setPosition(
                    x + (portraitSize - lb.width * finalScale) / 2.f,
                    y + (portraitSize - lb.height * finalScale) / 2.f
                );
            }
        } else {
            // Enemy Entry
            sf::Text nameText;
            nameText.setFont(font);
            int charSize = isCurrentTurn ? 22 : 20; // Use your desired text size
            nameText.setCharacterSize(charSize);
            nameText.setFillColor(sf::Color::White);
            nameText.setString(entry.enemyPtr->getDisplayName());

            sf::FloatRect textBounds = nameText.getLocalBounds();
            float textPadding = 12.f; 


            float textCenterY = y + (entryHeight / 2.f) - (textBounds.height / 2.f) - 4.f; // Adjust -4.f as needed for font alignment
            nameText.setPosition(x + 6.f, textCenterY);

            sf::RectangleShape bg;

            bg.setSize({ textBounds.width + textPadding, entryHeight }); 
            bg.setPosition(x, y);
            bg.setFillColor(sf::Color(50, 0, 0, 180));
            bg.setOutlineColor(sf::Color(120, 0, 0));
            bg.setOutlineThickness(1.0f);

            if (isCurrentTurn) {
                nameText.setFillColor(sf::Color::Green);
                bg.setFillColor(sf::Color(0, 100, 0, 200));
            }

            enemyNameBackgrounds.push_back(bg);
            turnEnemyNames.push_back(nameText);
            if (i < turnPortraitBoxes.size()) turnPortraitBoxes[i].setSize({0.f, 0.f});
            if (i < turnPortraitSprites.size()) turnPortraitSprites[i].setTextureRect(sf::IntRect(0, 0, 0, 0));
        }
        
        currentY += entryHeight + entrySpacingY;
    }
}

// Dynamically update the skills 

void GameStateBattle::buildSkillButtonsFor(Player* character) {
    skillButtons.clear();

    float baseX = 150.f;
    float baseY = 780.f;
    float offsetY = 55.f;
    float columnSpacing = 260.f;

    size_t visibleIndex = 0;
    int charLevel = character->getLVL();

    size_t i = 0;
    for (const auto& skillName : character->getSkillNames()) {
        if (i++ == 0) continue; // skip first entry
    
        const Skill* s = character->getSkillPtr(skillName, game->skillMasterList);
        if (!s || charLevel < s->getUnlockLevel()) continue;
    
        size_t col = visibleIndex % 2;
        size_t row = visibleIndex / 2;
        float x = baseX + col * columnSpacing;
        float y = baseY + row * offsetY;
    
        skillButtons.emplace_back(s->getName(), sf::Vector2f(x, y), 28, game, sf::Color::White);
        visibleIndex++;
    }    
}

int GameStateBattle::getFirstLivingEnemy() {
    if (enemies.empty()) return -1;
    for (int i = 0; i < static_cast<int>(enemies.size()); ++i)
        if (!enemies[i].isDead()) return i;
    return -1; // fallback: none alive
}

int GameStateBattle::getNextLivingEnemy(int i) {
    if (enemies.empty()) return -1;
    int start = i;
    if (start < 0 || start >= static_cast<int>(enemies.size())) start = 0;
    int idx = start;
    do {
        idx = (idx + 1) % static_cast<int>(enemies.size());
    } while (enemies[idx].isDead() && idx != start);
    return idx;
}

int GameStateBattle::getPrevLivingEnemy(int i) {
    if (enemies.empty()) return -1;
    int start = i;
    if (start < 0 || start >= static_cast<int>(enemies.size())) start = 0;
    int idx = start;
    do {
        idx = (idx - 1 + static_cast<int>(enemies.size())) % static_cast<int>(enemies.size());
    } while (enemies[idx].isDead() && idx != start);
    return idx;
}

void GameStateBattle::cleanupDeadEnemies() {
    // Remove enemy sprites, names, data, turnQueue refs.
    for (int i = (int)enemies.size() - 1; i >= 0; --i) {
        if (enemies[i].isDead()) {

            // Remove sprite in same index
            if (i < (int)enemySprites.size())
                enemySprites.erase(enemySprites.begin() + i);

            // Remove from turnQueue
            for (auto it = turnQueue.begin(); it != turnQueue.end();) {
                if (*it == &enemies[i]) it = turnQueue.erase(it);
                else ++it;
            }

            // Actually remove enemy object
            enemies.erase(enemies.begin() + i);
        }
    }

    // Fix targeting index
    currentEnemyIndex = getFirstLivingEnemy();
}

float GameStateBattle::getElementMultiplier(const Player* target, const Skill* s) const {
    if (s->getType() == "Physical-Almighty" ||
        s->getType() == "Magic-Almighty" ||
        s->getType() == "Almighty")
        return 1.0f; // ignores affinities

    auto affinities = target->getAffinityMap(); // I'll show you how to add this getter
    auto it = affinities.find(s->getType());
    if (it == affinities.end()) return 1.0f;

    float a = it->second;
    if (a == 0.0f) return 0.0f;   // NULL
    return a;                     // 0.5 resist, 1.0 neutral, 1.5 weak
}

int GameStateBattle::getEnemyIndex(NPC* e) const {
    if (!e) return -1;
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (&enemies[i] == e)
            return static_cast<int>(i);
    }
    return -1;
}

