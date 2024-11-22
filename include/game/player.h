#ifndef CRCLONE_PLAYER_H
#define CRCLONE_PLAYER_H

#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/meeple.h"

// Maximum alattvalók száma per játékos
#define MAX_MEEPLES 7
// A játékos nevének max hossza (nulltermináló nélkül)
#define MAX_PLAYER_NAME_LEN 24

typedef struct {
    // Általános adatok
    // UTF-8 support: sajnos a TTF_RenderUTF8_Blended valójában char*-t fogad el, ezért nem tárolhatom wchar_t*-ben.
    char name[(MAX_PLAYER_NAME_LEN + 1) * sizeof(wchar_t)];
    unsigned int score;

    // Körre vonatkozó
    bool has_placed_card;

    // Alattvalók
    Meeple meeples[MAX_MEEPLES];
    size_t meeples_at_hand;

    // Textúrák
    SDL_Texture* score_counter;
    SDL_Texture* handle_texture;
    SDL_Texture* stat_panel;
    bool update_score;
} Player;
Player Player__construct(SDL_Renderer*, TTF_Font*, char*);
void Player__place_meeple(Player*, SDL_Point);
void Player__reclaim_meeple(Player*, Meeple*);
void Player__render(Player*, SDL_Renderer*, TTF_Font*);
void Player__toggle_turn_active(Player*);
void Player__add_to_score(Player*, SDL_Renderer*, TTF_Font*, unsigned int);
void Player__destroy(Player*);

// Dicsőséglista rekord
typedef struct {
    char name[MAX_PLAYER_NAME_LEN + 1];
    unsigned int highscore;
} LeaderboardEntry;

typedef struct {
    // TODO: save file name dyn
    LeaderboardEntry* entries;
    size_t entries_size;
} Leaderboard;
Leaderboard* Leaderboard__construct(char const*);
void Leaderboard__destroy(Leaderboard*);
void Leaderboard__sort(Leaderboard*);
bool Leaderboard__load(Leaderboard*, char const*);
unsigned int Leaderboard__get_highscore_for(Leaderboard*, char const*);
bool Leaderboard__insert_new(Leaderboard*, Player*);

#endif
