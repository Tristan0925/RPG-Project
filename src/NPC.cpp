#include "NPC.hpp"
#include "skill.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <array>
NPC::NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities, std::array<Skill, 9> skillList) : //party member constructor 
    Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities, skillList){} 
NPC::NPC(std::string name, std::string spriteLocation, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXPAmount, std::map<std::string, float> affinities, std::array<Skill, 9> skillList) :
    Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities, skillList){} //enemy constructor
NPC::NPC(std::string name, std::string spriteLocation, std::vector<std::string>, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXPAmount, std::map<std::string, float> affinities, std::array<Skill, 9> skillList) :
    Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities, skillList){} //boss constructor
int NPC::getBaseXPAmount() const {
    return baseXPAmount;
 }

std::vector<std::string> NPC::getAnimationsLocation() const {
    return animationsLocation;
}

std::string NPC::getSpriteLocation() const {
    return spriteLocation;
}
NPC::~NPC() = default;

