//This represents the main menu screen.

#include <SFML/Graphics.hpp>
#include "Button.hpp"
#include "game_state_start.hpp"
#include "game_state_editor.hpp"
#include "game_state.hpp"
#include <iostream>

void GameStateStart::draw(const float dt)
{
    
    this->game->window.setView(this->view);
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
                game -> window.close();
                break;
            }
            case sf::Event::Resized:
            {
                this->view.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height)); //resize window to new window size
                this->view.setCenter(this->view.getSize() / 2.f); //center view 
                this->game->window.setView(this->view); //updates view
                this->game->background.setPosition(0.f, 0.f); //set background img to the top left
                this->game->background.setScale(float(event.size.width) / this->game->background.getTexture()->getSize().x, float(event.size.height) / this->game->background.getTexture()->getSize().y); //scale background to new window size
                title.setPosition(750.f, 400.f);
                startgame.changePosition(750.f, 500.f);
                settings.changePosition(750.f, 600.f);
                endgame.changePosition(750.f, 700.f);

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
  startgame("Start Game", sf::Vector2f(0.f,0.f), 48, game),
  settings("Settings", sf::Vector2f(0.f,0.f), 48, game),
  endgame("End Game", sf::Vector2f(0.f,0.f), 48, game)
   
{
    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->view.setSize(pos);
    pos *= 0.5f;
    this->view.setCenter(pos);
    
    title.setFont(this->game->font);
    title.setString("UNTITLED RPG GAME");
    title.setCharacterSize(72); // in pixels
    title.setFillColor(sf::Color::Red);
    title.setStyle(sf::Text::Italic | sf::Text::Bold);
   
    
 
   
   
}

void GameStateStart::loadgame()
{
    this->game->pushState(std::make_unique<GameStateEditor>(this->game));
    return;
}
//std::unique_ptr<GameState> state