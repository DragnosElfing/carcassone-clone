#include <stdio.h>
#include "player.h"

Leaderboard Leaderboard__construct(char const* records_file_path)
{
    Leaderboard new_lboard = {0};
    FILE* records_f = fopen(records_file_path, "r");
    if(records_f == NULL) return new_lboard;

    size_t entries = 0U;
    char name[24+1];
    unsigned int record;
    while(fscanf(records_f, "%24s %u", name, &record) != EOF) {
        ++entries;
    }
    printf("Read entries: %zu.\r\n", entries);

    new_lboard.entries = malloc(entries * sizeof(Player));
    rewind(records_f);

    free(new_lboard.entries);

    fclose(records_f);

    return new_lboard;
}
