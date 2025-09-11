/*

Defines the Player class for tracking position and movement.

-- This verison is used to simulate first-person navigation by updating camera position based on input.

A .hpp file is a header file that contains declarations; not full implementations. You use it to declare things like: classes, function prototypes, and constants. 

Like having modules in python to handle classes and then you import the files you want into the main file or whatever to use.
*/


#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Map;

class Player {
    private:
        sf::Vector2f position; // Player's position in the world
        float angle; // and the angle they are facing

    public:
        Player(); // Constructor

        void move(sf::Vector2f delta); // Handle input and updates the position
        sf::Vector2f getPosition() const; // returns current position

        void rotate(float radians); // Handles turning left and right
        float getAngle() const;

        void moveForward(float distance, const Map& map); // moves forward, checks collisions
        void moveBackward(float distance, const Map& map); // same thing, but backwards

        
    private:
        void tryMove(sf::Vector2f delta, const Map& map); // checks for walls

};
