#include "skill.hpp"
#include "string"

Skill::Skill(std::string name, std::string description, std::string type, bool isSingleTarget, int baseAtk, float hpCost, float baseHitRate, float missRate, float critRate) :
name(name), description(description), type(type), isSingleTarget(isSingleTarget), baseAtk(baseAtk), hpCost(hpCost), baseHitRate(baseHitRate), missRate(missRate), critRate(critRate){}

Skill::Skill(std::string name, std::string description, std::string type, bool isSingleTarget, int baseAtk, int mpCost, float baseHitRate, float missRate, int limit, int correction) :
name(name), description(description), type(type), isSingleTarget(isSingleTarget), baseAtk(baseAtk), mpCost(mpCost), baseHitRate(baseHitRate), missRate(missRate), limit(limit), correction(correction){}

Skill::Skill(std::string name, std::string description, std::string type, bool isSingleTarget, float damageResist, float damageAmp, float hitEvadeBoost, float hitEvadeReduction) :
name(name), description(description), type(type), isSingleTarget(isSingleTarget), damageResist(damageResist), damageAmp(damageAmp), hitEvadeBoost(hitEvadeBoost), hitEvadeReduction(hitEvadeReduction){}