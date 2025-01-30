#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITY 0.8f
#define JUMP_STRENGTH -10.0f
#define PIPE_WIDTH 50
#define MIN_GAP 100
#define MAX_GAP 250
#define PIPE_SPEED 4
#define CEILING_HEIGHT 1  // Высота потолка
#define CEILING_WIDTH 600  // Ширина потолка
#define GROUND_HEIGHT 50   // Высота пола

typedef struct {
    int x, height;
    bool passed;
} Pipe;

typedef struct {
    float x, y;
    float velocity;
} Bird;

void handle_events(bool *running, Bird *bird, bool *game_over, bool *restart_game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_SPACE) {
                if (*game_over) {
                    *restart_game = true; // Устанавливаем флаг перезапуска
                }
                bird->velocity = JUMP_STRENGTH;
            }
        }
    }
}

void update_bird(Bird *bird) {
    bird->velocity += GRAVITY;
    bird->y += bird->velocity;

    // Проверяем столкновение с потолком
    if (bird->y < CEILING_HEIGHT) {
        bird->y = CEILING_HEIGHT;
        bird->velocity = 0;
    }

    // Проверяем столкновение с полом
    if (bird->y > SCREEN_HEIGHT - GROUND_HEIGHT - 20) {
        bird->y = SCREEN_HEIGHT - GROUND_HEIGHT - 20; // Устанавливаем птичку на пол
        bird->velocity = 0;
    }
}

void update_pipes(Pipe *top_pipe, Pipe *bottom_pipe, int *score) {
    // Обновление позиции труб
    top_pipe->x -= PIPE_SPEED;
    bottom_pipe->x -= PIPE_SPEED;

    // Генерация новых труб, если старые ушли за экран
    if (top_pipe->x + PIPE_WIDTH < 0) {
        int gap = rand() % (MAX_GAP - MIN_GAP) + MIN_GAP; // Рандомный зазор между трубами
        top_pipe->height = rand() % (SCREEN_HEIGHT - gap); // Высота верхней трубы
        bottom_pipe->height = top_pipe->height + gap; // Нижняя труба отодвигается на зазор

        // Устанавливаем новые позиции для труб
        top_pipe->x = SCREEN_WIDTH;
        bottom_pipe->x = SCREEN_WIDTH;

        top_pipe->passed = false;
        bottom_pipe->passed = false;
    }

    // Увеличение счета, если птичка прошла через препятствия
    if (!top_pipe->passed && top_pipe->x < 100) {
        top_pipe->passed = true;
        (*score)++;
    }
}

bool check_collision(Bird *bird, Pipe *top_pipe, Pipe *bottom_pipe) {
    if (bird->x + 20 > top_pipe->x && bird->x < top_pipe->x + PIPE_WIDTH) {
        if (bird->y < top_pipe->height || bird->y + 20 > bottom_pipe->height) {
            return true;
        }
    }
    return false;
}

void render(SDL_Renderer *renderer, Bird *bird, Pipe *top_pipe, Pipe *bottom_pipe, int score, TTF_Font *font, bool game_over) {
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_RenderClear(renderer);

    // Отображаем потолок выше экрана
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Красный цвет для потолка
    SDL_Rect ceiling_rect = {(SCREEN_WIDTH - CEILING_WIDTH) / 2, -CEILING_HEIGHT, CEILING_WIDTH, CEILING_HEIGHT};
    SDL_RenderFillRect(renderer, &ceiling_rect);

    // Отображаем пол
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);  // Коричневый цвет для пола
    SDL_Rect ground_rect = {0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT};
    SDL_RenderFillRect(renderer, &ground_rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect bird_rect = { (int)bird->x, (int)bird->y, 20, 20 };
    SDL_RenderFillRect(renderer, &bird_rect);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    
    // Отображаем верхнюю трубу
    SDL_Rect top_pipe_rect = { top_pipe->x, 0, PIPE_WIDTH, top_pipe->height };
    SDL_RenderFillRect(renderer, &top_pipe_rect);
    
    // Отображаем нижнюю трубу
    SDL_Rect bottom_pipe_rect = { bottom_pipe->x, bottom_pipe->height, PIPE_WIDTH, SCREEN_HEIGHT - bottom_pipe->height - GROUND_HEIGHT };
    SDL_RenderFillRect(renderer, &bottom_pipe_rect);

    // Отображение счета
    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    SDL_Surface *surface = TTF_RenderText_Solid(font, scoreText, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect score_rect = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &score_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Если игра окончена, отображаем сообщение о конце игры
    if (game_over) {
        SDL_Surface *game_over_surface = TTF_RenderText_Solid(font, "Game Over! Press SPACE to restart.", (SDL_Color){255, 0, 0, 255});
        SDL_Texture *game_over_texture = SDL_CreateTextureFromSurface(renderer, game_over_surface);
        SDL_Rect game_over_rect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, game_over_surface->w, game_over_surface->h };
        SDL_RenderCopy(renderer, game_over_texture, NULL, &game_over_rect);
        SDL_FreeSurface(game_over_surface);
        SDL_DestroyTexture(game_over_texture);
    }

    SDL_RenderPresent(renderer);
}

void reset_game(Bird *bird, Pipe *top_pipe, Pipe *bottom_pipe, int *score) {
    bird->x = 100;
    bird->y = SCREEN_HEIGHT / 2;
    bird->velocity = 0;
    *score = 0;

    top_pipe->x = SCREEN_WIDTH;
    top_pipe->height = rand() % (SCREEN_HEIGHT - MAX_GAP);
    bottom_pipe->x = SCREEN_WIDTH;
    bottom_pipe->height = top_pipe->height + (rand() % (MAX_GAP - MIN_GAP) + MIN_GAP);
    top_pipe->passed = false;
    bottom_pipe->passed = false;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    Bird bird = {100, SCREEN_HEIGHT / 2, 0};
    Pipe top_pipe = {SCREEN_WIDTH, rand() % (SCREEN_HEIGHT - MAX_GAP), false};
    Pipe bottom_pipe = {SCREEN_WIDTH, top_pipe.height + (rand() % (MAX_GAP - MIN_GAP) + MIN_GAP), false};
    int score = 0;
    bool game_over = false;
    bool restart_game = false;

    bool running = true;
    while (running) {
        handle_events(&running, &bird, &game_over, &restart_game);

        if (game_over && restart_game) {
            reset_game(&bird, &top_pipe, &bottom_pipe, &score);
            game_over = false;
            restart_game = false; // Сбрасываем флаг перезапуска
        }

        if (!game_over) {
            update_bird(&bird);
            update_pipes(&top_pipe, &bottom_pipe, &score);
            if (check_collision(&bird, &top_pipe, &bottom_pipe)) {
                game_over = true;
            }
        }

        render(renderer, &bird, &top_pipe, &bottom_pipe, score, font, game_over);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    return 0;
}
