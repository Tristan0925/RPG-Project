#include "Button.hpp"
#include "game.hpp"
#include "iostream"
#include <cmath>
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

    // Hexagon background
    hex.setPointCount(6);
    float radiusX = 70.f;  // horizontal radius
    float radiusY = 20.f;  // vertical radius
    for (int i = 0; i < 6; ++i) {
        float angle = 3.14159f / 3 * i;
        hex.setPoint(i, sf::Vector2f(std::cos(angle) * radiusX, std::sin(angle) * radiusY));
    }
    hex.setOrigin(hex.getLocalBounds().width / 2, hex.getLocalBounds().height / 2);
    hex.setFillColor(sf::Color(100, 0, 0, 150));

    useHexBackground = false; 
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

void Button::setHighlight(bool highlight) {
    isHighlighted = highlight;

    // Only affect hex visuals if enabled
    if (useHexBackground) {
        if (highlight)
            hex.setFillColor(sf::Color(144, 238, 144, 120));
        else
            hex.setFillColor(sf::Color(100, 0, 0, 150));
    }
}


void Button::changePosition(float posx, float posy){
    text.setPosition(posx, posy);
    underline.setPosition(posx, (text.getGlobalBounds().top + text.getGlobalBounds().height + 2.f));
}

void Button::draw(sf::RenderWindow& window){
    if (!visible)
        return;

    if (useHexBackground) {
        sf::FloatRect textBounds = text.getGlobalBounds();
        hex.setPosition(
            (textBounds.left + 68.f) + textBounds.width / 2.f,
            (textBounds.top + 20.f) + textBounds.height / 2.f
        );

        if (isHighlighted)
            hex.setFillColor(sf::Color(144, 238, 144, 120));
        else
            hex.setFillColor(sf::Color(100, 0, 0, 150));

        window.draw(hex);
    }

    window.draw(text);
    if (isHighlighted) {
        underline.setPosition(
            text.getPosition().x,
            text.getPosition().y + text.getGlobalBounds().height + 4.f
        );
    window.draw(underline);
    }
}
