#include "game_state_battle.hpp"
#include "game_state_editor.hpp"   
#include <iostream>

GameStateBattle::GameStateBattle(Game* game) {
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
    }
}
