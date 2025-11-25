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
#include "skill.hpp"
#include <unordered_map>
#include <algorithm>


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
    std::map<std::string, float> affinities;
    std::array<std::string, 9> skills; //save skills as strings instead of ptrs
};

class Player {
    private:
        sf::Vector2f position; // Player's position in the world
        float angle; // and the angle they are facing
        float targetAngle;  // snapped target
        float turnSpeed;    // turn speed
        void tryMove(sf::Vector2f delta, const Map& map); // checks for walls
        // Attack + every affinity + almighty. I think the battle_game_state should figure out damage #'s and stuff.
        std::array<Item, 2> inventory; // Only 2 items in game: Dragon Morsel (healing) and Energizing Moss (mana restoration) 
    protected:
        std::string name; //remember to add a change name function so when we start the game, we prompt to change the name
        int LVL, maxMP, maxHP, HP, MP, STR, VIT, MAG, AGI, LU, XP;
        std::map<std::string, float> affinities; //Fire, Ice, Phys, Elec, Force (Format: [ELEMENT] - [NULL(0)/RESIST(0.5)/NEUTRAL(1.0)/WEAK(1.5)]) If resist, x0.5 dmg, If weak, 1.5x dmg.
        std::array<const Skill*, 9> skillsList;  // Attack + every affinity + phys-almighty (ignores resistances, uses physical damage formula) + some extra skills. This probably should be its own class.
        protected:
        std::vector<Skill> skills;   // Stores actual Skill objects for players/NPCs
    public:
        int inDoor;
        Player(); // Constructor
        Player(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, const std::map<std::string, float>& affinities); //parameterized for NPCs as they inherit from player
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
        int physATK(float scalar, int baseAtk, bool isCrit); //essentially skills will have a super complicated formula based on the wiki
        int magATK(float scalar, int baseAtk, int limit, int correction, bool isWeak); 
        void statUp(int strength, int vitality, int magic, int agility, int luck);
        int doorX, doorY; //track last door entered
        void levelUp();
        void turnLeft();
        void turnRight();
        void update(float dt);
        PlayerData getData() const;       // Create a snapshot of player state
        void setData(const PlayerData&, const std::vector<Skill>& masterList, bool preserveLevel = true);  // Restore player state from snapshot
        bool saveToFile(const std::string& filename) const;
        bool loadFromFile(const std::string& filename, const std::vector<Skill>& masterList);
        void setDefault(const Map& map);
        bool isDead() const { return HP <= 0; }


        int getXpForNextLevel();
        int getHP() const, getmaxHP() const, getMP() const, getmaxMP() const, getLVL() const, getAGI() const, getSTR() const, getVIT() const, getXp() const, getMAG() const, getLU() const;
        std::string getName() const;
        std::array<Item, 2> getInventory() const;
        std::array<const Skill*, 9> getSkillsList() const;
        void addToInventory(Item item, int quantity);
        const Skill* getSkillPtr(std::string skillName, const std::vector<Skill>& masterList);
        void addToSkillList(std::string skillName, const std::vector<Skill>& masterList);
        std::vector<std::string> getSkillNames() const {
            std::vector<std::string> names;
            for (const Skill* skillPtr : skillsList) {
                if (skillPtr != nullptr)
                    names.push_back(skillPtr->getName());
            }
            return names;
        }
        float getAffinity(const std::string& element) const {
            auto it = affinities.find(element);
            if (it != affinities.end())
                return it->second;
            return 1.0f; // default = neutral
        }
        // Returns map<string, float> of affinities like Fire=0.5, Ice=1.5, etc.
        const std::map<std::string, float>& getAffinityMap() const {
            return affinities;
        }

        // Buff/debuff declarations
        struct ActiveBuff {
            std::string name; // e.g. "Damage Amp", "Damage Resist", "Hit Boost", "Hit Reduction"
            float value; // multipler (e.g. 1.25f for +25%, 0.80f for -20%)
            int turnsRemaining; // number of actor turns remaining
            bool affectsOutgoing; // true if buff modifies outgoing damage/hit (actor-side)
            bool affectsIncoming; // true if buff modifies incoming damage (target-side)
        };

        std::vector<ActiveBuff> activeBuffs;

        // Buff API
        void addBuff(const std::string& name, float value, int turns, bool affectsOutgoing = true, bool affectsIncoming = false);
        void decrementBuffTurns(); // decrement and remove expired buffs (called when actor finishes turn)
        void removeExpiredBuffs();

        float getOutgoingDamageMultiplier() const; // multiplies outgoing damage (actor)
        float getIncomingDamageMultiplier() const; // multiplies incoming damage (target)
        float getHitModifier() const; // multiplies hit chance/evasion as needed

};

