/*

Implements the Player class logic.
Handles keyboard input and updates the player's position in the world.

*/

#include "Player.hpp"
#include "Map.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include "game_state_door.hpp"
#include <iostream>
#include <array>



// normalize angle helper
 float normalizeAngle(float a) {
    while (a < 0) a += 2 * M_PI;
    while (a >= 2 * M_PI) a -= 2 * M_PI;
    return a;
}

Player::Player() { //default constructor
    name = "Tatsuya";
    HP = 100;
    maxHP = 100;
    MP = 100;
    maxMP = 100;
    LVL = 1;
    XP = 0;
    position = sf::Vector2f(0.f, 0.f);
    angle = 0.f;
    targetAngle = 0.f;
    turnSpeed = 3.0f; // radians/sec, tweak to taste
}

Player::Player(std::string name, int HP, int maxHP, int MP, int maxMP, int STR, int VIT, int MAG, int AGI, int LU, int XP, int LVL) : 
    name(name), HP(HP), maxHP(maxHP), MP(MP), maxMP(maxMP), STR(STR), VIT(VIT), MAG(MAG), AGI(AGI), LU(LU), XP(XP), LVL(LVL){} //parametized constructor (mainly used for NPCs)

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
 int Player::physATK(float scalar, int baseAtk){ //base atk + scalar * STR 
    return (int) (baseAtk + (scalar * STR));
 }
 int Player::magATK(float scalar, int baseAtk){
    return (int) (baseAtk + (scalar * MAG));

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
 }


