#include "game_state_battle.hpp"
#include "game_state_editor.hpp"   
#include <iostream>
#include <algorithm>

GameStateBattle::GameStateBattle(Game* game, bool isBossBattle) {
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
    textBox.setOutlineColor(sf::Color::White);
    textBox.setFillColor(sf::Color::Black);
    textBox.setPosition(85.f, 620.f);
    
    // Enemy Background
    enemyBackground.setSize({1907.f, 550.f});
    enemyBackground.setOutlineThickness(2.f);
    enemyBackground.setOutlineColor(sf::Color::White);
    enemyBackground.setPosition(5.f, 50.f);

    // Background 
    background.setSize(sf::Vector2f(game->window.getSize()));
    background.setFillColor(sf::Color::Black);

    // Texture loading
    // enemyBackgroundTex.loadFromFile("assets/wall_texture.jpg");
    this->game->texmgr.loadTexture("enemy_bg", "./assets/backgrounds/border_dw_titan_base_0.png");
    enemyBackground.setTexture(&this->game->texmgr.getRef("enemy_bg"));

    enemyBackground.setTextureRect(sf::IntRect(0, 0, enemyBackground.getSize().x, enemyBackground.getSize().y));

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
        bgBox.setOutlineColor(sf::Color::White);
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

        // Load it once through the texture manager
        this->game->texmgr.loadTexture(texName, path);
        icon.setTexture(this->game->texmgr.getRef(texName));
    
        // Position and scale icon inside the red box
        float targetHeight = 70.f;
        float textureHeight = icon.getLocalBounds().height;
        float scale = targetHeight / textureHeight;
        icon.setScale(scale, scale);
        icon.setPosition(xOffset - 15.f, yOffset - 10.f);
        portraitBaseScales.push_back(scale);


        // Place bars below the icon
        hpBar.setPosition(xOffset, yOffset + 80.f);
        mpBar.setPosition(xOffset, yOffset + 100.f);

        // Place HP/MP text next to bars
        hpText.setPosition(xOffset + 110.f, yOffset + 70.f);
        mpText.setPosition(xOffset + 110.f, yOffset + 90.f);
    
        // Store elements
        playerBackgrounds.push_back(bgBox);
        playerIcons.push_back(icon);
        hpBars.push_back(hpBar);
        mpBars.push_back(mpBar);
        hpTexts.push_back(hpText);
        mpTexts.push_back(mpText);
    }    

    // Turn Order UI ------------
    float panelW = 220.f;
    float panelH = 310.f;
    turnPanelBackground.setSize({panelW, panelH});
    turnPanelBackground.setFillColor(sf::Color(20,20,20, 200));
    turnPanelBackground.setOutlineColor(sf::Color::White);
    turnPanelBackground.setOutlineThickness(2.f);

    // panel position
    float marginRight = 40.f;
    turnPanelBackground.setPosition(static_cast<float>(this->game->window.getSize().x) - panelW - marginRight, 735.f);

    // Portrait Boxes
    float portraitSize = 64.f;
    float portraitPadding = 12.f;
    for (size_t i = 0; i < party.size(); ++i) {
        sf::RectangleShape box({portraitSize, portraitSize});
        box.setFillColor(sf::Color::Transparent);

        float x = turnPanelBackground.getPosition().x + 20.f;
        float y = turnPanelBackground.getPosition().y + 12.f + i * (portraitSize + portraitPadding);
        box.setPosition(x,y);

        sf::Sprite spr;

        if (i == 0) spr.setTexture(this->game->texmgr.getRef("playerSprite"));
        else if (i == 1) spr.setTexture(this->game->texmgr.getRef("pmember2Sprite"));
        else if (i == 2) spr.setTexture(this->game->texmgr.getRef("pmember3Sprite"));
        else if (i == 3) spr.setTexture(this->game->texmgr.getRef("pmember4Sprite"));
        
        sf::FloatRect lb = spr.getLocalBounds();
        if (lb.height > 0.0001f) {
            float scale = portraitSize / lb.height * 0.65f;
            spr.setScale(scale, scale);
            turnPortraitBaseScales.push_back(scale);
        }

        spr.setPosition(x + (portraitSize - spr.getLocalBounds().width * spr.getScale().x) / 2.f,
        y + (portraitSize - spr.getLocalBounds().height * spr.getScale().y) / 2.f);

        turnPortraitBoxes.push_back(box);
        turnPortraitSprites.push_back(spr);
    }

    // --- Compute initial turn order by AGI
    std::vector<std::pair<int, Player*>> tmp;
    for (auto* p : party)
        tmp.emplace_back(p->getAGI(), p);

    std::sort(tmp.begin(), tmp.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });

    turnQueue.clear();
    for (auto& pr : tmp)
        turnQueue.push_back(pr.second);

    // highlight first actor
    currentTurnIndex = 0;

}

void GameStateBattle::draw(const float dt) {
    this->game->window.clear();
    this->game->window.draw(background);
    this->game->window.draw(enemyBackground);
    this->game->window.draw(textBox);
    this->game->window.draw(battleText);

    for (size_t i = 0; i < party.size(); ++i) {
        this->game->window.draw(playerBackgrounds[i]);
        this->game->window.draw(playerIcons[i]);
        this->game->window.draw(hpBars[i]);
        this->game->window.draw(mpBars[i]);
        this->game->window.draw(hpTexts[i]);
        this->game->window.draw(mpTexts[i]);
    }    

    this->game->window.draw(turnPanelBackground);
    for (size_t i = 0; i < turnPortraitBoxes.size(); ++i) {
        this->game->window.draw(turnPortraitBoxes[i]);
        this->game->window.draw(turnPortraitSprites[i]);
        // Optional highlight
    }    
}

void GameStateBattle::update(const float dt) {
    for (size_t i = 0; i < party.size(); ++i) {
        auto* p = party[i];
        float hpPercent = static_cast<float>(p->getHP()) / p->getmaxHP();
        float mpPercent = static_cast<float>(p->getMP()) / p->getmaxMP();
    
        hpBars[i].setSize({100.f * hpPercent, 10.f});
        mpBars[i].setSize({100.f * mpPercent, 10.f});
    
        hpTexts[i].setString(std::to_string(p->getHP()) + "/" + std::to_string(p->getmaxHP()));
        mpTexts[i].setString(std::to_string(p->getMP()) + "/" + std::to_string(p->getmaxMP()));
    }    

    // Highlight the active player's UI box
    Player* active = nullptr;
    if (!turnQueue.empty()) active = turnQueue.front();

    for (size_t i = 0; i < party.size(); ++i) {
        float raise = 0.f;
        if (active == party[i]) {
            raise = -16.f;
            playerBackgrounds[i].setOutlineColor(sf::Color::Red);
            playerBackgrounds[i].setOutlineThickness(3.f);
        } else {
            playerBackgrounds[i].setOutlineColor(sf::Color::White);
            playerBackgrounds[i].setOutlineThickness(2.f);           
        }
        sf::Vector2f base = basePositions[i];
        playerBackgrounds[i].setPosition(base.x, base.y + raise);
        playerIcons[i].setPosition(playerIcons[i].getPosition().x, base.y - 10.f + raise);
        hpBars[i].setPosition(hpBars[i].getPosition().x, base.y + 80.f + raise);
        mpBars[i].setPosition(mpBars[i].getPosition().x, base.y + 100.f + raise);
        hpTexts[i].setPosition(hpTexts[i].getPosition().x, base.y + 70.f + raise);
        mpTexts[i].setPosition(mpTexts[i].getPosition().x, base.y + 90.f + raise);
        // small raise
        

        for (size_t j = 0; j < turnPortraitSprites.size(); ++j) {
            if (!turnQueue.empty() && turnQueue.front() == party[j]) {
                float scale = 1.1f * turnPortraitBaseScales[j];
                turnPortraitSprites[j].setScale(scale, scale);
            } else {
                turnPortraitSprites[j].setScale(turnPortraitBaseScales[j], turnPortraitBaseScales[j]);
            }
        }       
    }
}


void GameStateBattle::handleInput() {
    sf::Event event;
    while (this->game->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            this->game->window.close();
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            this->game->requestChange(std::make_unique<GameStateEditor>(this->game,false));
            return;
        }   
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
            // rotate queue: move front to back
            if (!turnQueue.empty()) {
                Player* front = turnQueue.front();
                turnQueue.pop_front();
                turnQueue.push_back(front);
            }
        }             
    }
}
