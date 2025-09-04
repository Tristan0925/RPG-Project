#include <SFML/Graphics.hpp> 

int main() {
    // Creating a window
    sf::RenderWindow window(sf::VideoMode(800, 600), "RPG Starter");

    // Main game loop, runs until the window is closed
    while (window.isOpen()) {
        sf::Event event;

        // Poll and handle events (like closing the window)
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close(); // Close the window 
        }

        // Clear the window with black color
        window.clear(sf::Color::Black);

        // Display the contents of the window
        window.display();
    }

    return 0; // Exit the program
}
