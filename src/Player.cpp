/*

Implements the Player class logic.
Handles keyboard input and updates the player's position in the world.

*/

#include "Player.hpp"
#include "Map.hpp"
#include "skill.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
using json = nlohmann::json;
#include "game_state_door.hpp"
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

Player::Player(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, const std::map<std::string, float>& affinities) : 
    name(std::move(name)), LVL(LVL), maxMP((LVL + MAG) * 3), maxHP((LVL + VIT) * 6), HP(maxHP), MP(maxMP), STR(STR), VIT(VIT), MAG(MAG), AGI(AGI), LU(LU), XP(XP), affinities(affinities){} //parametized constructor (mainly used for NPCs)

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

void Player::setDefault(const Map& map)
{
    position = sf::Vector2f(map.getSpawnX() * 64.f, map.getSpawnY() * 64.f);; // starting coordinates
    angle = 0.f; // facing forward
    // reset inventory, health, etc.
    maxHP = (LVL + VIT) * 6;
    HP = maxHP;
    maxMP = (LVL + MAG) * 3;
    MP = maxMP;
    LVL = 1;
    XP = 50;
    name = "Tatsuya";
    STR = 4;
    VIT = 3;
    MAG = 3;
    AGI = 3;
    LU = 2;
    affinities = {{"Fire", 0.0}, {"Ice", 1.5}, {"Physical", 1.0}, {"Force", 1.0}, {"Electric", 1.0}};
    skillsList = { nullptr };
    angle = 0.f;
    targetAngle = 0.f;
    turnSpeed = 3.0f; // radians/sec, tweak to taste
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
int Player::getAGI() const {
    return AGI;
}

int Player::getLVL() const {
    return LVL;
}


int Player::getXp() const{
    return XP;
}

int Player::getXpForNextLevel(){
    if (LVL <= 10) return 100 + (LVL - 1) * 50;
    else if (LVL <= 25) return 550 + pow(LVL - 10, 2) * 15;
    else return 3000 + pow(LVL - 25, 2.3) * 25;
}

int Player::getLU() const{
    return LU;
}

int Player::getMAG() const{
    return MAG;
}

int Player::getSTR() const { return STR; }
int Player::getVIT() const { return VIT; }

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
    std::uniform_real_distribution<float> damageDifferential(-0.05f, 0.05f); //damage has a +/- 5% added to it, to keep damage from being deterministic
    float damageDifference = damageDifferential(gen);
    float raw = ((static_cast<float>(LVL) + static_cast<float>(STR)) * static_cast<float>(baseAtk) / 15.0f) * scalar;
    float varied = raw * (1.0f + damageDifference); // equivalent to raw + raw * damageDifference
    if (isCrit) varied *= 1.5f;

    // Buff multiplier
    float outgoingMult = getOutgoingDamageMultiplier();
    varied *= outgoingMult;

    int damage = static_cast<int>(std::round(varied));
    if (damage < 0) damage = 0;
    return damage;
 }

 int Player::magATK(float scalar, int baseAtk, int limit, int correction, bool isWeak){ //super complicated formulas which essentially says magic gets weaker over time (past lvl 30 is where it starts to fall off)
    int peak = ((limit - correction) / baseAtk) * (255/24);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> damageDifferential(-0.05f ,0.05f); //damage has a +/- 5% added to it, to keep damage from being deterministic
    int damageDifference = damageDifferential(gen);

    float weaknessMultiplier = isWeak ? 1.5f : 1.0f;;
    float dmg = 0.f;  // use float internally, convert to int later

    if (LVL < peak){
        dmg = 0.004f * (5 * (MAG + 36) - LVL) * ((24 * baseAtk * (LVL / 255.0f) + correction));
    }
    else if (LVL == peak){
        dmg = 0.004f * ((5 * (MAG + 36)) - ((limit - correction) / (float)baseAtk) * (255/24.0f)) * limit;
    }
    else if (LVL > peak && LVL <= 160){
        dmg = 0.004f * (5 * (MAG + 36) - LVL) * limit;
    }
    else if (LVL > 160){
        dmg = 0.004f * (5 * (MAG + 36) - 160) * limit;
    }
    else {
        std::cout << "Something went horribly wrong while attacking if you are seeing this" << std::endl;
        std::exit(100);
    }

    // Apply randomness
    dmg = dmg + dmg * damageDifference;

    // Apply weakness (from element multiplier logic outside)
    dmg *= weaknessMultiplier;

    // Outgoing damage buff multiplier
    float outgoingMult = getOutgoingDamageMultiplier();
    dmg *= outgoingMult;

    return (int) dmg;
 }
 
 void Player::statUp(int strength, int vitality, int magic, int agility, int luck){
    STR = strength;
    VIT = vitality;
    MAG = magic;
    AGI = agility;
    LU = luck;
    maxHP = (LVL + VIT) * 6;
    maxMP = (LVL + MAG) * 3;
    HP = maxHP;
    MP = maxMP;
 }

 const Skill* Player::getSkillPtr(std::string skillName, const std::vector<Skill>& masterList){
       for (const auto& skill : masterList){
        if (skill.getName() == skillName){
            return &skill;
        }
    }
    return nullptr;
}

 void Player::addToSkillList(std::string skillName, const std::vector<Skill>& masterList){
    const Skill* skillPtr = getSkillPtr(skillName, masterList);
    if(!skillPtr){
        std::cout << "NOT FOUND DUMMY" << std::endl;
        return;
    }

    for (auto& slot : skillsList){
        if (slot == nullptr){
            slot = skillPtr;
            std::cout <<"Added skill: " << skillPtr->getName() << "\n";
            return;
        }
    }
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
    data.inventory = inventory;
    data.affinities = affinities;

    for (int i = 0; i < 9; ++i){
        if (skillsList[i] != nullptr) data.skills[i] = skillsList[i]->getName(); //if not null, get name.
        else data.skills[i] = "EMPTY SLOT";
    }
    return data;
}

void Player::setData(const PlayerData& data, const std::vector<Skill>& masterList) {
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
    inventory = data.inventory;
    affinities = data.affinities;

    for (size_t i = 0; i < 9; ++i){
        if (data.skills[i] == "Empty Slot") {
        skillsList[i] = nullptr;
        }
        else{
            skillsList[i] = getSkillPtr(data.skills[i], masterList);
        } 
    }
        
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
    j["inventory"] = json::array();
    for (const auto& item : data.inventory) {
        j["inventory"].push_back({
            {"name", item.showName()},
            {"description", item.showDescription()},
            {"healAmount", item.getHealAmount()},
            {"manaAmount", item.getManaAmount()},
            {"quantity", item.getQuantity()}
        });
    }
    j["affinities"] = data.affinities;

    // skills array
    j["skills"] = data.skills;


    std::ofstream file(filename);
    if (!file.is_open()) return false;
    file << j.dump(4); // pretty-print with indent of 4
    return true;
}

bool Player::loadFromFile(const std::string& filename, const std::vector<Skill>& masterList) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Could not open " << filename << std::endl;
        return false;
    } 

    json j;
    file >> j;

    PlayerData data;
    if (j.contains("position") && j["position"].is_array() && j["position"].size() >= 2)
    data.position = sf::Vector2f(j["position"][0], j["position"][1]);
    else
        data.position = {0.f, 0.f};

    data.angle = j.value("angle", 0.f);
    data.HP = j.value("HP", 100);
    data.maxHP = j.value("maxHP", 100);
    data.MP = j.value("MP", 100);
    data.maxMP = j.value("maxMP", 100);
    data.STR = j.value("STR", 10);
    data.VIT = j.value("VIT", 10);
    data.AGI = j.value("AGI", 10);
    data.LU = j.value("LU", 10);
    data.XP = j.value("XP", 0);
    data.LVL = j.value("LVL", 1);
        
    if (j.contains("inventory") && j["inventory"].is_array()) {
        for (size_t i = 0; i < std::min(j["inventory"].size(), data.inventory.size()); ++i) {
            const auto& it = j["inventory"][i];
            std::string name = it.value("name", "Empty Slot");
            std::string desc = it.value("description", "");
            int heal = it.value("healAmount", 0);
            int mana = it.value("manaAmount", 0);
            int qty  = it.value("quantity", 0);
            data.inventory[i] = Item(name, desc, heal, mana, qty);
        }
    } else {
        for (auto& it : data.inventory)
            it = Item("Empty Slot", "", 0, 0, 0);
    }

    if (j.contains("affinities") && j["affinities"].is_object())
        data.affinities = j["affinities"].get<std::map<std::string,float>>();
    else
        data.affinities.clear();

    // Safe skills load (assumes skills are strings)
    if (j.contains("skills") && j["skills"].is_array() && j["skills"].size() == 9) {
        for (size_t i = 0; i < 9; ++i)
            data.skills[i] = j["skills"][i].get<std::string>();
    } else {
        for (size_t i = 0; i < 9; ++i)
            data.skills[i] = "";
    }


    setData(data,masterList);
    return true;
}
   std::array<const Skill*, 9> Player::getSkillsList() const{
    return skillsList;
   }
   
std::string Player::getName() const{
    return name;
}


void Player::levelUp(){
    LVL +=1;
}


// Buff/Debuff function locations

void Player::addBuff(const std::string& name, float value, int turns,
                     bool affectsOutgoing, bool affectsIncoming) {
    ActiveBuff b;
    b.name = name;
    b.value = value;
    b.turnsRemaining = turns;
    b.affectsOutgoing = affectsOutgoing;
    b.affectsIncoming = affectsIncoming;
    activeBuffs.push_back(b);
}

void Player::decrementBuffTurns() {
    for (auto &b : activeBuffs) b.turnsRemaining--;
    removeExpiredBuffs();
}

void Player::removeExpiredBuffs() {
    activeBuffs.erase(std::remove_if(activeBuffs.begin(), activeBuffs.end(),
        [](const ActiveBuff& b){ return b.turnsRemaining <= 0; }), activeBuffs.end());
}

float Player::getOutgoingDamageMultiplier() const {
    float mult = 1.0f;
    for (const auto &b : activeBuffs) {
        if (b.affectsOutgoing) mult *= b.value;
    }
    return mult;
}

float Player::getIncomingDamageMultiplier() const {
    float mult = 1.0f;
    for (const auto &b : activeBuffs) {
        if (b.affectsIncoming) mult *= b.value;
    }
    return mult;
}

float Player::getHitModifier() const {
    float mult = 1.0f;
    for (const auto &b : activeBuffs) {
        // We will use name match or standardized buff names for hit modifiers
        if (b.name == "Hit Boost" || b.name == "Evade Boost") mult *= b.value;
        if (b.name == "Hit Reduction" || b.name == "Evade Reduction") mult *= b.value;
    }
    return mult;
}