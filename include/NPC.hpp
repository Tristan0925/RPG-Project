#ifndef NPC_h
#define NPC_h

#include "Player.hpp"

class NPC : public Player {
    public:
    NPC(std::string name, int HP, int maxHP, int MP, int maxMP, int STR, int VIT, int AGI, int LU, int XP, int LVL);
    ~NPC();


};

#endif // NPC_Hl