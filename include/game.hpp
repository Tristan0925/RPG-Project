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

class GameState;

class Game
{
    private:

    void loadTextures();


    public:
    Item hpItem;
    Item manaItem;
    Player player;
    NPC pmember2;
    NPC pmember3;
    NPC pmember4;
  


    Map map;
    std::vector<std::string> doorCoordinates;
    std::unordered_map<std::string, bool> doorCoordinatesToHasLoot;
    sf::Font font;
      

    std::stack<std::unique_ptr<GameState>> states;

    sf::RenderWindow window;
    TextureManager texmgr;
    sf::Sprite background;
    sf::Sprite playerSprite;
    sf::Sprite pmember2Sprite;
    sf::Sprite pmember3Sprite;
    sf::Sprite pmember4Sprite;

    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void changeState(std::unique_ptr<GameState> state);
    GameState* peekState();

    void gameLoop();

    Game();
    ~Game();
    std::vector <Skill> skillMasterList = {
        Skill("Attack", "Basic attack made with your fists.", "Physical", 1, true, 32, 0.0f, 100.0f, 3.0f, 13.0f), 
        
        Skill("Freikugel","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Physical-Almighty", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 

        Skill("Divine Shot", "High physical damage to one foe. Medium crit rate.", "Physical", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 

        Skill("Fire Breath", "Low fire damage to all foes.", "Fire", 4, false, 30, 7, 100.0f, 0.0f, 85, 8), 
        
        Skill("Tornado", "High force damage to all foes.", "Force", 7, true, 65, 17, 100.0f, 0.0f, 183, 19),
        
        Skill("Flash Freeze", "Medium ice damage to one foe.", "Ice", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
        
        Skill("Focus", "Multiplies the damage of next physical attack by 2.5x.","Damage Amp", 10, true, 0.0f, 2.5f, 0.0f, 0.0f), 
        
        Skill("Dia", "Moderately restores one ally's health.", "Healing", 2, true, 7, 15.0f), 
        
        Skill("Shock", "Low electric damage to one foe.", "Electric", 1, true, 25, 5, 100.0f, 0.0f, 70, 7), 
        
        Skill("Zio","Low electric damage to a single foe. High accuracy and medium crit rate.", "Electric", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 
        
        Skill("Tarunda", "Raises attack power for a single ally by 25%.", "Damage Amp", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 
        
        Skill("Agi", "Low fire damage to all foes.", "Fire", 4, false, 30, 7, 100.0f, 0.0f, 85, 8),
        
        Skill("Bufula", "Moderate ice damage to a single enemy.", "Ice", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), 
        
        Skill("Mediarama", "Medium ice damage to one foe.", "Healing", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
        
        Skill("Omni-Dimension", "Multiplies the damage of next physical attack by 2.5x.","Almighty", 10, true, 0.0f, 2.5f, 0.0f, 0.0f), 
        
        Skill("Zanma","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Force", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 
        
        Skill("Diarama", "High physical damage to one foe. Medium crit rate.", "Healing", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 
        
        Skill("Rakunda", "Reduces defense of all enemies by 25%.", "Damage Resist", 4, false, 30, 7, 100.0f, 0.0f, 85, 8), 
        
        Skill("Megido", "High force damage to all foes.", "Magic-Almighty", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), 
        
        Skill("Sinful Shell", "40% ", "Physical", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
        
        Skill("Mediarama", "Multiplies the damage of next physical attack by 2.5x.","Healing", 10, true, 0.0f, 2.5f, 0.0f, 0.0f),
        
        Skill("Bufula","Massive phys-almighty damage to one foe. High accuracy and medium crit rate.", "Ice", 10, true, 65, 17.0f, 100.0f, 3.0f, 30.0f), 
        
        Skill("Andalucia", "High physical damage to one foe. Medium crit rate.", "Physical", 9, true, 50, 13.0f, 100.0f, 3.0f, 30.0f), 
        
        Skill("Mabufula", "Low fire damage to all foes.", "Ice", 4, false, 30, 7, 100.0f, 0.0f, 85, 8), 
        
        Skill("Matarukaja", "High force damage to all foes.", "Damage Amp", 7, true, 65, 17, 100.0f, 0.0f, 183, 19), 
        
        Skill("Zanma", "Medium ice damage to one foe.", "Force", 5, true, 45, 9, 100.0f, 0.0f, 120, 10),  
        
        Skill("Stasis Blade", "Multiplies the damage of next physical attack by 2.5x.","Physical", 10, true, 0.0f, 2.5f, 0.0f, 0.0f),  

        Skill(), //empty
    
    
    
    };
};
#endif 