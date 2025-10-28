#ifndef NPC_h
#define NPC_h

#include "Player.hpp"
#include "skill.hpp"
#include <array>

class NPC : public Player {
    public:
    NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities, std::array<Skill, 9> skillList);
    ~NPC();


};

#endif // NPC_Hl