#ifndef CRCLONE_PLAYER_H
#define CRCLONE_PLAYER_H

#include <SDL2/SDL_render.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "game/meeple.h"

#define MAX_MEEPLES 7

typedef struct {
    char name[24+1];
    unsigned int score;
    unsigned int highscore;

    Meeple meeples[MAX_MEEPLES];

    // Textúrák
    SDL_Texture* handle; // ? not needed
    SDL_Texture* score_counter;
    SDL_Texture* turn_indicator;
    SDL_Texture* stat_panel;
} Player;
Player* Player__construct(SDL_Renderer*, char*, unsigned int);
void Player__toggle_turn_active(Player*);
void Player__update_score(Player*, unsigned int);
void Player__destroy(Player*);

typedef struct {
    char name[24+1];
    unsigned int highscore;
} LeaderboardEntry;

typedef struct {
    LeaderboardEntry* entries;
    size_t entries_size;
} Leaderboard;
Leaderboard* Leaderboard__construct(char const*);
void Leaderboard__destroy(Leaderboard*);
void Leaderboard__sort(Leaderboard*);
unsigned int Leaderboard__get_highscore_for(Leaderboard*, Player*);

#endif
