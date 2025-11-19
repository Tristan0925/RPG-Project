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

GameStateBattle::GameStateBattle(Game* game, bool isBossBattle)     
: topBarTextBackground(sf::Quads,4), thingsEarnedBackground(sf::Quads,4), attackButton("Attack", {150.f, 800.f}, 30, game, sf::Color::White),
  skillButton("Skill",  {150.f, 840.f}, 30, game, sf::Color::White),
  itemButton("Item",   {150.f, 880.f}, 30, game, sf::Color::White),
  guardButton("Guard", {350.f, 800.f}, 30, game, sf::Color::White),
  escapeButton("Escape",{350.f, 840.f}, 30, game, sf::Color::White),
  backButton("Back", {350.f, 920.f}, 30, game, sf::Color::White)
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
        turnPortraitBaseScales[i] = scale;

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

        // Now set the portrait sprite for the turn panel (use same texture)
        if (i < turnPortraitSprites.size()) {
            turnPortraitSprites[i].setTexture(this->game->texmgr.getRef(texName), true);
            turnPortraitSprites[i].setScale(scale, scale);
            // set an initial position; actual centering will happen in updateTurnPanel()
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

    // Music
    if (!currentMusic.openFromFile("./assets/music/normalbattle.mp3")) {
        std::cout << "Could not load music file" << std::endl;
    } else {
        currentMusic.setLoop(true);
        currentMusic.play();
    }
    
    // Back button (reconfigure if you want different color)
    backButton = Button("Back", {150.f, 900.f}, 30, this->game, sf::Color(90, 90, 90));

    // --- Populate Skills dynamically from player's learned skills ---
    skillButtons.clear();
    float skillY = 300.f;
    for (const auto& skillName : this->game->playerSkills) {
        skillButtons.emplace_back(skillName, sf::Vector2f(150.f, skillY), 30, this->game, sf::Color(100, 100, 220));
        skillY += 70.f;
    }

    // now set positions for skill buttons if needed
    {
        float baseX = 150.f;
        float baseY = 800.f;
        float offsetY = 40.f;
        for (size_t k = 0; k < skillButtons.size(); ++k) {
            skillButtons[k].changePosition(baseX, baseY + k * offsetY);
        }
    }

    // Populate Items dynamically from inventory 
    itemButtons.clear();
    float itemY = 800.f;
    for (const auto& itemName : this->game->player.getInventory()) { 
        itemButtons.emplace_back(itemName.showName(), sf::Vector2f(150.f, itemY), 30, this->game, sf::Color(120, 180, 120));
        itemY += 70.f;
    }

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
    
    for (size_t x = 0; x < 4; x++){
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
    strength.setPosition(100.0f, 365.0f);

    stBar.setSize({500.0f * strengthValPercent, 10.0f});
    stBar.setFillColor(sf::Color(255,165,0));
    stBar.setPosition(200.0f, 380.0f);

    vitality.setFont(font);
    vitality.setString("VI              " + std::to_string(vitalityVal));
    vitality.setPosition(100.0f, 408.0f);

    viBar.setSize({500.0f * vitalityValPercent, 10.0f});
    viBar.setFillColor(sf::Color(255,165,0));
    viBar.setPosition(200.0f, 423.0f);

    agility.setFont(font);
    agility.setString("AG             " + std::to_string(agilityVal));
    agility.setPosition(100.0f, 451.0f);

    agBar.setSize({500.0f * agilityValPercent, 10.0f});
    agBar.setFillColor(sf::Color(255,165,0));
    agBar.setPosition(200.0f, 466.0f);

    luck.setFont(font);
    luck.setString("LU             " + std::to_string(luckVal));
    luck.setPosition(100.0f, 494.0f);

    luBar.setSize({500.0f * luckValPercent, 10.0f});
    luBar.setFillColor(sf::Color(255,165,0));
    luBar.setPosition(200.0f, 509.0f);

    maxHp.setFont(font);
    maxHp.setString("Max HP                 " + std::to_string(maxHpVal) + "  ==>  " + std::to_string(recalculatedMaxHp));
    maxHp.setPosition(730.0f, 365.0f);

    maxMp.setFont(font);
    maxMp.setString("Max HP                 " + std::to_string(maxMpVal) + "  ==>  " + std::to_string(recalculatedMaxMp));
    maxMp.setPosition(730.0f,408.0f);

    pointsToDistributeTextbox.setSize({290.0f,80.0f});
    pointsToDistributeTextbox.setFillColor(sf::Color::Transparent);
    pointsToDistributeTextbox.setOutlineColor(sf::Color(255,0,0,50));
    pointsToDistributeTextbox.setOutlineThickness(1.0f);
    pointsToDistributeTextbox.setPosition(710.0f, 455.0f);

    distributionText.setFont(font);
    distributionText.setPosition(710.0f, 455.0f);
    distributionText.setString("Distribute points.\n" + std::to_string(skillPoints) + " points remaining.");

    for (size_t x = 1; x < 9; x++){
        skillNamesForResults[x].setFont(font);
        skillNamesForResults[x].setPosition(200.0f, 509.0f + (50.0f * x-1));
    }

    levelUpBooleanMap[&this->game->player] = false;
    levelUpBooleanMap[&this->game->pmember2] = false;
    levelUpBooleanMap[&this->game->pmember3] = false;
    levelUpBooleanMap[&this->game->pmember4] = false;
    levelUpIterator = levelUpBooleanMap.begin();
}

void GameStateBattle::displayResultsScreen(){
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
    this->game->window.draw(agility);
    this->game->window.draw(luck);
    this->game->window.draw(maxHp);
    this->game->window.draw(maxMp);
    this->game->window.draw(stBar);
    this->game->window.draw(viBar);
    this->game->window.draw(agBar);
    this->game->window.draw(luBar);
      if (printSkillNames){
        for (auto& skillName : skillNamesForResults){
            this->game->window.draw(skillName);
        }
    }
}



void GameStateBattle::draw(const float dt) {
    if (battleOver && !levelUpTime){
        if (!playResultsMusic){
            currentMusic.stop();
            if (!currentMusic.openFromFile("./assets/music/battleresults.mp3")) std::cout << "Could not load music file" << std::endl;
            else {
                currentMusic.setLoop(true);
                currentMusic.play();
                playResultsMusic = true;
                }
            }
         displayResultsScreen();
     
    }
    else if (battleOver && levelUpTime){
        displayLevelUpScreen();
    } 

    else{
    this->game->window.clear();
    this->game->window.draw(background);
    this->game->window.draw(enemyBackground);
    this->game->window.draw(textBox);
    this->game->window.draw(battleText);

    // Draw party
    for (size_t i = 0; i < party.size(); ++i) {
        // safety checks
        if (i >= playerBackgrounds.size() || i >= playerIcons.size() || i >= hpBars.size() || i >= mpBars.size())
            continue;
        this->game->window.draw(playerBackgrounds[i]);
        this->game->window.draw(playerIcons[i]);
        this->game->window.draw(hpBars[i]);
        this->game->window.draw(mpBars[i]);
        this->game->window.draw(hpTexts[i]);
        this->game->window.draw(mpTexts[i]);
    } 
    
    // Draw Enemies
    for (size_t i = 0; i < enemySprites.size(); ++i)
    {
        this->game->window.draw(enemySprites[i]);
        
        // Highlight if it's their turn
        if (!turnQueue.empty() && turnQueue.front() == &enemies[i]) {
            sf::RectangleShape highlightBox;
            sf::FloatRect bounds = enemySprites[i].getGlobalBounds();
            highlightBox.setSize(sf::Vector2f(bounds.width + 10.f, bounds.height + 10.f));
            highlightBox.setPosition(bounds.left - 5.f, bounds.top - 5.f);
            highlightBox.setFillColor(sf::Color(255, 255, 255, 50)); // translucent white
            highlightBox.setOutlineColor(sf::Color::Red);
            highlightBox.setOutlineThickness(2.f);

            this->game->window.draw(highlightBox);
        }
    }

    // Draw Turn Panel
    this->game->window.draw(turnPanelBackground);

    // Party portraits
    for (size_t i = 0; i < party.size(); ++i) {
        if (i < turnPortraitBoxes.size()) this->game->window.draw(turnPortraitBoxes[i]);
        if (i < turnPortraitSprites.size()) this->game->window.draw(turnPortraitSprites[i]);
    }

    // Enemy names
    for (auto& name : turnEnemyNames) {
        this->game->window.draw(name);
    }

    if (currentMenuState == BattleMenuState::Main) {
        attackButton.draw(this->game->window);
        skillButton.draw(this->game->window);
        itemButton.draw(this->game->window);
        guardButton.draw(this->game->window);
        escapeButton.draw(this->game->window);
    }
    else if (currentMenuState == BattleMenuState::Skill) {
        for (auto& b : skillButtons) b.draw(this->game->window);
        backButton.draw(this->game->window);
    }
    else if (currentMenuState == BattleMenuState::Item) {
        for (auto& b : itemButtons) b.draw(this->game->window);
        backButton.draw(this->game->window);
    }    
}
}

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
            Player* character = levelUpIterator->first;
            bool leveledUp = levelUpIterator->second;
            if (leveledUp){
                strengthVal = character->getSTR();
                strengthValPercent = (float)strengthVal / 99;

                vitalityVal = character->getVI();
                vitalityValPercent = (float)vitalityVal / 99;

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
                vitality.setString("VI              " + std::to_string(vitalityVal));
                viBar.setSize({500.0f * vitalityValPercent, 10.0f});
                agility.setString("AG             " + std::to_string(agilityVal));
                agBar.setSize({500.0f * agilityValPercent, 10.0f});
                luck.setString("LU             " + std::to_string(luckVal));
                luBar.setSize({500.0f * luckValPercent, 10.0f});
                maxHp.setString("Max HP                 " + std::to_string(maxHpVal) + "  ==>  " + std::to_string(recalculatedMaxHp));
                maxMp.setString("Max HP                 " + std::to_string(maxMpVal) + "  ==>  " + std::to_string(recalculatedMaxMp));

                 for (size_t x = 1; x < 9; x++){
                    auto* skill = character->getSkillsList()[x];
                    if(skill && skill->getName() != "EMPTY SLOT" && skill->getUnlockLevel() <= character->getLVL()){
                        skillNamesForResults[x-1].setString(skill->getName());
                    }
            }
            printSkillNames = true;
        }
    }
}
    else{
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
        int count = std::uniform_int_distribution<>(1, 4)(gen); // 1–4 enemies
        enemies = loadRandomEnemies(count);

        // Reserve to avoid reallocation invalidating addresses
        enemySprites.clear();
        enemyTextures.clear();
        enemySprites.reserve(enemies.size());

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

            enemySprites.push_back(spr);
        }

        // Populate turn queue by AGI: players then enemies (store pointers to elements)
        turnQueue.clear();
        for (auto* p : party) turnQueue.push_back(p);

        // ensure enemies vector will not be reallocated later unexpectedly
        // we already have enemies stored in this->enemies; store their addresses
        for (auto& e : enemies) turnQueue.push_back(&e);

        std::sort(turnQueue.begin(), turnQueue.end(),
                  [](Player* a, Player* b) { return a->getAGI() > b->getAGI(); });
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

        // small raise animations for portraits
        for (size_t j = 0; j < party.size() && j < turnPortraitBaseScales.size() && j < turnPortraitSprites.size(); ++j) {
            if (!turnQueue.empty() && turnQueue.front() == party[j]) {
                float scale = 1.1f * turnPortraitBaseScales[j];
                turnPortraitSprites[j].setScale(scale, scale);
            } else {
                turnPortraitSprites[j].setScale(turnPortraitBaseScales[j], turnPortraitBaseScales[j]);
            }
        }       
    }

    // Button Highlights
    attackButton.setHighlight(attackButton.isHovered(this->game->window));
    skillButton.setHighlight(skillButton.isHovered(this->game->window));
    itemButton.setHighlight(itemButton.isHovered(this->game->window));
    guardButton.setHighlight(guardButton.isHovered(this->game->window));
    escapeButton.setHighlight(escapeButton.isHovered(this->game->window));
}
}

void GameStateBattle::handleInput() {
    sf::Event event;
    while (this->game->window.pollEvent(event)) {

        // --- Close window
        if (event.type == sf::Event::Closed) {
            this->game->window.close();
            return;
        }

        // --- Key press events
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                this->game->requestPop();
                return;
            }
            else if (event.key.code == sf::Keyboard::Space) {
                // rotate queue: move front to back
                if (!turnQueue.empty()) {
                    Player* front = turnQueue.front();
                    turnQueue.pop_front();
                    turnQueue.push_back(front);
                }
                if (battleOver && distributionFinished && levelUpTime){
                    if (levelUpIterator == levelUpBooleanMap.end()){
                        this->game->requestPop();
                        return;
                    }
                    if (levelUpIterator != levelUpBooleanMap.end()){
                        ++levelUpIterator; 
                    }
                
                }
                if (battleOver && distributionFinished){
                    levelUpTime = true;
                }
            }
        
        }

        // --- Mouse click events
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

            if (currentMenuState == BattleMenuState::Main) {
                if (attackButton.wasClicked(this->game->window)) {
                    // do attack logic
                }
                else if (skillButton.wasClicked(this->game->window)) {
                    currentMenuState = BattleMenuState::Skill;
                }
                else if (itemButton.wasClicked(this->game->window)) {
                    currentMenuState = BattleMenuState::Item;
                }
            }

            else if (currentMenuState == BattleMenuState::Skill) {
                for (auto& b : skillButtons) {
                    if (b.wasClicked(this->game->window)) {
                        // skill selected
                    }
                }
                if (backButton.wasClicked(this->game->window)) {
                    currentMenuState = BattleMenuState::Main;
                }
            }

            else if (currentMenuState == BattleMenuState::Item) {
                for (auto& b : itemButtons) {
                    if (b.wasClicked(this->game->window)) {
                        // item selected
                    }
                }
                if (backButton.wasClicked(this->game->window)) {
                    currentMenuState = BattleMenuState::Main;
                }
            }
        }
    }
}

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
        selectedEnemies.emplace_back(
            e.value("name", "Unknown"),
            e.value("sprite", "default.png"),
            e.value("LVL", 1),
            e.value("STR", 1),
            e.value("VIT", 1),
            e.value("MAG", 0),
            e.value("AGI", 1),
            e.value("LU", 1),
            e.value("XP", 0),
            std::map<std::string,float>{{"fire",1.0f}, {"ice",1.0f}}  // placeholder
        );
    }
    return selectedEnemies;
}

void GameStateBattle::updateTurnPanel() {
    if (party.empty()) return; // safety
    if (turnPanelBackground.getSize().x == 0) return; // not ready
    if (turnPortraitBaseScales.empty()) return; // not yet initialized

    size_t totalActors = party.size() + enemies.size();
    if (totalActors == 0) return; // nothing to draw

    // Ensure turnPortraitBoxes/sprites at least have party-sized capacity (won't overwrite existing)
    if (turnPortraitBoxes.size() < party.size()) turnPortraitBoxes.resize(party.size());
    if (turnPortraitSprites.size() < party.size()) turnPortraitSprites.resize(party.size());

    float padding = 12.f;
    float portraitSize = 64.f;
    float availableHeight = turnPanelBackground.getSize().y - 2.f * padding;
    float spacingY = (totalActors > 0) ? std::min(portraitSize + padding, availableHeight / float(totalActors)) : (portraitSize + padding);

    // Party portraits (only if we have data for them)
    for (size_t i = 0; i < party.size(); ++i) {
        // safety: ensure we have a box/sprite slot
        if (i >= turnPortraitBoxes.size() || i >= turnPortraitSprites.size()) break;

        sf::RectangleShape &box = turnPortraitBoxes[i];
        sf::Sprite &spr = turnPortraitSprites[i];

        float x = turnPanelBackground.getPosition().x + 55.f;
        float y = turnPanelBackground.getPosition().y + padding + i * spacingY;

        box.setPosition(x, y);
        box.setSize({portraitSize, portraitSize});

        // Default scale fallback if we don't have a base scale
        float baseScale = 1.0f;
        if (i < turnPortraitBaseScales.size() && turnPortraitBaseScales[i] > 0.f)
            baseScale = turnPortraitBaseScales[i];

        // highlight active
        float finalScale = baseScale;
        if (!turnQueue.empty() && turnQueue.front() == party[i]) finalScale *= 1.1f;

        spr.setScale(finalScale, finalScale);

        // Only position sprite if it has a texture
        if (spr.getTexture() != nullptr) {
            const auto lb = spr.getLocalBounds();
            float sprW = lb.width * spr.getScale().x;
            float sprH = lb.height * spr.getScale().y;
            // center inside box
            spr.setPosition(
                x + (portraitSize - sprW) / 2.f,
                y + (portraitSize - sprH) / 2.f
            );
        } else {
            // No texture: put sprite at box origin so it doesn't produce NaNs later
            spr.setPosition(x, y);
        }
    }

    // Enemy names
    turnEnemyNames.clear();
    for (size_t ei = 0; ei < enemies.size(); ++ei) {
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setCharacterSize(18);
        nameText.setFillColor(sf::Color::White);

        // Use a safe public getter (don't access protected fields directly)
        nameText.setString(enemies[ei].getDisplayName()); // make/get this getter in NPC

        float x = turnPanelBackground.getPosition().x + 55.f;
        float y = turnPanelBackground.getPosition().y + padding + (party.size() + ei) * spacingY;
        nameText.setPosition(x, y);

        // Highlight if it's their turn
        if (!turnQueue.empty() && turnQueue.front() == &enemies[ei]) {
            nameText.setCharacterSize(22);
            nameText.setFillColor(sf::Color::Green);
        }
        turnEnemyNames.push_back(std::move(nameText));
    }
}
