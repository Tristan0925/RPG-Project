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
:   topBarTextBackground(sf::Quads,4), 
    thingsEarnedBackground(sf::Quads,4),
    attackButton("Attack", {150.f, 800.f}, 30, game, sf::Color::White),
    skillButton("Skill",  {150.f, 840.f}, 30, game, sf::Color::White),
    itemButton("Item",   {150.f, 880.f}, 30, game, sf::Color::White),
    guardButton("Guard", {350.f, 800.f}, 30, game, sf::Color::White),
    escapeButton("Escape", {350.f, 840.f}, 30, game, sf::Color::White),
    backButton("Back", {350.f, 1000.f}, 30, game, sf::Color::White)
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

    // Music
    if (!currentMusic.openFromFile("./assets/music/normalbattle.mp3")) {
        std::cout << "Could not load music file" << std::endl;
    } else {
        currentMusic.setLoop(true);
        currentMusic.play();
    }
    
    // Back button (reconfigure if you want different color)
    backButton = Button("Back", {150.f, 1000.f}, 30, this->game, sf::Color(90, 90, 90));

    // --- Populate Skills dynamically from player's learned skills ---
    skillButtons.clear();
    float skillY = 300.f;
    for (const auto& skillName : this->game->playerSkills) {
        skillButtons.emplace_back(skillName, sf::Vector2f(150.f, skillY), 30, this->game, sf::Color::White);
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

}

void GameStateBattle::displayResultsScreen(bool displayResults){
    
    this->game->window.draw(topBarTextBackground);
    this->game->window.draw(topBarText);
    this->game->window.draw(thingsEarnedBackground);
    this->game->window.draw(totalEarnedExp);
   
}




void GameStateBattle::draw(const float dt) {
    if (battleOver){
        if (!playResultsMusic){
            currentMusic.stop();
            if (!currentMusic.openFromFile("./assets/music/battleresults.mp3")) std::cout << "Could not load music file" << std::endl;
            else {
                currentMusic.setLoop(true);
                currentMusic.play();
                playResultsMusic = true;
                }
}
         displayResultsScreen(true);
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

    // --- FIX: Draw Turn Panel elements separately ---

    // 1. Draw Player Portraits (using the pre-sized vector)
    for (size_t i = 0; i < turnPortraitBoxes.size(); ++i) {
        // Only draw player portraits and boxes if they have a non-zero size
        if (turnPortraitBoxes[i].getSize().x > 0.f) {
            this->game->window.draw(turnPortraitBoxes[i]);
            if (i < turnPortraitSprites.size() && turnPortraitSprites[i].getTexture() != nullptr) {
                this->game->window.draw(turnPortraitSprites[i]);
            }
        }
    }

    // 2. Draw Enemy Names (using the push_backed vectors)
    for (size_t i = 0; i < enemyNameBackgrounds.size(); ++i) {
        this->game->window.draw(enemyNameBackgrounds[i]);
        if (i < turnEnemyNames.size()) {
            this->game->window.draw(turnEnemyNames[i]);
        }
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
                this->game->requestChange(std::make_unique<GameStateEditor>(this->game, false));
                return;
            }
            else if (event.key.code == sf::Keyboard::Space) {
                // rotate queue: move front to back
                if (!turnQueue.empty()) {
                    Player* front = turnQueue.front();
                    turnQueue.pop_front();
                    turnQueue.push_back(front);
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

    // --- FIX: Clear all draw vectors ---
    turnPortraitBoxes.clear();
    turnPortraitSprites.clear();
    enemyNameBackgrounds.clear();
    turnEnemyNames.clear();

    // --- FIX: Resize player vectors to match the full turn list size ---
    // We will "blank" the ones that are enemies
    turnPortraitBoxes.resize(turnList.size());
    turnPortraitSprites.resize(turnList.size());

    // --- Layout ---
    float panelX = turnPanelBackground.getPosition().x;
    float panelY = turnPanelBackground.getPosition().y;
    float padding = 16.f;
    float portraitSize = 25.f;       // <-- Your small size (e.g., 50.f)
    float entrySpacingY = 4.f;       // <-- Your small spacing (e.g., 8.f)

    // --- FIX: Use a running Y-coordinate for dynamic spacing ---
    float currentY = panelY + padding;

    for (size_t i = 0; i < turnList.size(); ++i) {
        const auto& entry = turnList[i];
        float x = panelX + 70.f;
        float y = currentY; // Use the running Y position

        // --- FIX: Ensure every entry (player or enemy) has the same height ---
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

            // --- FIX: Vertically center the text within the standard entryHeight ---
            float textCenterY = y + (entryHeight / 2.f) - (textBounds.height / 2.f) - 4.f; // Adjust -4.f as needed for font alignment
            nameText.setPosition(x + 6.f, textCenterY);

            sf::RectangleShape bg;
            // --- FIX: Make the background box use the standard entryHeight ---
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

            // --- FIX: Mark this slot as "not a player" by setting size to 0 ---
            if (i < turnPortraitBoxes.size()) turnPortraitBoxes[i].setSize({0.f, 0.f});
            if (i < turnPortraitSprites.size()) turnPortraitSprites[i].setTextureRect(sf::IntRect(0, 0, 0, 0));
        }
        
        // --- FIX: Advance Y-coordinate by the standard height *after* the if/else block ---
        currentY += entryHeight + entrySpacingY;
    }

    // --- Draw everything (REMOVED: Drawing is now handled in the main GameStateBattle::draw() function) ---
}