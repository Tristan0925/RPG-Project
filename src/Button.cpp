#include "Button.hpp"
#include "game.hpp"
#include "iostream"
//later include a thing for audio params
Button::Button(std::string words, sf::Vector2f location, int size, Game* game, sf::Color color): game(game) {

    text.setFont(this->game->font);
    text.setString(words);
    text.setPosition(location);
    text.setCharacterSize(size);
    text.setFillColor(color);
    underline.setSize(sf::Vector2f(text.getGlobalBounds().width, 2.f));
    underline.setFillColor(color);
    underline.setPosition(location.x, (location.y + 3.f));
}

const sf::RectangleShape& Button::getUnderline() const{
    return underline;
}

bool Button::wasClicked(sf::RenderWindow& window) {
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel); 

    sf::FloatRect bounds = text.getGlobalBounds();
    return bounds.contains(mouseWorld);
}

bool Button::isHovered(sf::RenderWindow& window) {
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window); //i know these do the same thing, i am just using them for different purposes for the sake of clarity
    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel); 

    sf::FloatRect bounds = text.getGlobalBounds();
    return bounds.contains(mouseWorld);
}


void Button::changePosition(float posx, float posy){
    text.setPosition(posx, posy);
    underline.setPosition(posx, (text.getGlobalBounds().top + text.getGlobalBounds().height + 2.f));
}

void Button::draw(sf::RenderWindow& window){
    window.draw(text);
}
