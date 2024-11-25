#ifndef CRCLONE_PLAYER_H
#define CRCLONE_PLAYER_H

#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game/meeple.h"

/*! Maximum alattvalók száma per játékos. */
#define MAX_MEEPLES 7
/*! Egy játékos nevének max hossza (multibyte és nulltermináló nélkül). */
#define MAX_PLAYER_NAME_LEN 24

typedef struct {
    // Általános adatok.
    // UTF-8 support: sajnos a TTF_RenderUTF8_Blended valójában char*-t fogad el, ezért nem tárolhatom wchar_t*-ben.
    char name[(MAX_PLAYER_NAME_LEN + 1) * sizeof(wchar_t)];
    unsigned int score;

    // Tett e már le kártyát az adott körben.
    bool has_placed_card;

    // Alattvalók.
    Meeple meeples[MAX_MEEPLES];
    size_t meeples_at_hand;

    // Textúrák.
    SDL_Texture* score_counter;
    SDL_Texture* handle_texture;
    SDL_Texture* stat_panel;
    SDL_Texture* own_meeple_texture;

    // Kell e frissíteni a pontot.
    bool update_score;
} Player;
Player Player__construct(SDL_Renderer*, TTF_Font*, char*, char const*);
void Player__place_meeple(Player*, SDL_Point);
void Player__reclaim_meeple(Player*, Meeple*);
void Player__render(Player*, SDL_Renderer*, TTF_Font*);
void Player__add_to_score(Player*, unsigned int);
void Player__destroy(Player*);

/*! Dicsőséglista rekord. */
typedef struct {
    // Játékos neve.
    char name[(MAX_PLAYER_NAME_LEN + 1) * sizeof(wchar_t)];

    // Játékos rekordja.
    unsigned int highscore;
} LeaderboardEntry;

/*! Dicsőséglista. */
typedef struct {
    // A rekordfájl elérési útvonala.
    char const* records_file_path;

    // A rekordok.
    LeaderboardEntry* entries;
    size_t entries_size;
} Leaderboard;
Leaderboard* Leaderboard__construct(char const*);
void Leaderboard__destroy(Leaderboard*);
void Leaderboard__sort(Leaderboard*);
bool Leaderboard__load(Leaderboard*);
bool Leaderboard__insert_new(Leaderboard*, Player*);

#endif
