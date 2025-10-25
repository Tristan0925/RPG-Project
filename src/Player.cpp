/*

Implements the Player class logic.
Handles keyboard input and updates the player's position in the world.

*/

#include "Player.hpp"
#include "Map.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;



// normalize angle helper
 float normalizeAngle(float a) {
    while (a < 0) a += 2 * M_PI;
    while (a >= 2 * M_PI) a -= 2 * M_PI;
    return a;
}

Player::Player() {
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
    // if hitting door, load door gamestate 
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

PlayerData Player::getData() const {
    PlayerData data;
    data.position = position;
    data.angle = angle;
    data.HP = HP;
    data.maxHP = maxHP;
    data.MP = MP;
    data.maxMP = maxMP;
    data.STR = STR;
    data.VIT = VIT;
    data.AGI = AGI;
    data.LU = LU;
    data.XP = XP;
    data.LVL = LVL;
    data.MONEY = MONEY;
    data.inventory = inventory;
    data.affinities = affinities;

    for (int i = 0; i < 7; ++i)
        data.skills[i] = skills[i];

    return data;
}

void Player::setData(const PlayerData& data) {
    position = data.position;
    angle = data.angle;
    HP = data.HP;
    maxHP = data.maxHP;
    MP = data.MP;
    maxMP = data.maxMP;
    STR = data.STR;
    VIT = data.VIT;
    AGI = data.AGI;
    LU = data.LU;
    XP = data.XP;
    LVL = data.LVL;
    MONEY = data.MONEY;
    inventory = data.inventory;
    affinities = data.affinities;

    for (int i = 0; i < 7; ++i)
        skills[i] = data.skills[i];
}

bool Player::saveToFile(const std::string& filename) const {
    PlayerData data = getData();

    json j;
    j["position"] = { data.position.x, data.position.y };
    j["angle"] = data.angle;
    j["HP"] = data.HP;
    j["maxHP"] = data.maxHP;
    j["MP"] = data.MP;
    j["maxMP"] = data.maxMP;
    j["STR"] = data.STR;
    j["VIT"] = data.VIT;
    j["AGI"] = data.AGI;
    j["LU"] = data.LU;
    j["XP"] = data.XP;
    j["LVL"] = data.LVL;
    j["MONEY"] = data.MONEY;
    j["inventory"] = data.inventory;
    j["affinities"] = data.affinities;

    // skills array
    for (int i = 0; i < 7; ++i) {
        j["skills"].push_back(data.skills[i]);
    }

    std::ofstream file(filename);
    if (!file.is_open()) return false;
    file << j.dump(4); // pretty-print with indent of 4
    return true;
}

bool Player::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    json j;
    file >> j;

    PlayerData data;
    data.position = sf::Vector2f(j["position"][0], j["position"][1]);
    data.angle = j["angle"];
    data.HP = j["HP"];
    data.maxHP = j["maxHP"];
    data.MP = j["MP"];
    data.maxMP = j["maxMP"];
    data.STR = j["STR"];
    data.VIT = j["VIT"];
    data.AGI = j["AGI"];
    data.LU = j["LU"];
    data.XP = j["XP"];
    data.LVL = j["LVL"];
    data.MONEY = j["MONEY"];
    data.inventory = j["inventory"].get<std::map<std::string, int>>();
    data.affinities = j["affinities"].get<std::map<std::string, int>>();
    
    for (int i = 0; i < 7; ++i) {
        data.skills[i] = j["skills"][i];
    }

    setData(data);
    return true;
}
