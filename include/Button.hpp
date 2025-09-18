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
    bool isHovered(sf::RenderWindow& window);
    sf::RectangleShape underline;
    const sf::RectangleShape& getUnderline() const;


    private:
   
    sf::Text text;
    Game* game;
};
