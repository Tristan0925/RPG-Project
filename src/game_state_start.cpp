//This represents the main menu screen. game_state_start.cpp

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Button.hpp"
#include "Player.hpp"
#include "game_state_start.hpp"
#include "game_state_editor.hpp"
#include "game_state.hpp"
#include <iostream>

void GameStateStart::draw(const float dt)
{
    this->game->window.setView(this->game->window.getDefaultView());
    this->game->window.clear(sf::Color::Black);
    this->game->window.draw(this->game->background);

    this->game->window.draw(title);

    if (!slotMenuActive) { // main menu buttons
        startgame.draw(this->game->window);
        loadButton.draw(this->game->window);
        settings.draw(this->game->window);
        endgame.draw(this->game->window);

        if(startgame.isHovered(this->game->window)) this->game->window.draw(startgame.getUnderline());
        if(loadButton.isHovered(this->game->window)) this->game->window.draw(loadButton.getUnderline());
        if(settings.isHovered(this->game->window)) this->game->window.draw(settings.getUnderline());
        if(endgame.isHovered(this->game->window)) this->game->window.draw(endgame.getUnderline());
    } 
    else { // slot menu buttons
        slot1.draw(this->game->window);
        slot2.draw(this->game->window);
        slot3.draw(this->game->window);
        backButton.draw(this->game->window);

        if(slot1.isHovered(this->game->window)) this->game->window.draw(slot1.getUnderline());
        if(slot2.isHovered(this->game->window)) this->game->window.draw(slot2.getUnderline());
        if(slot3.isHovered(this->game->window)) this->game->window.draw(slot3.getUnderline());
        if(backButton.isHovered(this->game->window)) this->game->window.draw(backButton.getUnderline());
    }
    return;
}

void GameStateStart::update(const float dt)
{
    // Could add animations here later
}

void GameStateStart::handleInput()
{
    sf::Event event;
    while(this->game->window.pollEvent(event)) {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

            if (!slotMenuActive) { // main menu
                if (startgame.wasClicked(this->game->window)) {
                    Map map;
                    map.loadFromFile("assets/map1.txt");
                    this->game->player.setDefault(map);
                    requestStartGame = true;
                }

                if (loadButton.wasClicked(this->game->window)) {
                    slotMenuActive = true; // show the save/load slot menu
                }

                if (settings.wasClicked(this->game->window)) {
                    std::cout << "Whenever we have settings to change, put it here." << std::endl;
                }

                if (endgame.wasClicked(this->game->window)) {
                    this->game->window.close();
                }
            }
            else { // slot menu active
                if (slot1.wasClicked(this->game->window)) {
                    this->game->loadFromFile("save1.json", this->game->skillMasterList);
                    mainTheme.stop();
                    this->game->changeState(std::make_unique<GameStateEditor>(this->game, requestStartGame, this->game->floorNumber));
                    return;
                }
                if (slot2.wasClicked(this->game->window)) {
                    this->game->loadFromFile("save2.json", this->game->skillMasterList);
                    mainTheme.stop();
                    this->game->changeState(std::make_unique<GameStateEditor>(this->game, requestStartGame, this->game->floorNumber));
                    return;
                }
                if (slot3.wasClicked(this->game->window)) {
                    this->game->loadFromFile("save3.json", this->game->skillMasterList);
                    mainTheme.stop();
                    this->game->changeState(std::make_unique<GameStateEditor>(this->game, requestStartGame, this->game->floorNumber));
                    return;
                }
                if (backButton.wasClicked(this->game->window)) {
                    slotMenuActive = false; // go back to main menu
                }
            }
        }
        

        // Close or resize window
        switch(event.type)
        {
            case sf::Event::Closed:
                game->window.close();
                break;

            case sf::Event::Resized:
            {
                this->view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
                this->view.setCenter(this->view.getSize() / 2.f);
                this->game->window.setView(this->view);
                this->game->background.setPosition(0.f, 0.f);
                this->game->background.setScale(
                    float(event.size.width) / this->game->background.getTexture()->getSize().x,
                    float(event.size.height) / this->game->background.getTexture()->getSize().y
                );

                // Scale & reposition main menu elements
                float scaleRatio = float(event.size.width) / this->game->background.getTexture()->getSize().x;
                title.setPosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1500.f * scaleRatio));
                startgame.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1200.f * scaleRatio));
                loadButton.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1100.f * scaleRatio));
                settings.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1000.f * scaleRatio));
                endgame.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (900.f * scaleRatio));

                // Slot buttons
                slot1.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1200.f * scaleRatio));
                slot2.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1100.f * scaleRatio));
                slot3.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1000.f * scaleRatio));
                backButton.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (900.f * scaleRatio));

                break;
            }

            case sf::Event::KeyPressed:
                if(event.key.code == sf::Keyboard::Escape) this->game->window.close();
                break;

            default: break;
        }
    }
    if (requestStartGame) {
        this->game->changeState(std::make_unique<GameStateEditor>(this->game, requestStartGame, this->game->floorNumber));
        return; // stop further input for this frame
    }
}


GameStateStart::GameStateStart(Game* game):
  startgame("Start Game", sf::Vector2f(0.f,0.f), 34, game),
  loadButton("Load Game", sf::Vector2f(0.f,0.f), 34, game), 
  settings("Settings", sf::Vector2f(0.f,0.f), 34, game),
  endgame("End Game", sf::Vector2f(0.f,0.f), 34, game),
  slot1("Slot 1", sf::Vector2f(100.f, 200.f), 34, game),
  slot2("Slot 2", sf::Vector2f(100.f, 250.f), 34, game),
  slot3("Slot 3", sf::Vector2f(100.f, 300.f), 34, game),
  backButton("Back", sf::Vector2f(100.f, 350.f), 34, game)
{
    this->game = game;

    // Setup view
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->view.setSize(pos);
    pos *= 0.5f;
    this->view.setCenter(pos);

    // Setup title
    title.setFont(this->game->font);
    title.setString("Sloth Megami Tensei IV:\nMegatherium");
    title.setCharacterSize(100); 
    title.setFillColor(sf::Color::Red);
    title.setStyle(sf::Text::Italic | sf::Text::Bold);
    title.setScale(0.5f, 0.5f); // fixed size

    // Position everything the same as on initial startup
    float width = static_cast<float>(this->game->window.getSize().x);
    float height = static_cast<float>(this->game->window.getSize().y);
    float scaleRatio = width / this->game->background.getTexture()->getSize().x;

    // Main menu buttons
    title.setPosition(width - (2200.f * scaleRatio), height - (1500.f * scaleRatio));
    startgame.changePosition(width - (2200.f * scaleRatio), height - (1200.f * scaleRatio));
    loadButton.changePosition(width - (2200.f * scaleRatio), height - (1100.f * scaleRatio));
    settings.changePosition(width - (2200.f * scaleRatio), height - (1000.f * scaleRatio));
    endgame.changePosition(width - (2200.f * scaleRatio), height - (900.f * scaleRatio));

    // Slot menu buttons (same positions as main menu buttons)
    slot1.changePosition(width - (2200.f * scaleRatio), height - (1200.f * scaleRatio));
    slot2.changePosition(width - (2200.f * scaleRatio), height - (1100.f * scaleRatio));
    slot3.changePosition(width - (2200.f * scaleRatio), height - (1000.f * scaleRatio));
    backButton.changePosition(width - (2200.f * scaleRatio), height - (900.f * scaleRatio));    

    //Set up main menu music
    if (!mainTheme.openFromFile("./assets/music/title.mp3")) std::cout << "NO TITLE MUSIC FOUND" << std::endl;
    else {
        mainTheme.setLoop(true);
        mainTheme.play();
    }

}


void GameStateStart::loadgame()
{
    this->game->requestChange(std::make_unique<GameStateEditor>(this->game,requestStartGame, this->game->floorNumber));
    return;    
}
//std::unique_ptr<GameState> state
