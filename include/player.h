#ifndef CRCLONE_PLAYER_H
#define CRCLONE_PLAYER_H

#include <SDL2/SDL_render.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    char name[24+1];
    unsigned int highscore;
} Stat;

typedef struct {
    Stat stat;
    unsigned int matches_count;

    // Csak ha épp játékban van
    unsigned int score;
    bool is_turn_over;

    // Textúrák
    SDL_Texture* handle;
    SDL_Texture* score_counter;
    SDL_Texture* turn_indicator;
} Player;

typedef struct {
    Stat* entries;
    size_t entries_size;
} Leaderboard;

Leaderboard* Leaderboard__construct(char const*);
void Leaderboard__sort(Leaderboard*);
void Leaderboard__destroy(Leaderboard*);

#endif
