#include "pokemon.h"
#include "tsv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Pokemon *items;
    int count;
    int capacity;
    const Move *moves;
    int move_count;
} PokemonBuffer;

/* --- Technical buffer helper --- */

static int append_pokemon(PokemonBuffer *buf, const Pokemon *pokemon) {
    int new_cap = 0;
    Pokemon *tmp = NULL;

    if (buf->count == buf->capacity) {
        new_cap = buf->capacity == 0 ? 16 : buf->capacity * 2;
        tmp = realloc(buf->items, (size_t)new_cap * sizeof(Pokemon));
        if (!tmp) {
            return -1;
        }
        buf->items = tmp;
        buf->capacity = new_cap;
    }
    buf->items[buf->count++] = *pokemon;
    return 0;
}

/* --- Business logic --- */

static int in_range(int val, int min, int max) {
    return val >= min && val <= max;
}

static int has_pokemon_name(const PokemonBuffer *buf, const char *name) {
    for (int i = 0; i < buf->count; i++) {
        if (strcmp(buf->items[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

static const Move *find_move(const Move *moves, int move_count, const char *name) {
    for (int i = 0; i < move_count; i++) {
        if (strcmp(moves[i].name, name) == 0) {
            return &moves[i];
        }
    }
    return NULL;
}

static int is_valid_pokemon(const Pokemon *p) {
    if (p->name[0] == '\0') {
        return 0;
    }
    return in_range(p->hp, 0, 100) && in_range(p->attack, 0, 100) &&
           in_range(p->defense, 0, 100) && in_range(p->speed, 0, 100);
}

static int parse_pokemon_stats(char **fields, Pokemon *out) {
    snprintf(out->name, sizeof(out->name), "%s", fields[0]);
    if (tsv_parse_int(fields[1], &out->hp) != 0 ||
        tsv_parse_int(fields[2], &out->attack) != 0 ||
        tsv_parse_int(fields[3], &out->defense) != 0 ||
        tsv_parse_int(fields[4], &out->speed) != 0) {
        return -1;
    }
    out->max_hp = out->hp;
    return type_from_string(fields[5], &out->type);
}

static int parse_pokemon_moves(char *field, const Move *moves, int move_count,
                               Pokemon *out) {
    char *cursor = field;
    char *comma = NULL;
    char *name = NULL;
    int slot = 0;
    const Move *found = NULL;

        out->move_count = 0;
    while (cursor) {
        comma = strchr(cursor, ',');
        if (comma) {
            *comma = '\0';
        }

        name = tsv_trim(cursor);
        if (*name == '\0' || slot >= 4) {
            return -1;
        }
        slot++;

        if (strcmp(name, "-") != 0) {
            found = find_move(moves, move_count, name);
            if (!found) {
                return -1;
            }
            out->moves[out->move_count++] = *found;
        }
        cursor = comma ? comma + 1 : NULL;
    }
    return 0;
}

static int add_pokemon_row(int field_count, char **fields, void *userdata) {
    if (field_count < 7) {
        return -1;
    }

    PokemonBuffer *buf = userdata;
    Pokemon p;
    if (parse_pokemon_stats(fields, &p) != 0 || !is_valid_pokemon(&p)) {
        return -1;
    }
    if (has_pokemon_name(buf, p.name)) {
        return -1;
    }
    if (parse_pokemon_moves(fields[6], buf->moves, buf->move_count, &p) != 0) {
        return -1;
    }
    return append_pokemon(buf, &p);
}

Pokemon *load_pokemons(const char *path, const Move *moves, int move_count, int *out_count) {
    if (out_count) {
        *out_count = 0;
    }
    if (!path || !moves || move_count <= 0) {
        return NULL;
    }

    PokemonBuffer buf = {NULL, 0, 0, moves, move_count};
    if (tsv_parse_file(path, add_pokemon_row, &buf) < 0) {
        free(buf.items);
        return NULL;
    }

    if (out_count) {
        *out_count = buf.count;
    }
    return buf.items;
}

void free_pokemons(Pokemon *pokemons) {
    free(pokemons);
}
