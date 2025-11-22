#ifndef NPC_h
#define NPC_h

#include "Player.hpp"
#include "skill.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <string>

class NPC : public Player {
    private:
        int baseXPAmount = 0;
        std::vector<std::string> animationsLocation;
        std::string spriteLocation;
        bool isBoss = false;
        public:
        NPC(std::string name, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities); //party member constructor
        NPC(std::string name, std::string spriteLocation, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int baseXPAmount, std::map<std::string, float> affinities, bool isBoss = false); //enemy constructor
        NPC(std::string name, std::string spriteLocation, std::vector<std::string> animationsLocation, int LVL, int STR, int VIT, int MAG, int AGI, int LU, int XP, std::map<std::string, float> affinities, bool isBoss = true); //boss constructor, only they have animations for my own sanity
        ~NPC();
        int getBaseXPAmount() const;
        std::vector<std::string> getAnimationsLocation() const;
        std::string getSpriteLocation () const;
        bool getIsBoss() const;
        std::vector<std::string> skillNames;
    public:
        const std::string& getName() const { return name; }
        const std::string& getDisplayName() const { return name; }
        void setSkillNames(const std::vector<std::string>& names) { skillNames = names; }
        const std::vector<std::string>& getSkillNames() const { return skillNames; }
};

#endif // NPC_H