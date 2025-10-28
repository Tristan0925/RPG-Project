#ifndef NPC_h
#define NPC_h

#include "Player.hpp"

class NPC : public Player {
    public:
    NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP,  std::map<std::string, float> affinities);
    ~NPC();


};

#endif // NPC_Hl