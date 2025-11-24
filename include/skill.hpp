
#pragma once
#include <string>

class Skill{
    private:
    // all floats are percentage based, all ints are flat numbers
    std::string name = "";
    std::string description = "N/A";
    std::string type = "N/A";
    int unlockLevel = 0;
    bool isSingleTarget = true;
    bool targetsEnemies = true;
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
    float healthRestorePercent = 0.0;
    

    public:
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    std::string getType() const { return type; }
    int getUnlockLevel() const { return unlockLevel; }
    bool getIsSingleTarget() const { return isSingleTarget; }
    int getBaseAtk() const { return baseAtk; }
    float getHpCost() const { return hpCost; }
    int getMpCost() const { return mpCost; }
    float getBaseHitRate() const { return baseHitRate; }
    float getMissRate() const { return missRate; }
    int getLimit() const { return limit; }
    int getCorrection() const { return correction; }
    float getDamageResist() const { return damageResist; } //im not sure if i want to do this
    float getDamageAmp() const { return damageAmp; }
    float getHitEvadeBoost() const {return hitEvadeBoost;}
    float getHitEvadeReduction() const {return hitEvadeReduction; }
    float getHealthRestorePercent() const { return healthRestorePercent; }
    float getCritRate() const { return critRate; }


    Skill() = default;
    Skill(std::string name, std::string description, std::string type, int unlockLevel, bool isSingleTarget, int baseAtk, float hpCost, float baseHitRate, float missRate, float critRate); //phys skill constructor
    Skill(std::string name, std::string description, std::string type, int unlockLevel, bool isSingleTarget, int baseAtk, int mpCost, float baseHitRate, float missRate, int limit, int correction); //magic skill constructor
    Skill(std::string name, std::string description, std::string type, int unlockLevel, bool isSingleTarget, bool targetsEnemies, int mpCost, float damageResist, float damageAmp, float hitEvadeBoost, float hitEvadeReduction); //magic utility skills constructor (damage boost, damage resist, evasion/accuracy boost)
    Skill(std::string name, std::string description, std::string type, int unlockLevel, bool isSingleTarget, int mpCost, float healthRestorePercent); //magic healing skills constructor 
};
