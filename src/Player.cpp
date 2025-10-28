/*

Implements the Player class logic.
Handles keyboard input and updates the player's position in the world.

*/

#include "Player.hpp"
#include "Map.hpp"
#include "skill.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include "game_state_door.hpp"
#include <iostream>
#include <array>
#include <map>
#include <random>
#include <vector>



// normalize angle helper
 float normalizeAngle(float a) {
    while (a < 0) a += 2 * M_PI;
    while (a >= 2 * M_PI) a -= 2 * M_PI;
    return a;
}

Player::Player() { //default constructor
    name = "Tatsuya";
    LVL = 1;
    STR = 4;
    VIT = 3;
    MAG = 3;
    AGI = 3;
    LU = 2;
    maxHP = (LVL + VIT) * 6;
    maxMP = (LVL + MAG) * 3;
    HP = maxHP;
    MP = maxMP;
    affinities = {{"Fire", 0.0}, {"Ice", 1.5}, {"Physical", 1.0}, {"Force", 1.0}, {"Electric", 1.0}};
    XP = 0;
    skillsList = { nullptr };
    position = sf::Vector2f(0.f, 0.f);
    angle = 0.f;
    targetAngle = 0.f;
    turnSpeed = 3.0f; // radians/sec, tweak to taste
}
        // Skill("Attack", "Basic attack made with your fists.", "Physical", 1, true, 32, 0.0f, 100.0f, 3.0f, 13.0f), //basic attack
        // Skill("Freikugel","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Physical-Almighty", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), //phys-almighty skill
        // Skill("Divine Shot", "High physical damage to one foe. Medium crit rate.", "Physical", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), //physical skill
        // Skill("Fire Breath", "Low fire damage to all foes.", "Fire", 4, false, 30, 7, 100.0f, 0.0f, 85, 8), //fire skill
        // Skill("Tornado", "High force damage to all foes.", "Force", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), //force skill
        // Skill("Flash Freeze", "Medium ice damage to one foe.", "Ice", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  //ice skill
        // Skill("Focus", "Multiplies the damage of next physical attack by 2.5x.","Damage Amp", 10, true, 0.0f, 2.5f, 0.0f, 0.0f), // self buff skill
        // Skill("Dia", "Moderately restores one ally's health.", "Healing", 2, true, 7, 15.0f), //heal one skill
        // Skill("Shock", "Low electric damage to one foe.", "Electric", 1, true, 25, 5, 100.0f, 0.0f, 70, 7) //elec skill
Player::Player(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities, std::array<Skill, 9> skillsList) : 
    name(name), LVL(LVL), maxMP((LVL + MAG) * 3), maxHP((LVL + VIT) * 6), HP(maxHP), MP(maxMP), STR(STR), VIT(VIT), MAG(MAG), AGI(AGI), LU(LU), XP(XP), affinities(affinities), skillsList(skillsList){} //parametized constructor (mainly used for NPCs)

Player::Player(const sf::Vector2f& spawnPos) {
    position = spawnPos * 64.0f; // Start the player in the center of the screen
    angle = 0.f; // player facing right, (π/2 = down, π = left, 3π/2)
    targetAngle = 0.f;
    turnSpeed = 3.0f;
}

void Player::move(sf::Vector2f delta) {
    position += delta; // adding delta directly (delta signifies movement)
}

sf::Vector2f Player::getPosition() const {
    return position; // returns current position so stuff like the minimap can draw the player
}

void Player::setPosition(const sf::Vector2f & pos)
{
    position = pos;
}

void Player::turnLeft() {
    targetAngle -= M_PI / 2.f;
    targetAngle = normalizeAngle(targetAngle);
}

void Player::turnRight() {
    targetAngle += M_PI / 2.f;
    targetAngle = normalizeAngle(targetAngle);
}


void Player::update(float dt) {
    // Smoothly rotate toward targetAngle
    float delta = targetAngle - angle;

    // shortest path wrapping
    if (delta > M_PI) delta -= 2 * M_PI;
    if (delta < -M_PI) delta += 2 * M_PI;

    float step = turnSpeed * dt;
    if (std::fabs(delta) <= step) {
        angle = targetAngle;
    } else {
        angle += (delta > 0 ? step : -step);
    }

    angle = normalizeAngle(angle);
}


float Player::getAngle() const {
    return angle; // for recognition in minimap
}


// Player collision detection
void Player::tryMove(sf::Vector2f delta, const Map& map) {
    
    sf::Vector2f newPos = position + delta; // predict new position

    // Convert to grid coordinates (assuming each tile = 64px)
    int gridX = static_cast<int>(newPos.x / 64);
    int gridY = static_cast<int>(newPos.y / 64);

    // only move if not hitting wall
    if (!map.isWall(gridX, gridY)) {
        position = newPos;
    }
    if (map.isDoor(gridX,gridY) && !inDoor){
        inDoor = 1;
        doorX = gridX;
        doorY = gridY;
        position = position - delta;
        std::cout <<"door position: (" << gridX << ", " << gridY << ")" << std::endl;
        std::cout <<"door position: (" << doorX << ", " << doorY << ")" << std::endl;
    }
}



// Uses the facing direction vector (cos(angle) - x direction, sin(angle) - y direction), both go through tryMove, so they respect collisions.
void Player::moveForward(float distance, const Map& map) { // Forward = add direction × distance.
    sf::Vector2f dir(std::cos(angle), std::sin(angle));
    tryMove(dir * distance, map);
}

void Player::moveBackward(float distance, const Map& map) {
    sf::Vector2f dir(std::cos(angle), std::sin(angle)); // Backward = subtract direction × distance.
    tryMove(-dir * distance, map);
}

void Player::addToInventory(Item item, int quantity){
    int firstemptyslot = -1;
    for (size_t i = 0; i < inventory.size(); i++){ 
        if (inventory[i].showName() == item.showName()){ 
            inventory[i].addToQuantity(quantity);
            return;
        } 
        if (inventory[i].showName() == "Empty Slot" && firstemptyslot == -1){
          firstemptyslot = i;
        }
    }
    if (firstemptyslot != -1){ 
    inventory[firstemptyslot] = item;
    inventory[firstemptyslot].addToQuantity(quantity);
    }
    else std::cout << "Error: item not found." << std::endl;
}

std::array<Item, 2> Player::getInventory() const{
    return inventory;
}

int Player::getHP() const {
    return HP;
}

int Player::getMP() const {
    return MP;
}

int Player::getmaxHP() const {
    return maxHP;
}

int Player::getmaxMP() const {
    return maxMP;
}
 void Player::takeDamage(int damage){
    if (HP - damage < 0) HP = 0;
    else HP -= damage;
 }
 void Player::heal(int healAmount){
    if (HP + healAmount > maxHP) HP = maxHP;
    else HP += healAmount;
 }
 void Player::spendMP(int mpSpent){
    if (MP - mpSpent < 0) MP = 0;
    else MP -= mpSpent; 
 }
 void Player::regainMP(int mpGained){
    if (MP + mpGained > maxMP) MP = maxMP;
    else MP += mpGained;
 }

 //https://megamitensei.fandom.com/wiki/Damage#Physical_Attack 
 //Also tells you how magic attacks work

 int Player::physATK(float scalar, int baseAtk, bool isCrit){ //No one can be weak to phys, hopefully somehow balances out the weakness of magic attacks over time
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> damageDifferential(-5,5); //damage has a +/- 5% added to it, to keep damage from being deterministic
    int damageDifference = damageDifferential(gen);
    int damage = ((LVL + STR) * baseAtk / 15);
    if (isCrit) return (int) (1.5 * (damage + (damage * (damageDifference))));
    else return (damage + (damage * (damageDifference)));
 }

 int Player::magATK(float scalar, int baseAtk, int limit, int correction, bool isWeak){ //super complicated formulas which essentially says magic gets weaker over time (past lvl 30 is where it starts to fall off)
    int peak = ((limit - correction) / baseAtk) * (255/24);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> damageDifferential(-5,5); //damage has a +/- 5% added to it, to keep damage from being deterministic
    int damageDifference = damageDifferential(gen);
    int damage;
    float weaknessMultiplier;
    if (isWeak) weaknessMultiplier = 1.5;
    else weaknessMultiplier = 1.0;

    if (LVL < peak){
        damage = 0.004 * (5 * (MAG + 36) - LVL) * ((24 * baseAtk * (LVL / 255) + correction));
        return (int) (damage + (damage * (damageDifference))) * weaknessMultiplier;
    }
    else if (LVL == peak){
        damage = 0.004 * ((5 * (MAG + 36)) - ((limit - correction) / baseAtk) * (255/24)) * limit;
        return (int) (damage + (damage * (damageDifference))) * weaknessMultiplier;
    }
    else if (LVL > peak && LVL <= 160){
        damage = 0.004 * (5 * (MAG + 36) - LVL) * limit;
        return (int) (damage + (damage * (damageDifference))) * weaknessMultiplier;
    }
    else if (LVL > 160){
        damage = 0.004 * (5 * (MAG + 36) - 160) * limit;
        return (int) (damage + (damage * (damageDifference))) * weaknessMultiplier;
    }
    else {
     std::cout << "Something went horribly wrong while attacking if you are seeing this" << std::endl;
     std::exit(100);
    }
 }
 
 void Player::levelUp(std::map<std::string, int> skillPointDistribution){
    for (auto distribution : skillPointDistribution){
        std::string trait = distribution.first;
        int skillPoints = distribution.second;
        if (trait == "STR"){
            STR += skillPoints;
        }
        else if (trait == "VIT"){
            VIT += skillPoints;
        }
        else if (trait == "AGI"){
            AGI += skillPoints;
        }
        else if (trait == "LU"){
            LU += skillPoints;
        }
        else if (trait == "MAG"){
            MAG += skillPoints;
        }
    }
    maxHP = (LVL + VIT) * 6;
    maxMP = (LVL + MAG) * 3;
    HP = maxHP;
    MP = maxMP;
 }

 void Player::addToSkillList(const Skill& skill){
    for (auto& skillSlot : skillsList){
        if (s == nullptr){
            s = &skill;
            return;
        }
    }
    std::cout << "IM FULL STOP IT" << std::endl;
 }


