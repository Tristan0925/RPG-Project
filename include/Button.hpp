#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class Game;

class Button{
    public:
    Button(std::string words, sf::Vector2f location,int size, Game* game, sf::Color color = sf::Color::Red  );
    bool wasClicked(sf::RenderWindow& window);
    void changePosition(float posx, float posy);
    void draw(sf::RenderWindow& window);
    private:
    bool isHovered(sf::RenderWindow& window);
    sf::Text text;
    Game* game;
};
