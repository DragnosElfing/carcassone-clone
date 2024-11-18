#ifndef CRCLONE_PLAYER_H
#define CRCLONE_PLAYER_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "game/meeple.h"

#define MAX_MEEPLES 7

typedef struct {
    char name[(24+1)*sizeof(wchar_t)];
    unsigned int score;
    unsigned int highscore;
    bool has_placed_card;
    bool has_placed_meeple;
    bool is_turn_active;

    Meeple meeples[MAX_MEEPLES];
    size_t meeples_at_hand;

    // Textúrák
    SDL_Texture* score_counter;
    SDL_Texture* stat_panel;
} Player;
Player Player__construct(SDL_Renderer*, TTF_Font*, char*, unsigned int);
void Player__place_meeple(Player*, SDL_Point);
void Player__reclaim_meeple(Player*, Meeple*);
void Player__toggle_turn_active(Player*);
void Player__add_to_score(Player*, unsigned int);
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
unsigned int Leaderboard__get_highscore_for(Leaderboard*, char const*);

#endif
