#include "game_state_battle.hpp"
#include "game_state_editor.hpp"   
#include <iostream>


GameStateBattle::GameStateBattle(Game* game) {
    this->game = game;
    this->player = &game->player;

    // Load font
    if (!font.loadFromFile("assets/edosz.ttf")) {
        // fallback if font fails
        throw std::runtime_error("Failed to load font for battle state!");
    }

    // Setup battle text
    battleText.setFont(font);
    battleText.setString("BATTLE MODE!\nPress Enter to return.");
    battleText.setCharacterSize(36);
    battleText.setFillColor(sf::Color::White);
    battleText.setPosition(100.f, 100.f);

    // Background (red screen to make it obvious)
    background.setSize(sf::Vector2f(game->window.getSize()));
    background.setFillColor(sf::Color(150, 0, 0));
}

void GameStateBattle::draw(const float dt) {
    this->game->window.clear();
    this->game->window.draw(background);
    this->game->window.draw(battleText);
    this->game->window.display();
}

void GameStateBattle::update(const float dt) {
    // Nothing dynamic yet
}

void GameStateBattle::handleInput() {
    std::cout << "GameStateBattle::handleInput called" << std::endl;
    sf::Event event;
    while (this->game->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            this->game->window.close();
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            std::cout << "Exiting battle, going back to exploration..." << std::endl;
            this->game->requestChange(std::make_unique<GameStateEditor>(this->game));
            return;
        }        
    }
}
