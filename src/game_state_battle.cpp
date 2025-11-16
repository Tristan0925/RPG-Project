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
    battleText.setString("BATTLE MODE!\nPress Enter to return.");
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
        // turnPortraitBaseScales[i] = scale; // Removed assignment, scale calculated in updateTurnPanel

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
            // turnPortraitSprites[i].setScale(scale, scale); // REMOVED: Scale will be calculated in updateTurnPanel
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

    // enemy index
    currentEnemyIndex = getFirstLivingEnemy();

    // Music
    if (!currentMusic.openFromFile("./assets/music/normalbattle.mp3")) {
        std::cout << "Could not load music file" << std::endl;
    } else {
        currentMusic.setLoop(true);
        currentMusic.play();
    }
    

    // --- Populate Skill Buttons (filtered by unlock level, 2-column layout) ---
    skillButtons.clear();

    // Layout parameters
    float baseX = 150.f;
    float baseY = 780.f;
    float offsetY = 55.f;        // vertical distance between buttons
    float columnSpacing = 260.f; // distance between columns

    int playerLevel = this->game->player.getLVL();

    size_t visibleIndex = 0;
    
    // Loop through skill names (strings)
    
    for (size_t i = 1; i < this->game->playerSkills.size(); ++i) {
        const Skill* s = this->game->player.getSkillPtr(this->game->playerSkills[i], this->game->skillMasterList);

        if (!s) continue;
    
        if (playerLevel < s->getUnlockLevel())
            continue;
    
        // 2-column layout
        size_t col = visibleIndex % 2;
        size_t row = visibleIndex / 2;
        float x = baseX + col * columnSpacing;
        float y = baseY + row * offsetY;
    
        skillButtons.emplace_back(s->getName(), sf::Vector2f(x, y), 28, game, sf::Color::White);
        visibleIndex++;
    }    

    // now set positions for skill buttons if needed
    {
        float baseX = 150.f;
        float baseY = 800.f;
        float offsetY = 50.f;  // vertical spacing between buttons
        float columnSpacing = 250.f; // space between columns

        for (size_t k = 1; k < skillButtons.size(); ++k) {
            size_t col = k % 2; // 0 = left column, 1 = right column
            size_t row = k / 2; // go down every two items
            float x = baseX + col * columnSpacing;
            float y = baseY + row * offsetY;
            skillButtons[k].changePosition(x, y);
        }
    }

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
    topBarText.setString("Results \t\t\t\t\t\t\t\t\t\t\t\t Obtained the following:");
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
    totalEarnedExpMessage = "EXP                                                                                                                                                                0";  // + std::to_string(totalXpGained) or whatever it is
    totalEarnedExp.setString(totalEarnedExpMessage);
    totalEarnedExp.setCharacterSize(75);
    totalEarnedExp.setFillColor(sf::Color::Red);
    totalEarnedExp.setPosition(25.0f,250.0f);

    //set up all the necessary variables for displaying the player + party
    playerName.setFont(font);
    playerName.setString(this->game->player.getName());
    playerName.setCharacterSize(25);
    playerName.setFillColor(sf::Color(130,25,13));

    pmember2Name.setFont(font);
    pmember2Name.setString(this->game->pmember2.getName());
    pmember2Name.setCharacterSize(25);
    pmember2Name.setFillColor(sf::Color(130,25,13));

    pmember3Name.setFont(font);
    pmember3Name.setString(this->game->pmember3.getName());
    pmember3Name.setCharacterSize(25);
    pmember3Name.setFillColor(sf::Color(130,25,13));

    pmember4Name.setFont(font);
    pmember4Name.setString(this->game->pmember4.getName());
    pmember4Name.setCharacterSize(25);
    pmember4Name.setFillColor(sf::Color(130,25,13));

    // Game over initialization
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(400.f, 200.f); // adjust for center
    gameOver = false;
    gameOverMenuState = GameOverMenuState::Main;

}

// Results Screen
void GameStateBattle::displayResultsScreen(bool displayResults){
    
    this->game->window.draw(topBarTextBackground);
    this->game->window.draw(topBarText);
    this->game->window.draw(thingsEarnedBackground);
    this->game->window.draw(totalEarnedExp);
   
}

// Draw 
void GameStateBattle::draw(const float dt) {
    // Always clear the window first
    this->game->window.clear(sf::Color::Black);

    // Battle finished / results screen
    if (battleOver) {
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

        displayResultsScreen(true);
        this->game->window.display(); // commit the frame
        return;
    }

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


// Update
void GameStateBattle::update(const float dt) {
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
        // we already have enemies stored in this->enemies; store their addresses
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

        // small raise animations for portraits
        for (size_t j = 0; j < party.size() && j < turnPortraitBaseScales.size() && j < turnPortraitSprites.size(); ++j) {
            if (!turnQueue.empty() && turnQueue.front() == party[j]) {
                // float scale = 1.1f * turnPortraitBaseScales[j]; // This logic is now in updateTurnPanel
            } else {
                // This logic is now in updateTurnPanel
            }
        }       
    }

    // Button Highlights
    attackButton.setHighlight(attackButton.isHovered(this->game->window));
    skillButton.setHighlight(skillButton.isHovered(this->game->window));
    itemButton.setHighlight(itemButton.isHovered(this->game->window));
    guardButton.setHighlight(guardButton.isHovered(this->game->window));
    escapeButton.setHighlight(escapeButton.isHovered(this->game->window));

    // dynamically update skill descriptions
    // Show skill info on hover
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

    // ---- Enemy AI turns
    // We only want to process an enemy's turn once when they become the active actor.
    // We'll use a static pointer to remember who acted last update.
    static Player* lastActor = nullptr;
    if (!turnQueue.empty()) {
        Player* currentActor = turnQueue.front();
        if (currentActor != lastActor) {
            // new actor this frame
            lastActor = currentActor;

            // If currentActor is an NPC from enemies (i.e., an enemy), perform AI action immediately
            bool actorIsEnemy = (std::find(party.begin(), party.end(), currentActor) == party.end());
            if (actorIsEnemy) {
                // Find which enemy in our enemies vector this corresponds to
                NPC* actingEnemy = nullptr;
                for (auto& e : enemies) {
                    if (&e == currentActor) { actingEnemy = &e; break; }
                }

                if (actingEnemy && !actingEnemy->isDead()) {
                    // Choose random living party member target
                    std::vector<Player*> livingTargets;
                    for (auto* p : party) if (p && p->getHP() > 0) livingTargets.push_back(p);

                    if (!livingTargets.empty()) {
                        std::uniform_int_distribution<> dis(0, static_cast<int>(livingTargets.size()) - 1);
                        Player* target = livingTargets[dis(globalRng())];

                        // Use the "Attack" skill if present; fallback baseAtk/crit defaults
                        const Skill* atkSkill = findSkillByName(this->game, "Attack");
                        int baseAtk = atkSkill ? atkSkill->getBaseAtk() : 15;
                        float critChance = atkSkill ? atkSkill->getCritRate() : 0.05f;

                        // compute damage using attacking enemy's physATK (enemy inherits Player)
                        int damage = actingEnemy->physATK(1.0f, baseAtk, false);

                        // crit roll (simple)
                        std::uniform_real_distribution<float> cr(0.f, 1.f);
                        bool isCrit = (cr(globalRng()) < critChance);
                        if (isCrit) {
                            damage = static_cast<int>(damage * 1.5f);
                        }

                        target->takeDamage(damage);

                        // spawn damage popup above target's approximate screen position
                        DamagePopup dp;
                        dp.text.setFont(font);
                        dp.text.setCharacterSize(28);
                        dp.text.setString((isCrit ? std::string("CRIT ") : std::string("")) + std::to_string(damage));
                        dp.text.setFillColor(sf::Color::Red);
                        // approximate target screen position: we only have player icons; place near corresponding icon
                        // We'll try to place popup above the first matching playerIcon (best effort)
                        sf::Vector2f popupPos(200.f, 700.f); // fallback
                        for (size_t i = 0; i < playerIcons.size(); ++i) {
                            if (i < party.size() && party[i] == target) {
                                popupPos = playerIcons[i].getPosition() - sf::Vector2f(0.f, 40.f);
                                break;
                            }
                        }
                        dp.text.setPosition(popupPos);
                        dp.velocity = sf::Vector2f(0.f, -30.f);
                        dp.life = 1.0f;
                        damagePopups.push_back(dp);

                        // message
                        battleText.setString(actingEnemy->getName() + " attacked " + target->getName() + " for " + std::to_string(damage) + "!");
                        cleanupDeadEnemies();
                    }

                    // rotate queue (end enemy's turn)
                    if (!turnQueue.empty()) {
                        Player* front = turnQueue.front();
                        turnQueue.pop_front();
                        turnQueue.push_back(front);
                    }
                } else {
                    // If acting enemy is dead or invalid, just pop them out
                    for (auto it = turnQueue.begin(); it != turnQueue.end();) {
                        if (*it == actingEnemy) it = turnQueue.erase(it);
                        else ++it;
                    }
                }
            }
        }
    }
    if (player->getHP() <= 0 && !gameOver) {
        gameOver = true;

        // Stop battle music
        currentMusic.stop();
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
                    }
                    else if (slot2.wasClicked(this->game->window)) {
                        this->game->player.loadFromFile("save2.json", this->game->skillMasterList);
                        loadMenuActive = false;
                        gameOver = false;
                    }
                    else if (slot3.wasClicked(this->game->window)) {
                        this->game->player.loadFromFile("save3.json", this->game->skillMasterList);
                        loadMenuActive = false;
                        gameOver = false;
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
            if (event.key.code == sf::Keyboard::Enter) {
                this->game->requestChange(std::make_unique<GameStateEditor>(this->game, false));
                return;
            }
            else if (event.key.code == sf::Keyboard::Space) {
                if (!turnQueue.empty()) {
                    Player* front = turnQueue.front();
                    turnQueue.pop_front();
                    turnQueue.push_back(front);
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
                    int baseAtk = atkSkill ? atkSkill->getBaseAtk() : 15;
                    float critChance = atkSkill ? atkSkill->getCritRate() : 0.05f;

                    // compute damage using attacking player's physATK (player inherits Player)
                    int damage = attacker->physATK(1.0f, baseAtk, false);

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
                    dp.text.setFillColor(isCrit ? sf::Color::Yellow : sf::Color::White);

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
                    }

                    return; // end click event
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

  

// Loading random enemies
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
