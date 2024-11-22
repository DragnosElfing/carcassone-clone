#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "game/player.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

Leaderboard* Leaderboard__construct(char const* records_file_path)
{
    Leaderboard* new_lboard = malloc(sizeof(Leaderboard));
    if(!Leaderboard__load(new_lboard, records_file_path)) {
        return NULL;
    } else {
        return new_lboard;
    }
}

void Leaderboard__destroy(Leaderboard* this)
{
    free(this->entries);
    free(this);
}

bool Leaderboard__load(Leaderboard* this, char const* records_file_path)
{
    FILE* records_f = fopen(records_file_path, "r");
    if(records_f == NULL) return false;

    char name[24+1];
    unsigned int record;
    size_t entries_size = 1U;
    this->entries = malloc(entries_size * sizeof(LeaderboardEntry));
    size_t entry_idx = 0U;
    int read_status; 
    while((read_status = fscanf(records_f, "%24s %u", name, &record)) != EOF) {
        if(read_status != 2 && read_status != EOF) {
            // hibás fájlformátum
            return false;
        }

        if(entry_idx >= entries_size) {
            entries_size *= 2;
            this->entries = realloc(this->entries, entries_size * sizeof(LeaderboardEntry));
        }
        strcpy(this->entries[entry_idx].name, name);
        this->entries[entry_idx].highscore = record;
        ++entry_idx;
    }
    this->entries_size = entry_idx;

    fclose(records_f);

    return true;
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

bool Leaderboard__insert_new(Leaderboard* this, Player* new)
{
    if(new == NULL) return false;

    bool update = false;
    for(size_t i = 0U; i < this->entries_size; ++i) {
        DBG_LOG("Checking %s...", new->name);
        if(strcmp(this->entries[i].name, new->name) == 0) {
            update = true;
            if(this->entries[i].highscore < new->score) {
                DBG_LOG("Changed %u -> %u for %s", this->entries[i].highscore, new->score, new->name);
                this->entries[i].highscore = new->score;
            }
            break;
        }
    }

    if(!update) {
        ++this->entries_size;
        this->entries = realloc(this->entries, this->entries_size * sizeof(LeaderboardEntry));
        strcpy(this->entries[this->entries_size - 1].name, new->name);
        this->entries[this->entries_size - 1].highscore = new->score;
    }

    FILE* records_f = fopen("res/data/records.dat", "w");
    if(records_f == NULL) return NULL;
    for(size_t i = 0U; i < this->entries_size; ++i) {
        fprintf(records_f, "%s %u\n", this->entries[i].name, this->entries[i].highscore);
    }
    fclose(records_f);

    return update;
}

