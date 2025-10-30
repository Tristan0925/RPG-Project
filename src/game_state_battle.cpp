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
    battleText.setPosition(850.f, 50.f);

    // Text box
    textBox.setSize({1750.f, 100.f});
    textBox.setOutlineThickness(2.f);
    textBox.setOutlineColor(sf::Color::White);
    textBox.setFillColor(sf::Color::Black);
    textBox.setPosition(85.f, 50.f);

    // Enemy Background
    enemyBackground.setSize({1907.f, 350.f});
    enemyBackground.setOutlineThickness(2.f);
    enemyBackground.setOutlineColor(sf::Color::White);
    enemyBackground.setFillColor(sf::Color::Black);
    enemyBackground.setPosition(5.f, 250.f);

    // Background 
    background.setSize(sf::Vector2f(game->window.getSize()));
    background.setFillColor(sf::Color::Black);

    // ------------
    // Player 1
    // ------------

    // Player portrait background
    playerBackground.setPrimitiveType(sf::Quads);
    playerBackground.resize(4);
    playerBackground[0].position = sf::Vector2f(1960.0f, 260.0f);
    playerBackground[1].position = sf::Vector2f(1960.0f, 350.0f);
    playerBackground[2].position = sf::Vector2f(1730.0f, 350.0f);
    playerBackground[3].position = sf::Vector2f(1730.0f, 260.0f);
    playerBackground[0].color = sf::Color(255,0,0,200);
    playerBackground[1].color = sf::Color(255,0,0,200);
    playerBackground[2].color = sf::Color(0,0,0,200);
    playerBackground[3].color = sf::Color(0,0,0,200);
  
    // Player sprite
    playerSprite = this->game->playerSprite;
    playerSprite.setPosition(1770.0f, 260.0f);
  
    // HP bar setup
    playerHPBarBackground.setSize({100.f, 10.f});
    playerHPBarBackground.setPosition(1770.f, 310.f);
    playerHPBarBackground.setFillColor(sf::Color(51,51,51));
    playerHPBarBackground.setOutlineThickness(1.2f);
    playerHPBarBackground.setOutlineColor(sf::Color::Black);
  
    playerHPBar.setFillColor(sf::Color(127,255,0));
    playerHPBar.setPosition(1770.f, 310.f);
  
    // MP bar setup
    playerMPBarBackground.setSize({100.f, 10.f});
    playerMPBarBackground.setPosition(1770.f, 330.f);
    playerMPBarBackground.setFillColor(sf::Color(51,51,51));
    playerMPBarBackground.setOutlineThickness(1.2f);
    playerMPBarBackground.setOutlineColor(sf::Color::Black);
  
    playerMPBar.setFillColor(sf::Color(0,0,255));
    playerMPBar.setPosition(1770.f, 330.f);
  
    // HP/MP text
    playerHP.setFont(this->game->font);
    playerHP.setCharacterSize(18);
    playerHP.setPosition(1735.f, 305.f);
  
    playerMP.setFont(this->game->font);
    playerMP.setCharacterSize(18);
    playerMP.setPosition(1735.f, 323.f);
}

void GameStateBattle::draw(const float dt) {
    this->game->window.clear();
    this->game->window.draw(background);
    this->game->window.draw(enemyBackground);
    this->game->window.draw(textBox);
    this->game->window.draw(battleText);

    // Texture loading
    enemyBackgroundTex.loadFromFile("assets/wall_texture.jpg");

    // Player UI
    this->game->window.draw(playerBackground);
    this->game->window.draw(playerHPBarBackground);
    this->game->window.draw(playerHPBar);
    this->game->window.draw(playerMPBarBackground);
    this->game->window.draw(playerMPBar);
    this->game->window.draw(playerSprite);
    this->game->window.draw(playerHP);
    this->game->window.draw(playerMP);

    this->game->window.display();
}

void GameStateBattle::update(const float dt) {
    float hpPercent = static_cast<float>(player->getHP()) / player->getmaxHP();
    float mpPercent = static_cast<float>(player->getMP()) / player->getmaxMP();

    playerHPBar.setSize({100.f * hpPercent, 10.f});
    playerMPBar.setSize({100.f * mpPercent, 10.f});

    playerHP.setString(std::to_string(player->getHP()) + "/" + std::to_string(player->getmaxHP()));
    playerMP.setString(std::to_string(player->getMP()) + "/" + std::to_string(player->getmaxMP()));
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
