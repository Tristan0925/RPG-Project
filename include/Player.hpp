/*

Defines the Player class for tracking position and movement.

-- This verison is used to simulate first-person navigation by updating camera position based on input.

A .hpp file is a header file that contains declarations; not full implementations. You use it to declare things like: classes, function prototypes, and constants. 

Like having modules in python to handle classes and then you import the files you want into the main file or whatever to use.
*/


#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "item.hpp"
#include <array>

class Map;

class Player {
    private:
        sf::Vector2f position; // Player's position in the world
        float angle; // and the angle they are facing
        float targetAngle;  // snapped target
        float turnSpeed;    // turn speed
        void tryMove(sf::Vector2f delta, const Map& map); // checks for walls
        
        std::array<Item, 2> inventory; // Only 2 items in game: Dragon Morsel (healing) and Energizing Moss (mana restoration) 
    protected:
        std::string name; //remember to add a change name function so when we start the game, we prompt to change the name

        int LVL, maxMP, maxHP, HP, MP, STR, VIT, MAG, AGI, LU, XP;
        std::map<std::string, float> affinities; //Fire, Ice, Phys, Elec, Force (Format: [ELEMENT] - [NULL(0)/RESIST(0.5)/NEUTRAL(1.0)/WEAK(1.5)]) If resist, x0.5 dmg, If weak, 1.5x dmg.
        std::string skills[7];  // Attack + every affinity + almighty. I think the battle_game_state should figure out damage #'s and stuff. This probably should be its own class.
    public:
        int inDoor;
        Player(); // Constructor
        Player(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities); //parameterized for NPCs as they inherit from player
        Player(const sf::Vector2f& spawnPos);
        void move(sf::Vector2f delta); // Handle input and updates the position
        sf::Vector2f getPosition() const; // returns current position
        void setPosition(const sf::Vector2f& pos);

        
        float getAngle() const;

        void moveForward(float distance, const Map& map); // moves forward, checks collisions
        void moveBackward(float distance, const Map& map); // same thing, but backwards
        void takeDamage(int damage);
        void heal(int healAmount);
        void spendMP(int mpSpent);
        void regainMP(int mpGained);
        int physATK(float scalar, int baseAtk); //essentially skills will use either physATK/magATK then multi by a scalar, return the dmg number
        int magATK(float scalar, int baseAtk); 
        void levelUp(std::map<std::string, int> skillPointDistribution);
        int doorX, doorY; //track last door entered

        void turnLeft();
        void turnRight();
        void update(float dt);

        int getHP() const, getmaxHP() const, getMP() const, getmaxMP() const, getMoney() const, getLVL() const;
        std::array<Item, 2> getInventory() const;
        void addToInventory(Item item, int quantity);
    
        
};
