#include "NPC.hpp"

NPC::NPC(std::string name, int HP, int maxHP, int MP, int maxMP, int STR, int VIT, int AGI, int LU, int XP, int LVL) : 
    Player(name, HP, maxHP, MP, maxMP, STR, VIT, AGI, LU, XP, LVL){}
NPC::~NPC() = default;
