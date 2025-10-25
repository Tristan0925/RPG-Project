//This represents the main menu screen. game_state_start.cpp

#include <SFML/Graphics.hpp>
#include "Button.hpp"
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
    startgame.draw(this->game->window);
    settings.draw(this->game->window);
    endgame.draw(this->game->window);
    
    if(startgame.isHovered(this->game->window)){
        this->game->window.draw(startgame.getUnderline());
    }

    if(settings.isHovered(this->game->window)){
        this->game->window.draw(settings.getUnderline());
    }

    if(endgame.isHovered(this->game->window)){
        this->game->window.draw(endgame.getUnderline());
    }

    return;
}

void GameStateStart::update(const float dt)
{

}

void GameStateStart::handleInput()
{
    sf::Event event;
    while(this->game->window.pollEvent(event))
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
                if (startgame.wasClicked(this->game->window)){
                    this->loadgame();
                }
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
                if (settings.wasClicked(this->game->window)){
                    std::cout << "Whenever we have settings to change, put it here." << std::endl;
                }
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){ //for some reason, putting elseifs causes buttons to not work
                if (endgame.wasClicked(this->game->window)){
                    this->game->window.close();
                }
            }
        }

        switch(event.type)
        {
            case sf::Event::Closed:
            {
                game->window.close();
                break;
            }

            case sf::Event::Resized:
            {
                this->view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height)); //resize window to new window size
                this->view.setCenter(this->view.getSize() / 2.f); //center view 
                this->game->window.setView(this->view); //updates view
                this->game->background.setPosition(0.f, 0.f); //set background img to the top left
                this->game->background.setScale(
                    float(event.size.width) / this->game->background.getTexture()->getSize().x,
                    float(event.size.height) / this->game->background.getTexture()->getSize().y
                ); //scale background to new window size

                // Keep original positions relative to window size
                float scaleRatio = (float(event.size.width) / this->game->background.getTexture()->getSize().x);

                // Reposition and rescale title the same way as the buttons
                title.setPosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1400.f * scaleRatio));
                startgame.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1200.f * scaleRatio));
                settings.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1100.f * scaleRatio));
                endgame.changePosition(event.size.width - (2200.f * scaleRatio), event.size.height - (1000.f * scaleRatio));

                break;
            }

            case sf::Event::KeyPressed:
            {
                if(event.key.code == sf::Keyboard::Escape) this->game->window.close();
                break;
            }

            default: break;
        }
    }
    return;
}

GameStateStart::GameStateStart(Game* game):
  startgame("Start Game", sf::Vector2f(0.f,0.f), 34, game),
  settings("Settings", sf::Vector2f(0.f,0.f), 34, game),
  endgame("End Game", sf::Vector2f(0.f,0.f), 34, game)
{
    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->view.setSize(pos);
    pos *= 0.5f;
    this->view.setCenter(pos);
    
    title.setFont(this->game->font);
    title.setString("UNTITLED RPG GAME");
    title.setCharacterSize(100); // in pixels
    title.setFillColor(sf::Color::Red);
    title.setStyle(sf::Text::Italic | sf::Text::Bold);
    title.setScale(0.5f, 0.5f); // fixed size

    // Position everything the same as on initial startup
    float width = static_cast<float>(this->game->window.getSize().x);
    float height = static_cast<float>(this->game->window.getSize().y);
    float scaleRatio = width / this->game->background.getTexture()->getSize().x;

    title.setPosition(width - (2200.f * scaleRatio), height - (1400.f * scaleRatio));
    startgame.changePosition(width - (2200.f * scaleRatio), height - (1200.f * scaleRatio));
    settings.changePosition(width - (2200.f * scaleRatio), height - (1100.f * scaleRatio));
    endgame.changePosition(width - (2200.f * scaleRatio), height - (1000.f * scaleRatio));
    
}

void GameStateStart::loadgame()
{
    this->game->pushState(std::make_unique<GameStateEditor>(this->game));
    return;
}
//std::unique_ptr<GameState> state
