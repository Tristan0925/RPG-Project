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
    // Save the current view
    sf::View oldView = window.getView();

    // Switch to default (GUI) view so clicks line up with screen-space
    window.setView(window.getDefaultView());

    // Get mouse position in screen coordinates
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    sf::Vector2f mouseScreen = window.mapPixelToCoords(mousePixel);

    // Restore the old view
    window.setView(oldView);

    // Check if the mouse is inside the text bounds
    sf::FloatRect bounds = text.getGlobalBounds();
    return bounds.contains(mouseScreen);
}

bool Button::isHovered(sf::RenderWindow& window) {
    sf::View oldView = window.getView();
    window.setView(window.getDefaultView());

    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    sf::Vector2f mouseScreen = window.mapPixelToCoords(mousePixel);

    window.setView(oldView);

    sf::FloatRect bounds = text.getGlobalBounds();
    return bounds.contains(mouseScreen);
}



void Button::changePosition(float posx, float posy){
    text.setPosition(posx, posy);
    underline.setPosition(posx, (text.getGlobalBounds().top + text.getGlobalBounds().height + 2.f));
}

void Button::draw(sf::RenderWindow& window){
    window.draw(text);
}
