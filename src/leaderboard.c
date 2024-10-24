#include <stdio.h>
#include <stdlib.h>
#include "player.h"

Leaderboard* Leaderboard__construct(char const* records_file_path)
{
    Leaderboard* new_lboard = malloc(sizeof(Leaderboard));
    FILE* records_f = fopen(records_file_path, "r");
    if(records_f == NULL) return NULL;

    size_t entries = 0U;
    char name[24+1];
    unsigned int record;
    while(fscanf(records_f, "%24s %u", name, &record) != EOF) {
        ++entries;
    }
    //printf("Read entries: %zu.\r\n", entries);

    new_lboard->entries = malloc(entries * sizeof(Stat));
    new_lboard->entries_size = entries;
    rewind(records_f);

    size_t entry_idx = 0U;
    while(fscanf(records_f, "%24s %u", name, &record) != EOF) {
        new_lboard->entries[entry_idx] = (Stat){name, record};
        ++entry_idx;
    }

    fclose(records_f);

    return new_lboard;
}

int entry_cmp(void const* e1, void const* e2)
{
    Stat s1 = *(Stat*)e1;
    Stat s2 = *(Stat*)e2;
 
    if (s1.highscore < s2.highscore) return -1;
    if (s1.highscore > s2.highscore) return 1;
    return 0;
}

void Leaderboard__sort(Leaderboard* this)
{
    printf("Read %zu records.\n", this->entries_size);
    qsort(this->entries, this->entries_size, sizeof(Stat), entry_cmp);

    for(size_t i = 0U; i < this->entries_size; ++i) {
        printf("%zu: %s %u\n\r", i, this->entries[i].name, this->entries[i].highscore);
    }
}

void Leaderboard__destroy(Leaderboard* this)
{
    free(this->entries);
}
