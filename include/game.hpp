#ifndef GAME_HPP
#define GAME_HPP

#include <stack>
#include <SFML/Graphics.hpp>
#include <memory>
#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"
#include "NPC.hpp"
#include "item.hpp"
#include "skill.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class GameState; // forward declaration

// pending-state system (declared BEFORE Game)
enum class StateAction { None, Push, Pop, Change };

struct PendingState {
    StateAction action = StateAction::None;
    std::unique_ptr<GameState> state;
};

#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"
#include <string>
#include <array>

class Game
{
private:
    void loadTextures();

    // store a pending state request to be applied safely in the game loop
    PendingState pendingState;
    public:
    Item hpItem;
    Item manaItem;
    Player player;
    NPC pmember2;
    NPC pmember3;
    NPC pmember4;

    std::array<std::string, 9> playerSkills = {"Attack", "Shock", "Dia", "Fire Breath", "Flash Freeze", "Tornado", "Divine Shot", "Freikugel", "Focus"};
    std::array<std::string, 9> pmember2Skills = {"Attack", "Zio", "Agi", "Matarukaja", "Bufula", "Megidola", "Omni-Dimension", "EMPTY SLOT", "EMPTY SLOT"}; //if you want you could make more skills 
    std::array<std::string, 9> pmember3Skills = {"Attack", "Zanma", "Diarama", "Marakunda", "Mediarama", "Scorched Earth", "EMPTY SLOT", "EMPTY SLOT", "EMPTY SLOT"};
    std::array<std::string, 9> pmember4Skills = {"Attack", "Andalucia", "Mabufula", "Masukukaja", "Debilitate", "Spiral Viper", "EMPTY SLOT", "EMPTY SLOT", "EMPTY SLOT"};


    Map map;
    std::vector<std::string> doorCoordinates;
    std::unordered_map<std::string, bool> doorCoordinatesToHasLoot;
    sf::Font font;
      

    std::stack<std::unique_ptr<GameState>> states;

    sf::RenderWindow window;
    TextureManager texmgr;
    sf::Sprite background;
    //Reused UI elements
    sf::Sprite playerSprite;
    sf::Sprite pmember2Sprite;
    sf::Sprite pmember3Sprite;
    sf::Sprite pmember4Sprite;
    

    // Immediate operations (internal helpers)
    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void changeState(std::unique_ptr<GameState> state);
    GameState* peekState();

    // Request-based API (queue changes safely)
    void requestPush(std::unique_ptr<GameState> state);
    void requestPop();
    void requestChange(std::unique_ptr<GameState> state);

    // apply any pending request (call from game loop)
    void applyPendingState();
    void gameLoop();

    Game();
    ~Game();
    std::vector <Skill> skillMasterList = { 
        Skill("Attack", "Basic attack made with your weapon.", "Physical", 1, true, 16, 0.0f, 1.0f, 0.03f, 0.13f), 
        
        Skill("Freikugel","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Physical-Almighty", 10, true, 65, 0.17f, 1.0f, 0.03f, .30f), 

        Skill("Divine Shot", "High physical damage to one foe. Medium crit rate.", "Physical", 9, true, 50, 0.13f, 1.0f, 0.03f, 0.30f), 

        Skill("Fire Breath", "Low fire damage to all foes.", "Fire", 4, false, 30, 7, 1.0f, 0.0f, 85, 8), 
        
        Skill("Tornado", "High force damage to all foes.", "Force", 7, true, 65, 17, 1.0f, 0.0f, 183, 19),
        
        Skill("Flash Freeze", "Medium ice damage to one foe.", "Ice", 5, true, 45, 9, 1.0f, 0.0f, 120, 10),  
        
        Skill("Focus", "Multiplies the damage of next physical attack by 2.5x.","Damage Amp", 10, true, 20, 0.0f, 2.50f, 0.0f, 0.0f), 
        
        Skill("Dia", "Moderately restores one ally's health.", "Healing", 2, true, 3, 1.25f), 
        
        Skill("Shock", "Low electric damage to all foe.", "Electric", 1, false, 23, 6, 1.0f, 0.0f, 70, 7), 
        
        Skill("Zio","Low electric damage to a single foe.", "Electric", 1, true, 30, 3, 1.0f, 0.0f, 85, 9), 
        
        Skill("Matarukaja", "Raises attack power for all allies by 25%.", "Damage Amp", 5, false, false, 18, 0.0f, 1.25f, 0.0f, 0.0f), 
        
        Skill("Agi", "Low fire damage to a single foe.", "Fire", 3, true, 37, 3, 1.0f, 0.0f, 104, 10),
        
        Skill("Bufula", "Moderate ice damage to a single enemy.", "Ice", 7, true, 45, 6, 1.0f, 0.0f, 126, 13), 
        
        Skill("Mediarama", "Greatly restores all allies' health.", "Healing", 8, false, 20, 1.50f),  
        
        Skill("Omni-Dimension", "Massive almighty damage to all foes.", "Almighty", 10, false, 90, 32, 1.0f, 0.0f, 253, 26), 
        
        Skill("Zanma","Moderate force damage to a single enemy.", "Force", 1, true, 55, 6, 1.0f, 0.0f, 155, 16), 
        
        Skill("Diarama", "Greatly restores one ally's health.", "Healing", 3, true, 7, 1.50f), 
        
        Skill("Marakunda", "Reduces defense of all enemies by 25%.", "Damage Resist", 5, false, true, 20, -0.25f, 0.0f,0.0f ), 

        Skill("Megidola", "Heavy almighty damage to all enemies.", "Magic-Almighty", 7, true, 80, 37, 1.0f, 0.0f, 225, 23), 
        
        Skill("Scorched Earth", "Massive fire damage to all enemies.", "Fire", 10, true, 90, 30, 1.0f, 0.0f, 253, 26),  
        
        Skill("Andalucia", "Medium physical damage to all enemies. Minimal chance to crit.", "Physical", 1, false, 32, 0.06f, 1.0f, 0.0f, 0.01f), 
        
        Skill("Mabufula", "Moderate ice damage to all enemies.", "Ice", 3, false, 35, 15, 1.0f, 0.0f, 98, 10), 
        
        Skill("Masukukaja", "Increase all allies' evasion and accuracy by 25%.", "Hit Evade Boost", 7, true, false, 20, 0.0f, 0.0f, 0.25f, 0.00f), 
        
        Skill("Debilitate", "Decrase all foes' evasion and accuracy by 50%.", "Hit Evade Reduction", 9, true, true, 25, 0.0f, 0.0f, 0.0f, -0.5f),  
        
        Skill("Spiral Viper", "Massive physical damage to a single foe.","Physical", 10, true, 90, 0.16f, 1.0f, 0.03f, 0.3f),  

        Skill(), //Roary Nyte Moves: Sever (Medium Phys Single Move), Falling Stars (Medium Almighty All Move), Piercing Blade (Heavy Phys Single Move), Reckoning (Heavy Almighty Single Move) idk how we balance any of these
        //Shaddai Moves: Godly Judgement (Light Almighty Single), El Roi (Light Almighty All), Zio, Shock
    
    
    
    };
};
#endif 
