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

constexpr int SCREEN_WIDTH = 600;
constexpr int SCREEN_HEIGHT = 400;
inline auto SPRITES_FOLDER = "images/";

struct Helicopter
{
    const float x = 100.0f;
    float y = 200.0f;
    const float width = 50.0f;
    const float height = 30.0f;

    const float TOP_BOUNDARY = 0.0f;
    const float BOTTOM_BOUNDARY = SCREEN_HEIGHT - height;

    const float VERTICAL_SPEED = 2.0f;
    const string HELICOPTER_IMAGE_PATH = SPRITES_FOLDER + string("helicopter2.png");

    [[nodiscard]] bool collidedWithWalls() const
    {
        return y < TOP_BOUNDARY || y > BOTTOM_BOUNDARY;
    }
};

const string IMAGE1_PATH = SPRITES_FOLDER + string("cacti-big.png");
const string IMAGE2_PATH = SPRITES_FOLDER + string("cacti-small.png");

struct Obstacle
{
    float x, y, width, height, speed;
    SDL_Texture* texture;
};

class Game
{
public:
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

        if (!loadSDLImageMedia())
        {
            SDL_Quit();
            return false;
        }

        for (int i = 0; i < maxObstacles; i++)
            obstacles.push_back(createObstacle());

        continuePlaying = true;
        continueGame = true;

        return true;
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
        if (currentKeyStates[SDL_SCANCODE_UP]) // up arrow key
        {
            helicopter.y -= helicopter.VERTICAL_SPEED;
        }
    }

    void update()
    {
        helicopter.y += GRAVITY;

        for (Obstacle& obstacle : obstacles)
        {
            obstacle.x -= obstacle.speed;
            if (obstacle.x + obstacle.width < 0)
            {
                obstacle.x = SCREEN_WIDTH;
            }
        }

        if (helicopter.collidedWithWalls() || detectHelicopterCollisionWithObstacles())
        {
            continueGame = false;
        }
    }

    void render()
    {
        SDL_RenderClear(renderer); // clear the renderer to the draw color

        if (continueGame)
        {
            renderHelicopter();
            renderObstacles();
        }
        else
        {
            SDL_Delay(500);
            renderGameOver();
            renderPlayAgainText();
        }

        SDL_RenderPresent(renderer); // draw to the screen
    }

    void clean() const
    {
        SDL_DestroyTexture(helicopterTexture);

        SDL_DestroyTexture(obstacle1Texture);
        SDL_DestroyTexture(obstacle2Texture);

        SDL_DestroyTexture(gameOverTexture);
        SDL_DestroyTexture(playAgainTexture);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();
    }

    // a function to access the private running variable
    [[nodiscard]] bool playing() const { return continuePlaying; }
    [[nodiscard]] bool gameOn() const { return continueGame; }

private:
    const char* FONT_PATH = "images/consolas.ttf";
    const int FONT_SIZE = 32;

    const string GAME_OVER_IMAGE_PATH = SPRITES_FOLDER + string("gameover.png");

    const float GRAVITY = 0.5f;

    const float helicopterYTopBoundary = 0.0f;
    const float helicopterYBottomBoundary = SCREEN_HEIGHT - 30.0f;

    Helicopter helicopter = {};

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Texture* helicopterTexture = nullptr;
    SDL_Texture* obstacle1Texture = nullptr;
    SDL_Texture* obstacle2Texture = nullptr;
    SDL_Texture* gameOverTexture = nullptr;
    SDL_Texture* playAgainTexture = nullptr;

    TTF_Font* font = nullptr;

    bool continuePlaying = false;
    bool continueGame = false;

    const int maxObstacles = 5;
    vector<Obstacle> obstacles;

    static bool checkCollision(const SDL_FRect& rect1, const SDL_FRect& rect2)
    {
        return SDL_HasIntersectionF(&rect1, &rect2);
    }

    bool detectHelicopterCollisionWithObstacles() const
    {
        const SDL_FRect hRect = {helicopter.x, helicopter.y, helicopter.width, helicopter.height};
        for (const Obstacle& obstacle : obstacles)
        {
            if (obstacle.x > helicopter.x || obstacle.x + obstacle.width > helicopter.x)
            {
                const SDL_FRect oRect = {obstacle.x, obstacle.y, obstacle.width, obstacle.height};
                if (checkCollision(hRect, oRect))
                    return true;
            }
        }

        return false;
    }

    void renderHelicopter() const
    {
        const SDL_FRect helicopterRect = {helicopter.x, helicopter.y, helicopter.width, helicopter.height};
        SDL_RenderCopyF(renderer, helicopterTexture, nullptr, &helicopterRect);
    }

    void renderObstacles()
    {
        for (Obstacle& obstacle : obstacles)
        {
            renderObstacle(obstacle);
        }
    }

    void renderObstacle(const Obstacle& obstacle) const
    {
        const SDL_FRect obstacleRect = {obstacle.x, obstacle.y, obstacle.width, obstacle.height};
        SDL_RenderCopyF(renderer, obstacle.texture, nullptr, &obstacleRect);
    }

    void resetGame()
    {
        helicopter.y = 200.0f;

        for (int i = 0; i < maxObstacles; i++)
        {
            obstacles[i].y = rand() % static_cast<int>(SCREEN_HEIGHT - obstacles[i].height);
            obstacles[i].speed = static_cast<float>(rand() % 5 + 1);
            obstacles[i].texture = rand() % 2 ? obstacle1Texture : obstacle2Texture;
        }

        continueGame = true;
    }

    bool loadSDLImageMedia()
    {
        helicopterTexture = IMG_LoadTexture(renderer, helicopter.HELICOPTER_IMAGE_PATH.c_str());
        if (!helicopterTexture)
        {
            cerr << "IMG_LoadTexture images/helicopter2.png error: " << IMG_GetError() << endl;
            return false;
        }

        obstacle1Texture = IMG_LoadTexture(renderer, IMAGE1_PATH.c_str());
        if (!obstacle1Texture)
        {
            cerr << "IMG_LoadTexture images/cactus-big.png error: " << IMG_GetError() << endl;
            return false;
        }

        obstacle2Texture = IMG_LoadTexture(renderer, IMAGE2_PATH.c_str());
        if (!obstacle2Texture)
        {
            cerr << "IMG_LoadTexture images/cactus-small.png error: " << IMG_GetError() << endl;
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

    void renderGameOver() const
    {
        SDL_Rect gameOverRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
    }

    void renderText(const char* text, SDL_Texture*& texture, SDL_Rect& destRect) const
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

    [[nodiscard]] Obstacle createObstacle() const
    {
        Obstacle obstacle;

        obstacle.width = 20;
        obstacle.height = 20;
        obstacle.x = SCREEN_WIDTH;
        obstacle.y = rand() % static_cast<int>(SCREEN_HEIGHT - obstacle.height);

        obstacle.speed = static_cast<float>(rand() % 3 + 1);

        obstacle.texture = rand() % 2 ? obstacle1Texture : obstacle2Texture;

        return obstacle;
    }
};

#endif //HELICOPTER_SDL2_GAME_H
