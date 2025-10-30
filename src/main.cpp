#include <iostream>
#include "Game.h"
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

using namespace std;

int main()
{
    Game game{};

    if (!game.init("Helicopter"))
    {
        game.clean();
        return 1;
    }

    // random seed
    srand(time(nullptr));

    while (game.playing())
    {
        game.handleEvents();
        game.update();
        game.render();
    }
    game.clean();

    return 0;
}