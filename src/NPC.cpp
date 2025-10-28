#include "NPC.hpp"
#include <string>
#include <map>
NPC::NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP,  std::map<std::string, float> affinities) : 
    Player(name, LVL, STR, VIT, MAG, AGI, LU, XP, affinities){}
NPC::~NPC() = default;

