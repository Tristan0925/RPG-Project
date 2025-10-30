/*

Defines the Player class for tracking position and movement.

-- This version is used to simulate first-person navigation by updating camera position based on input.

A .hpp file is a header file that contains declarations; not full implementations. You use it to declare things like: classes, function prototypes, and constants. 

Like having modules in python to handle classes and then you import the files you want into the main file or whatever to use.
*/

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <array>
#include <map>
#include "item.hpp"
#include <array>
class Map;

// Struct for saving and loading player state.
// This holds all player data that we might write to a file.
struct PlayerData {
    sf::Vector2f position;
    float angle;
    int HP;
    int maxHP;
    int MP;
    int maxMP;
    int STR;
    int VIT;
    int AGI;
    int LU;
    int XP;
    int LVL;
    std::array<Item, 2> inventory;
    std::map<std::string, int> affinities;
    std::array<std::string, 7> skills;
};

class Player {
    private:
        sf::Vector2f position; // Player's position in the world
        float angle; // and the angle they are facing
        float targetAngle;  // snapped target
        float turnSpeed;    // turn speed
        void tryMove(sf::Vector2f delta, const Map& map); // checks for walls
        sf::Vector2f postion;
        // Attack + every affinity + almighty. I think the battle_game_state should figure out damage #'s and stuff.
        std::array<Item, 2> inventory; // Only 2 items in game: Dragon Morsel (healing) and Energizing Moss (mana restoration) 
    protected:
        std::string name; //remember to add a change name function so when we start the game, we prompt to change the name
        int HP, maxHP, MP, maxMP, STR, VIT, AGI, LU, XP, LVL;
        std::map<std::string, int> affinities; //Fire, Ice, Phys, Elec, Force (Format: [ELEMENT] - [RESIST(0.5)/NEUTRAL(1.0)/WEAK(1.5)]) If resist, x0.5 dmg, If weak, 1.5x dmg.
        std::string skills[7];
    public:
        int inDoor;
        Player(); // Constructor
        Player(std::string name, int HP, int maxHP, int MP, int maxMP, int STR, int VIT, int AGI, int LU, int XP, int LVL); //parameterized for NPCs as they inherit from player
        Player(const sf::Vector2f& spawnPos);

        void move(sf::Vector2f delta); // Handle input and updates the position
        sf::Vector2f getPosition() const; // returns current position
        void setPosition(const sf::Vector2f& pos);

        float getAngle() const;

        void moveForward(float distance, const Map& map); // moves forward, checks collisions
        void moveBackward(float distance, const Map& map); // same thing, but backwards
        void takeDamage();
        void Heal();
        void spendMP();
        void regainMP();
        int physATK(); //essentially skills will use either physATK/magATK then multi by a scalar, return the dmg number
        int magATK();
        int doorX, doorY; //track last door entered

        void turnLeft();
        void turnRight();
        void update(float dt);
        PlayerData getData() const;       // Create a snapshot of player state
        void setData(const PlayerData&);  // Restore player state from snapshot
        bool saveToFile(const std::string& filename) const;
        bool loadFromFile(const std::string& filename);
        void setDefault(const Map& map);


        int getHP() const, getmaxHP() const, getMP() const, getmaxMP() const, getLVL() const;
        std::array<Item, 2> getInventory() const;
        void addToInventory(Item item, int quantity);
    
        
};

