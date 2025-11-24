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
    const sf::RectangleShape& getUnderline() const;

    void setHighlight(bool on); 
    void setVisible(bool visible);
    void enableHexBackground(bool enable ) {useHexBackground = enable;}
    std::string getText() const { return text.getString(); }


    private:

    Game* game;
    sf::Text text;
    sf::ConvexShape hex;
    sf::RectangleShape underline;

    bool useHexBackground = false;
    bool isHighlighted = false;
    bool visible = true;
};
