#include "NPC.hpp"
#include "skill.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <array>

// Party member constructor
NPC::NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP,  std::map<std::string, float> affinities)
: Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities), 
baseXPAmount(0),
animationsLocation({}),
spriteLocation(""),
isBoss(false)
{}

// Enemy constructor
NPC::NPC(std::string name, std::string spriteLoc, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXP, std::map<std::string, float> affinities, bool isBoss, const std::vector<std::string>& skills)
: Player(name, LVL, STR, VIT, MAG, AGI, LU, 0, affinities), baseXPAmount(baseXP),
animationsLocation({}),
spriteLocation(spriteLoc),
isBoss(isBoss),
skillNames(skills)  
{}

// Boss constructor
NPC::NPC(std::string name, std::string spriteLoc, std::string animLoc, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXP, std::map<std::string, float> affinities, bool boss, const std::vector<std::string>& skills)
: Player(name, LVL, STR, VIT, MAG, AGI, LU, 0, affinities),
baseXPAmount(baseXP),
animationsLocation(animLoc),
spriteLocation(spriteLoc),
isBoss(boss),
skillNames(skills)
{}



int NPC::getBaseXPAmount() const {
    return baseXPAmount;
 }

std::string NPC::getAnimationsLocation() const {
    return animationsLocation;
}

std::string NPC::getSpriteLocation() const {
    return spriteLocation;
}
bool NPC::getIsBoss() const {
    return isBoss;
}

NPC::~NPC() = default;

