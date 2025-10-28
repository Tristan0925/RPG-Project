
#pragma once
#include <string>

class Skill{
    private:
    // all floats are percentage based, all ints are flat numbers
    std::string name = "N/A";
    std::string description = "N/A";
    std::string type = "N/A";
    bool isSingleTarget = true;
    int baseAtk = 0;
    float hpCost = 0.0; 
    int mpCost = 0;
    float baseHitRate = 0.0;
    float missRate = 0.0;
    float critRate = 0.0;
    int limit = 0;
    int correction = 0;
    float damageResist = 0.0; //im not sure if i want to do this
    float damageAmp = 0.0;
    float hitEvadeBoost = 0.0;
    float hitEvadeReduction = 0.0;

    public:
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    std::string getType() const { return type; }
    bool getIsSingleTarget() const {return isSingleTarget;}
    int getBaseAtk() const { return baseAtk; }
    float getHpCost() const { return hpCost; }
    int getMpCost() const { return mpCost; }
    float getBaseHitRate() const { return baseHitRate; }
    float getMissRate() const { return missRate; }
    int getLimit() const { return limit; }
    int getCorrection() const { return correction; }
    Skill(std::string name, std::string description, std::string type, bool isSingleTarget, int baseAtk, float hpCost, float baseHitRate, float missRate, float critRate); //phys skill constructor
    Skill(std::string name, std::string description, std::string type, bool isSingleTarget, int baseAtk, int mpCost, float baseHitRate, float missRate, int limit, int correction); //magic skill constructor
    Skill(std::string name, std::string description, std::string type, bool isSingleTarget, float damageResist, float damageAmp, float hitEvadeBoost, float hitEvadeReduction); //magic utility skills constructor (damage boost, damage resist, evasion/accuracy boost)
};
