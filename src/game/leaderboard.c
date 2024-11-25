#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "utils.h"
#include "app.h"
#include "game/player.h"

#ifdef _CRCLONE_DEBUG
    #include "debug/debugmalloc.h"
#endif

/**
 * @brief Létrehozza a dicsőséglistát.
 *
 * Megjegyzés: A lefoglalt memória megfelelő felszabadításához meg kell hívni a `Leaderboard__destroy` függvényt.
 *
 * @param records_file_path A fájl elérési útvonala, ahonnan beolvassa a játékosneveket és rekordokat.
 * @return Pointer az újonnan létrehozott `Leaderboard`-ra.
 */
Leaderboard* Leaderboard__construct(char const* records_file_path)
{
    Leaderboard* new_lboard = malloc(sizeof(Leaderboard));
    new_lboard->records_file_path = strdup_(records_file_path);
    new_lboard->entries = NULL;
    new_lboard->entries_size = 0U;
    
    if(!Leaderboard__load(new_lboard)) {
        return NULL;
    } else {
        return new_lboard;
    }
}

/**
 * @brief Felszabadítja a megadott `Leaderboard` struktúra által lefoglalt memóriát.
 *
 * @param this A `Leaderboard` struktúra, aminek a lefoglalt memóriáját fel kell szabadítani.
 */
void Leaderboard__destroy(Leaderboard* this)
{
    if(this == NULL) return;
    
    free((char*)this->records_file_path);
    free(this->entries);
    free(this);
}

/**
 * @brief Betölti a rekordfájl tartalmát.
*
 * Érdemes meghívni a `Leaderboard__sort`-ot ezután.
 *
 * @param this A `Leaderboard` struktúra, amelyhez már tartozik egy rekordfájl.
 * @return Hibamentesen futott e le.
 */
bool Leaderboard__load(Leaderboard* this)
{
    if(this->records_file_path == NULL) return false;

    FILE* records_f = fopen(this->records_file_path, "r");
    if(records_f == NULL) return false;

    // Beolvasandó adatok.
    char name[(MAX_PLAYER_NAME_LEN + 1) * sizeof(wchar_t)];
    unsigned int record;

    size_t entries_size = 1U;
    this->entries = malloc(entries_size * sizeof(LeaderboardEntry));
    size_t entry_idx = 0U;

    // Addig olvas amíg nem kap EOF-ot vagy nem sikerült 2 adatot beolvasnia és eltárolnia a megfelelő adattípusban
    int read_status; 
    while((read_status = fscanf(records_f, "%s %u", name, &record)) != EOF) {
        if(read_status != 2 && read_status != EOF) {
            // Hibás fájlformátum.
            return false;
        }

        // Az `entries` méretének növelése amíg még van adat.
        if(entry_idx >= entries_size) {
            entries_size *= 2;
            this->entries = realloc(this->entries, entries_size * sizeof(LeaderboardEntry));
        }

        // Rekord beszúrása.
        strcpy(this->entries[entry_idx].name, name);
        this->entries[entry_idx].highscore = record;

        ++entry_idx;
    }
    this->entries_size = entry_idx;

    fclose(records_f);

    return true;
}

/**
 * @brief A `Leaderboard_sort` segédfüggvénye.
 */
static int entry_cmp(void const* e1, void const* e2)
{
    LeaderboardEntry s1 = *(LeaderboardEntry const*)e1;
    LeaderboardEntry s2 = *(LeaderboardEntry const*)e2;

    if (s1.highscore > s2.highscore) return -1;
    if (s1.highscore < s2.highscore) return 1;
    return 0;
}
/**
 * @brief A beolvasott rekordok csökkenő sorba rendezése.
 *
 * @param this A `Leaderboard` struktúra, amely a beolvasott rekordokat tárolja.
 */
void Leaderboard__sort(Leaderboard* this)
{
    qsort(this->entries, this->entries_size, sizeof(LeaderboardEntry), entry_cmp);
}

/**
 * @brief Új rekord beszúrása és mentése.
 *
 * Megjegyzés: A visszatérési érték nem feltétlen van kihasználva, elhanyagolható.
 *
 * @param this A `Leaderboard` struktúra, amihez tartozó rekordfájlba történjen a mentés.
 * @param new A potenciálisan új, rekordot elérő játékos.
 * @return Már tartalmazta e a dicsőséglista a játékost.
 */
bool Leaderboard__insert_new(Leaderboard* this, Player* new)
{
    if(new == NULL) return false;

    // Rekord potenciális frissítése.
    bool found = false;
    for(size_t i = 0U; i < this->entries_size; ++i) {
        if(strcmp(this->entries[i].name, new->name) == 0) {
            found = true;
            if(this->entries[i].highscore < new->score) {
                this->entries[i].highscore = new->score;
            }
            break;
        }
    }

    // Ha nem volt eddig benne, akkor biztosan új rekord.
    if(!found) {
        ++this->entries_size;
        this->entries = realloc(this->entries, this->entries_size * sizeof(LeaderboardEntry));

        strcpy(this->entries[this->entries_size - 1].name, new->name);
        this->entries[this->entries_size - 1].highscore = new->score;
    }

    // A rekordfájl frissítése.
    FILE* records_f = fopen(this->records_file_path, "w");
    if(records_f == NULL) return false;

    for(size_t i = 0U; i < this->entries_size; ++i) {
        fprintf(records_f, "%s %u\n", this->entries[i].name, this->entries[i].highscore);
    }

    fclose(records_f);

    return found;
}

