#ifndef HELICOPTER_SDL2_GAME_H
#define HELICOPTER_SDL2_GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

using namespace std;

class Game
{
public:
    Game(): SCREEN_WIDTH(600), SCREEN_HEIGHT(400)
    {
    }

    ~Game()
    {
    }

    bool init(const char* title)
    {
        return init(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, false);
    }

    /**
     * @brief Initialise SDL.
     *
     * Initialise SDL. Define a Window.
     * @param title title to display.
     * @param xpos top left position.
     * @param ypos top right position.
     * @param width width of window.
     * @param height height of window.
     * @param flag display modes.
     * @return true if initialisation worked without any issues, false otherwise
     */
    bool init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            cerr << "SDL_Init" << SDL_GetError() << endl;
            return false;
        }

        if (TTF_Init() == -1)
        {
            cerr << "TTF_Init error" << TTF_GetError() << endl;
            return false;
        }

        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;
        int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN;
        window = SDL_CreateWindow(title, xpos, ypos, width, SCREEN_HEIGHT, flags);
        if (!window)
        {
            cerr << "SDL_CreateWindow error: " << SDL_GetError() << endl;
            SDL_Quit();
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            cerr << "SDL_CreateRenderer error: " << SDL_GetError() << endl;
            SDL_Quit();
            return false;
        }

        if (!loadMedia())
        {
            SDL_Quit();
            return false;
        }

        helicopterX = (SCREEN_WIDTH - HELICOPTER_WIDTH) /4;
        helicopterY = SCREEN_HEIGHT - HELICOPTER_HEIGHT - 100;

        continuePlaying = true;
        continueGame = true;

        return true;
    }

    void render()
    {
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer); // clear the renderer to the draw color

        if (continueGame)
        {
            renderHelicopter();
        }
        else
        {
            renderGameOver();
            renderPlayAgainText();
        }

        SDL_RenderPresent(renderer); // draw to the screen
    }

    void update()
    {
    }

    void handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                continuePlaying = false;
                break;
            case SDL_KEYUP:
                if (!continueGame)
                {
                    if (event.key.keysym.sym == SDLK_SPACE)
                        resetGame();
                }
                break;
            default: ;
                // do nothing
            }
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
        if (currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_DOWN])
        {
            if (currentKeyStates[SDL_SCANCODE_UP]) // up arrow key
            {
                helicopterY -= HELICOPTER_SPEED;
            }
            if (currentKeyStates[SDL_SCANCODE_DOWN]) // down arrow key
            {
                helicopterY += HELICOPTER_SPEED;
            }

            detectHelicopterCollisionWithWalls();
        }
    }

    void clean()
    {
        SDL_DestroyTexture(helicopterTexture);
        SDL_DestroyTexture(gameOverTexture);
        SDL_DestroyTexture(playAgainTexture);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();
    }

    // a function to access the private running variable
    bool playing() { return continuePlaying; }
    bool gameOn() { return continueGame; }

private:
    int SCREEN_WIDTH = 600;
    int SCREEN_HEIGHT = 400;

    const char* SPRITES_FOLDER = "images/";
    const char* FONT_PATH = "images/consolas.ttf";
    const int FONT_SIZE = 32;

    const string GAME_OVER_IMAGE_PATH = SPRITES_FOLDER + string("gameover.png");

    const string HELICOPTER_IMAGE_PATH = SPRITES_FOLDER + string("helicopter2.png");
    const int HELICOPTER_WIDTH = 50;
    const int HELICOPTER_HEIGHT = 30;
    const int HELICOPTER_SPEED = 1;
    int helicopterX;
    int helicopterY;
    // const int helicopterXLeftBoundary = 0;
    // const int helicopterXRightBoundary = SCREEN_WIDTH - HELICOPTER_WIDTH;
    const int helicopterYTopBoundary = 0;
    const int helicopterYBottomBoundary = SCREEN_HEIGHT - HELICOPTER_HEIGHT;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* helicopterTexture = nullptr;
    SDL_Texture* gameOverTexture = nullptr;
    SDL_Texture* playAgainTexture = nullptr;

    TTF_Font* font = nullptr;

    bool continuePlaying = false;
    bool continueGame = false;

    static bool checkCollision(const SDL_Rect& rect1, const SDL_Rect& rect2)
    {
        return SDL_HasIntersection(&rect1, &rect2);
    }

    void detectHelicopterCollisionWithWalls()
    {
        // if (helicopterX < helicopterXLeftBoundary || helicopteX > helicopterXRightBoundary)
        // {
        //     // collided vertical walls
        // }
        if (helicopterY < helicopterYTopBoundary) // hit the top
        {
            // collided top walls
            continueGame = false;
        }
        else if (helicopterY > helicopterYBottomBoundary) // fell out of the bottom
        {
            // collided top walls
            continueGame = false;
        }
    }

    void renderHelicopter() const
    {
        const SDL_Rect helicopterRect = {helicopterX, helicopterY, HELICOPTER_WIDTH, HELICOPTER_HEIGHT};
        SDL_RenderCopy(renderer, helicopterTexture, nullptr, &helicopterRect);
    }

    void resetGame()
    {
        helicopterX = (SCREEN_WIDTH - HELICOPTER_WIDTH) /4;
        helicopterY = SCREEN_HEIGHT - HELICOPTER_HEIGHT - 100;

        continueGame = true;
    }

    bool loadMedia()
    {
        helicopterTexture = IMG_LoadTexture(renderer, HELICOPTER_IMAGE_PATH.c_str());
        if (!helicopterTexture)
        {
            cerr << "IMG_LoadTexture images/helicopter.png error: " << IMG_GetError() << endl;
            return false;
        }

        gameOverTexture = IMG_LoadTexture(renderer, GAME_OVER_IMAGE_PATH.c_str());
        if (!gameOverTexture)
        {
            cerr << "IMG_LoadTexture images/gameover.png error: " << IMG_GetError() << endl;
            return false;
        }

        font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
        if (!font)
        {
            cerr << "TTF_OpenFont error: " << TTF_GetError() << endl;
            return false;
        }

        return true;
    }

    void renderGameOver()
    {
        SDL_Rect gameOverRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
    }

    void renderText(const char* text, SDL_Texture*& texture, SDL_Rect& destRect)
    {
        SDL_Color textColour = {255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColour);
        texture = SDL_CreateTextureFromSurface(renderer, textSurface);

        destRect.w = textSurface->w;
        destRect.h = textSurface->h;

        SDL_FreeSurface(textSurface);

        destRect.x = (SCREEN_WIDTH - destRect.w) / 2;
    }

    void renderPlayAgainText()
    {
        SDL_Rect replayRect;

        replayRect.y = SCREEN_HEIGHT / 2 + 50;

        renderText("Press Spacebar to Play Again", playAgainTexture, replayRect);

        SDL_RenderCopy(renderer, playAgainTexture, nullptr, &replayRect);
    }
};

#endif //HELICOPTER_SDL2_GAME_H
