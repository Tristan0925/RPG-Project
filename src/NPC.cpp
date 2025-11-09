#include "NPC.hpp"
#include "skill.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <array>

// Party member constructor
NPC::NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP,  std::map<std::string, float> affinities)
: baseXPAmount(0),
animationsLocation({}),
spriteLocation(""),
isBoss(false),
Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities)
{}

// Enemy constructor
NPC::NPC(std::string name, std::string spriteLoc, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXP, std::map<std::string, float> affinities)
: baseXPAmount(baseXP),
animationsLocation({}),
spriteLocation(spriteLoc),
isBoss(false),
Player(name, LVL, STR, VIT, MAG, AGI, LU, 0, affinities)
{}

// Boss constructor
NPC::NPC(std::string name, std::string spriteLoc, std::vector<std::string> animLoc, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXP, std::map<std::string, float> affinities, bool boss)
: baseXPAmount(baseXP),
animationsLocation(animLoc),
spriteLocation(spriteLoc),
isBoss(boss),
Player(name, LVL, STR, VIT, MAG, AGI, LU, 0, affinities)
{}



int NPC::getBaseXPAmount() const {
    return baseXPAmount;
 }

std::vector<std::string> NPC::getAnimationsLocation() const {
    return animationsLocation;
}

std::string NPC::getSpriteLocation() const {
    return spriteLocation;
}
bool NPC::getIsBoss() const {
    return isBoss;
}

NPC::~NPC() = default;

