//This file represents the game stack. Essentially, all the parts of the game(main menu, main game, etc.) exist as separate states and all exist on this stack. The separate files(game_state_editor, game_state_start) create these separate states and let the game stack manage them. The current one being used exists at the top of the stack and will then get popped off when not in use. 
#include <SFML/Graphics.hpp> 
#include <SFML/System.hpp>
#include <cmath>
#include <stack>
#include "game.hpp"
#include "game_state.hpp"
#include "texture_manager.hpp"
#include "Player.hpp"
#include "Map.hpp"
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include "game_state_editor.hpp"


void Game::loadTextures() // load textures used in game_state_start and game_state_editor (for the most part anyway)
{ 
    texmgr.loadTexture("background", "./assets/mainmenu.jpeg");
    texmgr.loadTexture("playerSprite", "./assets/player.png");
    texmgr.loadTexture("pmember2Sprite", "./assets/partymember2.png");
    texmgr.loadTexture("pmember3Sprite", "./assets/partymember3.png");
    texmgr.loadTexture("pmember4Sprite", "./assets/partymember4.png");
}
void Game::pushState(std::unique_ptr<GameState> state) //place game state onto stack
{
    this->states.push(std::move(state));
    return;
}

void Game::popState() //remove the game state off the stack as to save on memory
{
    if(!this->states.empty())
    this->states.pop();
}

void Game::changeState(std::unique_ptr<GameState> state) //change game state
{
if (!this->states.empty())
{
    popState();
}
pushState(std::move(state));
return;
}

GameState* Game::peekState() //check game state
{
    if(this->states.empty())
    {
        return nullptr;
    }
    return this->states.top().get();
}

// requestPush/requestPop/requestChange implementations
void Game::requestPush(std::unique_ptr<GameState> state) {
    pendingState.action = StateAction::Push;
    pendingState.state = std::move(state);
}

void Game::requestPop() {
    pendingState.action = StateAction::Pop;
}

void Game::requestChange(std::unique_ptr<GameState> state) {
    pendingState.action = StateAction::Change;
    pendingState.state = std::move(state);
}

void Game::applyPendingState() {
    switch (pendingState.action) {
        case StateAction::Push:
            if (pendingState.state) {
                pushState(std::move(pendingState.state));
            }
            break;
        case StateAction::Pop:
            popState();
            break;
        case StateAction::Change:
            if (pendingState.state) {
                changeState(std::move(pendingState.state));
            } else {
                // If state is null but action is Change, just pop then do nothing
                popState();
            }
            break;
        case StateAction::None:
        default:
            break;
    }
    // reset pendingState
    pendingState.action = StateAction::None;
    pendingState.state.reset();
}



void Game::gameLoop() //handles the gameloop
{
   sf::Clock clock;

   while(this-> window.isOpen())
   {
    sf::Time elapsed = clock.restart();
    float dt = elapsed.asSeconds();

    if(peekState() == nullptr)
    {
        // maybe sleep a bit here or continue
        std::cout << "peekState is null" << std::endl;
        continue;
    }
    peekState() -> handleInput();
    peekState() -> update(dt);
    this ->window.clear(sf::Color::Black);
    peekState()->draw(dt);
    this->window.display();

    // <-- apply pending state changes safely after current state finished
    applyPendingState();
   }
}

Game::Game() : hpItem("Dragon Morsel", "Makes you feel like something, but you can't put your finger on it. Heals 100HP.", 100, 0, 0), 
manaItem("Energizing Moss", "Some moss you found in a chest. Not safe for human consumption, but somehow restores 100MP.", 0, 100, 0),
pmember2("Maya", 1, 2, 3, 5, 3, 3, 0, {{"Fire", 1.0}, {"Ice", 0.5}, {"Physical", 1.0}, {"Force", 1.5}, {"Electric", 1.0}}),
pmember3("Lisa", 1, 3, 3, 4, 3, 2, 0, {{"Fire", 1.5}, {"Ice", 1.0}, {"Physical", 1.0}, {"Force", 1.0}, {"Electric", 1.5}}), 
pmember4("Eikichi", 1, 5, 2, 3, 2, 3, 0, {{"Fire", 1.0}, {"Ice", 1.5}, {"Physical", 0.5}, {"Force", 1.0}, {"Electric", 1.0}}) 
   
{
    
   this->loadTextures();
   this->window.create(sf::VideoMode(1920, 1080), "Untitled RPG Project");
   this->window.setFramerateLimit(60); 
   this->background.setTexture(this->texmgr.getRef("background"));
   this->playerSprite.setTexture(this->texmgr.getRef("playerSprite"));
   this->pmember2Sprite.setTexture(this->texmgr.getRef("pmember2Sprite"));
   this->pmember3Sprite.setTexture(this->texmgr.getRef("pmember3Sprite"));
   this->pmember4Sprite.setTexture(this->texmgr.getRef("pmember4Sprite"));
   if (!map.loadFromFile("assets/map1.txt")) {
    throw std::runtime_error("failed to load");
   }
    if (!font.loadFromFile("assets/Birch.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        std::exit(1);
    } else {
        std::cout << "Font loaded: " << font.getInfo().family << std::endl;
    }
    for (auto& s : skillMasterList) {
        if (s.getName() == "Marakunda") {
            std::cout << "[DEBUG] Marakunda damageResist = " << s.getDamageResist() << "\n";
            break;
        }
    } // DEBUG REMEMBER TO REMOVE EVENTUALLY



    this->doorCoordinates = map.getDoorCoordinates();
    for (const auto& coord : doorCoordinates) {
        std::cout << coord << " ";
    }
    std::cout << std::endl;
    for (const auto& coord : doorCoordinates) {
        doorCoordinatesToHasLoot[coord] = 1;
    }
    
}

Game::~Game() //get rid of all the things on the stack
{
while(!this->states.empty())
{
    popState();
}

}

using json = nlohmann::json;

// requires: #include <nlohmann/json.hpp>
// using json = nlohmann::json;

void Game::saveFromFile(const std::string& filename) const {
    json j;

    // --- Save main player ---
    PlayerData pdata = player.getData();
    j["player"]["position"] = { pdata.position.x, pdata.position.y };
    j["player"]["angle"] = pdata.angle;
    j["player"]["HP"] = pdata.HP;
    j["player"]["maxHP"] = pdata.maxHP;
    j["player"]["MP"] = pdata.MP;
    j["player"]["maxMP"] = pdata.maxMP;
    j["player"]["STR"] = pdata.STR;
    j["player"]["VIT"] = pdata.VIT;
    j["player"]["AGI"] = pdata.AGI;
    j["player"]["LU"] = pdata.LU;
    j["player"]["XP"] = pdata.XP;
    j["player"]["LVL"] = pdata.LVL;
    j["floorNumber"] = floorNumber;

    j["player"]["inventory"] = json::array();
    for (const auto& item : pdata.inventory) {
        j["player"]["inventory"].push_back({
            {"name", item.showName()},
            {"quantity", item.getQuantity()}
        });
    }
    j["player"]["affinities"] = pdata.affinities;
    j["player"]["skills"] = pdata.skills;

    // --- Save explicit party members that Game owns ---
    j["party"] = json::array();

    auto pushMember = [&](const Player& pm) {
        PlayerData pd = pm.getData();
        json pmj;
        pmj["position"] = { pd.position.x, pd.position.y };
        pmj["angle"] = pd.angle;
        pmj["HP"] = pd.HP;
        pmj["maxHP"] = pd.maxHP;
        pmj["MP"] = pd.MP;
        pmj["maxMP"] = pd.maxMP;
        pmj["STR"] = pd.STR;
        pmj["VIT"] = pd.VIT;
        pmj["AGI"] = pd.AGI;
        pmj["LU"] = pd.LU;
        pmj["XP"] = pd.XP;
        pmj["LVL"] = pd.LVL;

        pmj["inventory"] = json::array();
        for (const auto& item : pd.inventory) {
            pmj["inventory"].push_back({
                {"name", item.showName()},
                {"quantity", item.getQuantity()}
            });
        }

        pmj["affinities"] = pd.affinities;
        pmj["skills"] = pd.skills;

        j["party"].push_back(pmj);
    };

    // Push each pmember explicitly (skip empty/unused ones if you like)
    pushMember(player);      // optional if you want player also in party array; keep if load expects it
    pushMember(pmember2);
    pushMember(pmember3);
    pushMember(pmember4);

    std::ofstream out(filename);
    out << j.dump(4);
}

bool Game::loadFromFile(const std::string& filename, const std::vector<Skill>& masterList) {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    json j;
    in >> j;
    
    if (j.contains("floorNumber")) {
        floorNumber = j["floorNumber"];
    }
    this->states.push(std::make_unique<GameStateEditor>(this, false, floorNumber));



    // --- Load main player ---
    if (j.contains("player")) {
        PlayerData pdata;
        if (j["player"].contains("position") && j["player"]["position"].is_array() && j["player"]["position"].size() >= 2) {
            pdata.position.x = j["player"]["position"][0].get<float>();
            pdata.position.y = j["player"]["position"][1].get<float>();
        }
        pdata.angle = j["player"].value("angle", 0.0f);
        pdata.HP = j["player"].value("HP", pdata.HP);
        pdata.maxHP = j["player"].value("maxHP", pdata.maxHP);
        pdata.MP = j["player"].value("MP", pdata.MP);
        pdata.maxMP = j["player"].value("maxMP", pdata.maxMP);
        pdata.STR = j["player"].value("STR", pdata.STR);
        pdata.VIT = j["player"].value("VIT", pdata.VIT);
        pdata.AGI = j["player"].value("AGI", pdata.AGI);
        pdata.LU = j["player"].value("LU", pdata.LU);
        pdata.XP = j["player"].value("XP", pdata.XP);
        pdata.LVL = j["player"].value("LVL", pdata.LVL);

        // inventory
        if (j["player"].contains("inventory") && j["player"]["inventory"].is_array()) {
            size_t idx = 0;
            for (const auto& item_json : j["player"]["inventory"]) {
                if (idx >= pdata.inventory.size()) break;
                pdata.inventory[idx] = Item(
                    item_json.value("name", std::string("Empty Slot")),
                    "None", 0, 0,
                    item_json.value("quantity", 0)
                );
                ++idx;
            }
        }

        if (j["player"].contains("affinities"))
            pdata.affinities = j["player"]["affinities"].get<decltype(pdata.affinities)>();
        if (j["player"].contains("skills"))
            pdata.skills = j["player"]["skills"].get<decltype(pdata.skills)>();

        // apply loaded data (ensure setData uses pdata.LVL to set LVL)
        player.setData(pdata, masterList, true);
    }

    // --- Load explicit party members into Game's pmember2/3/4 ---
    // Expecting party array in order: player, pmember2, pmember3, pmember4
    if (j.contains("party") && j["party"].is_array()) {
        const auto& parr = j["party"];
        // index 0 -> player (optional), 1->pmember2, 2->pmember3, 3->pmember4
        if (parr.size() > 1) {
            // pmember2
            const auto& pmj = parr[1];
            PlayerData pd;
            if (pmj.contains("position") && pmj["position"].is_array() && pmj["position"].size() >= 2) {
                pd.position.x = pmj["position"][0].get<float>();
                pd.position.y = pmj["position"][1].get<float>();
            }
            pd.angle = pmj.value("angle", 0.0f);
            pd.HP = pmj.value("HP", pd.HP);
            pd.maxHP = pmj.value("maxHP", pd.maxHP);
            pd.MP = pmj.value("MP", pd.MP);
            pd.maxMP = pmj.value("maxMP", pd.maxMP);
            pd.STR = pmj.value("STR", pd.STR);
            pd.VIT = pmj.value("VIT", pd.VIT);
            pd.AGI = pmj.value("AGI", pd.AGI);
            pd.LU = pmj.value("LU", pd.LU);
            pd.XP = pmj.value("XP", pd.XP);
            pd.LVL = pmj.value("LVL", pd.LVL);

            if (pmj.contains("inventory") && pmj["inventory"].is_array()) {
                size_t idx = 0;
                for (const auto& item_json : pmj["inventory"]) {
                    if (idx >= pd.inventory.size()) break;
                    pd.inventory[idx] = Item(
                        item_json.value("name", std::string("Empty Slot")),
                        "None", 0, 0,
                        item_json.value("quantity", 0)
                    );
                    ++idx;
                }
            }

            if (pmj.contains("affinities")) pd.affinities = pmj["affinities"].get<decltype(pd.affinities)>();
            if (pmj.contains("skills")) pd.skills = pmj["skills"].get<decltype(pd.skills)>();

            pmember2.setData(pd, masterList, true);
        }

        if (parr.size() > 2) {
            const auto& pmj = parr[2];
            PlayerData pd;
            if (pmj.contains("position") && pmj["position"].is_array() && pmj["position"].size() >= 2) {
                pd.position.x = pmj["position"][0].get<float>();
                pd.position.y = pmj["position"][1].get<float>();
            }
            pd.angle = pmj.value("angle", 0.0f);
            pd.HP = pmj.value("HP", pd.HP);
            pd.maxHP = pmj.value("maxHP", pd.maxHP);
            pd.MP = pmj.value("MP", pd.MP);
            pd.maxMP = pmj.value("maxMP", pd.maxMP);
            pd.STR = pmj.value("STR", pd.STR);
            pd.VIT = pmj.value("VIT", pd.VIT);
            pd.AGI = pmj.value("AGI", pd.AGI);
            pd.LU = pmj.value("LU", pd.LU);
            pd.XP = pmj.value("XP", pd.XP);
            pd.LVL = pmj.value("LVL", pd.LVL);

            if (pmj.contains("inventory") && pmj["inventory"].is_array()) {
                size_t idx = 0;
                for (const auto& item_json : pmj["inventory"]) {
                    if (idx >= pd.inventory.size()) break;
                    pd.inventory[idx] = Item(
                        item_json.value("name", std::string("Empty Slot")),
                        "None", 0, 0,
                        item_json.value("quantity", 0)
                    );
                    ++idx;
                }
            }

            if (pmj.contains("affinities")) pd.affinities = pmj["affinities"].get<decltype(pd.affinities)>();
            if (pmj.contains("skills")) pd.skills = pmj["skills"].get<decltype(pd.skills)>();

            pmember3.setData(pd, masterList, true);
        }

        if (parr.size() > 3) {
            const auto& pmj = parr[3];
            PlayerData pd;
            if (pmj.contains("position") && pmj["position"].is_array() && pmj["position"].size() >= 2) {
                pd.position.x = pmj["position"][0].get<float>();
                pd.position.y = pmj["position"][1].get<float>();
            }
            pd.angle = pmj.value("angle", 0.0f);
            pd.HP = pmj.value("HP", pd.HP);
            pd.maxHP = pmj.value("maxHP", pd.maxHP);
            pd.MP = pmj.value("MP", pd.MP);
            pd.maxMP = pmj.value("maxMP", pd.maxMP);
            pd.STR = pmj.value("STR", pd.STR);
            pd.VIT = pmj.value("VIT", pd.VIT);
            pd.AGI = pmj.value("AGI", pd.AGI);
            pd.LU = pmj.value("LU", pd.LU);
            pd.XP = pmj.value("XP", pd.XP);
            pd.LVL = pmj.value("LVL", pd.LVL);

            if (pmj.contains("inventory") && pmj["inventory"].is_array()) {
                size_t idx = 0;
                for (const auto& item_json : pmj["inventory"]) {
                    if (idx >= pd.inventory.size()) break;
                    pd.inventory[idx] = Item(
                        item_json.value("name", std::string("Empty Slot")),
                        "None", 0, 0,
                        item_json.value("quantity", 0)
                    );
                    ++idx;
                }
            }

            if (pmj.contains("affinities")) pd.affinities = pmj["affinities"].get<decltype(pd.affinities)>();
            if (pmj.contains("skills")) pd.skills = pmj["skills"].get<decltype(pd.skills)>();

            pmember4.setData(pd, masterList, true);
        }
    }

    return true;
}