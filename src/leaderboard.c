#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game/player.h"

Leaderboard* Leaderboard__construct(char const* records_file_path)
{
    Leaderboard* new_lboard = malloc(sizeof(Leaderboard));
    FILE* records_f = fopen(records_file_path, "r");
    if(records_f == NULL) return NULL;

    char name[24+1];
    unsigned int record;
    size_t entries_size = 1U;
    new_lboard->entries = malloc(entries_size * sizeof(LeaderboardEntry));
    size_t entry_idx = 0U;
    int read_status; 
    while((read_status = fscanf(records_f, "%24s %u", name, &record)) != EOF) {
        if(read_status != 2 && read_status != EOF) {
            // hibás fájlformátum
            return NULL;
        }

        if(entry_idx >= entries_size) {
            entries_size *= 2;
            new_lboard->entries = realloc(new_lboard->entries, entries_size * sizeof(LeaderboardEntry));
        }
        strcpy(new_lboard->entries[entry_idx].name, name);
        new_lboard->entries[entry_idx].highscore = record;
        ++entry_idx;
    }
    new_lboard->entries_size = entry_idx;

    fclose(records_f);

    return new_lboard;
}

static int entry_cmp(void const* e1, void const* e2)
{
    LeaderboardEntry s1 = *(LeaderboardEntry const*)e1;
    LeaderboardEntry s2 = *(LeaderboardEntry const*)e2;

    if (s1.highscore > s2.highscore) return -1;
    if (s1.highscore < s2.highscore) return 1;
    return 0;
}

void Leaderboard__sort(Leaderboard* this)
{
    qsort(this->entries, this->entries_size, sizeof(LeaderboardEntry), entry_cmp);
}

unsigned int Leaderboard__get_highscore_for(Leaderboard* this, char const* name)
{
    for(size_t i = 0U; i < this->entries_size; ++i) {
        if(strcmp(this->entries[i].name, name) == 0) {
            return this->entries[i].highscore;
        }
    }

    return 0U;
}

void Leaderboard__destroy(Leaderboard* this)
{
    free(this->entries);
}
