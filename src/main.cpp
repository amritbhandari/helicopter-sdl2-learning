#include <iostream>
#include "Game.h"

using namespace std;

int main()
{
    Game game{};

    if (!game.init("Helicopter"))
    {
        game.clean();
        return 1;
    }

    while (game.playing())
    {
        game.handleEvents();
        game.update();
        game.render();
    }
    game.clean();

    return 0;
}