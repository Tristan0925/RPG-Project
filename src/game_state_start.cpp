//This represents the main menu screen.

#include <SFML/Graphics.hpp>

#include "game_state_start.hpp"
#include "game_state_editor.hpp"
#include "game_state.hpp"

void GameStateStart::draw(const float dt)
{
    this->game->window.setView(this->view);
    this->game->window.clear(sf::Color::Black);
    this->game->window.draw(this->game->background);
    sf::Text title;
    title.setFont(this->game->font);
    title.setString("UNTITLED RPG PROJECT");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::Red);
    title.setPosition(100.f, 50.f);
    this->game->window.draw(title);
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
                
                
                break;
            }
            case sf::Event::KeyPressed:
            {
                if(event.key.code == sf::Keyboard::Escape) this->game->window.close();
                else if(event.key.code == sf::Keyboard::Space) this->loadgame();
                 break;
            }
            
            default: break;
        }
    }
    return;
}

GameStateStart::GameStateStart(Game* game)
{
    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
    this->view.setSize(pos);
    pos *= 0.5f;
    this->view.setCenter(pos);
}

void GameStateStart::loadgame()
{
    this->game->pushState(std::make_unique<GameStateEditor>(this->game));
    return;
}
//std::unique_ptr<GameState> state