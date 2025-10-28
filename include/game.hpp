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
};

#endif