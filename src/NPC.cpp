#include "NPC.hpp"
#include "skill.hpp"
#include <string>
#include <map>
#include <array>
NPC::NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities, std::array<Skill, 9> skillList) : 
    Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities, skillList){}
NPC::~NPC() = default;

