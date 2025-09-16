#include "game.hpp"
#include "game_state_start.hpp"

int main()
{
    Game game;

    game.pushState(std::make_unique<GameStateStart>(&game));
    game.gameLoop();

    return 0;
}
