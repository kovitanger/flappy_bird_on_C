#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITY 0.8f
#define JUMP_STRENGTH -10.0f
#define PIPE_WIDTH 50
#define PIPE_GAP 150
#define HORIZONTAL_PIPE_SPACING 200
#define PIPE_SPAWN_INTERVAL (PIPE_WIDTH + HORIZONTAL_PIPE_SPACING)
#define PIPE_SPEED 4
#define CEILING_HEIGHT 1  
#define GROUND_HEIGHT 50    
#define PIPE_COUNT 10

typedef struct {
    int x, height;
    bool passed;
} PipePair;

typedef struct {
    float x, y;
    float velocity;
} Bird;

SDL_Texture *load_texture(const char *file, SDL_Renderer *renderer) {
    SDL_Texture *texture = NULL;
    SDL_Surface *surface = IMG_Load(file);
    if (!surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void handle_events(bool *running, Bird *bird, bool *game_over, bool *restart_game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_SPACE) {
                if (*game_over) {
                    *restart_game = true;
                }
                bird->velocity = JUMP_STRENGTH;
            }
        }
    }
}

void update_bird(Bird *bird) {
    bird->velocity += GRAVITY;
    bird->y += bird->velocity;

    if (bird->y < CEILING_HEIGHT) {
        bird->y = CEILING_HEIGHT;
        bird->velocity = 0;
    }

    if (bird->y > SCREEN_HEIGHT - GROUND_HEIGHT - 24) { // Учитываем высоту птицы
        bird->y = SCREEN_HEIGHT - GROUND_HEIGHT - 24;
        bird->velocity = 0;
    }
}

void update_pipes(PipePair *pipes, int *score) {
    for (int i = 0; i < PIPE_COUNT; i++) {
        pipes[i].x -= PIPE_SPEED;

        if (pipes[i].x + PIPE_WIDTH < 0) {
            int max_x = 0;
            for (int j = 0; j < PIPE_COUNT; j++) {
                if (pipes[j].x > max_x) max_x = pipes[j].x;
            }
            pipes[i].x = max_x + PIPE_SPAWN_INTERVAL;
            pipes[i].height = rand() % (SCREEN_HEIGHT - PIPE_GAP - GROUND_HEIGHT - CEILING_HEIGHT) + CEILING_HEIGHT;
            pipes[i].passed = false;
        }

        if (!pipes[i].passed && pipes[i].x < 100) {
            pipes[i].passed = true;
            (*score)++;
        }
    }
}

bool check_collision(Bird *bird, PipePair *pipes) {
    SDL_Rect bird_rect = {(int)bird->x, (int)bird->y, 34, 24}; // Размеры птицы
    for (int i = 0; i < PIPE_COUNT; i++) {
        SDL_Rect pipe_rect_top = {pipes[i].x, 0, PIPE_WIDTH, pipes[i].height};
        SDL_Rect pipe_rect_bottom = {pipes[i].x, pipes[i].height + PIPE_GAP, PIPE_WIDTH, SCREEN_HEIGHT - pipes[i].height - PIPE_GAP - GROUND_HEIGHT};
        
        SDL_Rect intersection;
        if (SDL_IntersectRect(&bird_rect, &pipe_rect_top, &intersection) ||
            SDL_IntersectRect(&bird_rect, &pipe_rect_bottom, &intersection)) {
            return true;
        }
    }
    return false;
}

void render(SDL_Renderer *renderer, Bird *bird, PipePair *pipes, int score, TTF_Font *font, bool game_over, SDL_Texture *base_texture, SDL_Texture *bird_texture) {
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_RenderClear(renderer);

    // Потолок
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect ceiling_rect = {0, -CEILING_HEIGHT, SCREEN_WIDTH, CEILING_HEIGHT};
    SDL_RenderFillRect(renderer, &ceiling_rect);

    // Земля
    SDL_Rect base_rect = {0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT};
    SDL_RenderCopy(renderer, base_texture, NULL, &base_rect);

    // Птица
    SDL_Rect bird_rect = {(int)bird->x, (int)bird->y, 34, 24}; // Размеры текстуры птицы
    SDL_RenderCopy(renderer, bird_texture, NULL, &bird_rect);

    // Трубы
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < PIPE_COUNT; i++) {
        SDL_Rect top_pipe_rect = {pipes[i].x, 0, PIPE_WIDTH, pipes[i].height};
        SDL_RenderFillRect(renderer, &top_pipe_rect);

        SDL_Rect bottom_pipe_rect = {pipes[i].x, pipes[i].height + PIPE_GAP, PIPE_WIDTH, SCREEN_HEIGHT - pipes[i].height - PIPE_GAP - GROUND_HEIGHT};
        SDL_RenderFillRect(renderer, &bottom_pipe_rect);
    }

    // Счет
    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    SDL_Surface *surface = TTF_RenderText_Solid(font, scoreText, (SDL_Color){255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect score_rect = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &score_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Game Over
    if (game_over) {
        SDL_Surface *game_over_surface = TTF_RenderText_Solid(font, "Game Over! Press SPACE to restart.", (SDL_Color){255, 0, 0, 255});
        SDL_Texture *game_over_texture = SDL_CreateTextureFromSurface(renderer, game_over_surface);
        SDL_Rect game_over_rect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, game_over_surface->w, game_over_surface->h};
        SDL_RenderCopy(renderer, game_over_texture, NULL, &game_over_rect);
        SDL_FreeSurface(game_over_surface);
        SDL_DestroyTexture(game_over_texture);
    }

    SDL_RenderPresent(renderer);
}

void reset_game(Bird *bird, PipePair *pipes, int *score) {
    bird->x = 100;
    bird->y = SCREEN_HEIGHT / 2;
    bird->velocity = 0;
    *score = 0;

    for (int i = 0; i < PIPE_COUNT; i++) {
        pipes[i].x = SCREEN_WIDTH + i * PIPE_SPAWN_INTERVAL;
        pipes[i].height = rand() % (SCREEN_HEIGHT - PIPE_GAP - GROUND_HEIGHT - CEILING_HEIGHT) + CEILING_HEIGHT;
        pipes[i].passed = false;
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font *font = TTF_OpenFont("/home/kovitang/Документы/flappy_bird/assets/arial.ttf", 24);
    SDL_Texture *base_texture = load_texture("/home/kovitang/Документы/flappy_bird/assets/base.png", renderer);
    SDL_Texture *bird_texture = load_texture("/home/kovitang/Документы/flappy_bird/assets/bird.png", renderer);

    if (!font || !base_texture || !bird_texture) {
        printf("Failed to load resources!\n");
        return 1;
    }

    Bird bird = {100, SCREEN_HEIGHT / 2, 0};
    PipePair pipes[PIPE_COUNT];
    reset_game(&bird, pipes, &(int){0});
    int score = 0;
    bool game_over = false;
    bool restart_game = false;
    bool running = true;

    while (running) {
        handle_events(&running, &bird, &game_over, &restart_game);

        if (game_over && restart_game) {
            reset_game(&bird, pipes, &score);
            game_over = false;
            restart_game = false;
        }

        if (!game_over) {
            update_bird(&bird);
            update_pipes(pipes, &score);
            if (check_collision(&bird, pipes)) {
                game_over = true;
            }
        }

        render(renderer, &bird, pipes, score, font, game_over, base_texture, bird_texture);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bird_texture);
    SDL_DestroyTexture(base_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}