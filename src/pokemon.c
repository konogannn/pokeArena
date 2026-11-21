#include "pokemon.h"
#include "tsv.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    Pokemon *items;
    int count;
    int capacity;
    const Move *moves;
    int move_count;
} PokemonBuffer;

static int append_pokemon(PokemonBuffer *buf, const Pokemon *pokemon)
{
    Pokemon *tmp = NULL;

    if (buf->count == buf->capacity)
    {
        int new_cap = buf->capacity == 0 ? 16 : buf->capacity * 2;
        tmp = realloc(buf->items, (size_t)new_cap * sizeof(Pokemon));
        if (!tmp)
        {
            fprintf(stderr, "pokemon: memory allocation failed\n");
            return -1;
        }
        buf->items = tmp;
        buf->capacity = new_cap;
    }
    buf->items[buf->count++] = *pokemon;
    return 0;
}

static int check_stat(const char *pokemon_name, const char *stat_name, const int val)
{
    if (val < 0 || val > 255)
    {
        fprintf(stderr, "pokemon: '%s' %s (%d) must be in 0..255\n", pokemon_name, stat_name, val);
        return 0;
    }
    return 1;
}

static int has_pokemon_name(const PokemonBuffer *buf, const char *name)
{
    for (int i = 0; i < buf->count; i++)
    {
        if (strcmp(buf->items[i].name, name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

static const Move *find_move(const Move *moves, int move_count, const char *name)
{
    for (int i = 0; i < move_count; i++)
    {
        if (strcmp(moves[i].name, name) == 0)
        {
            return &moves[i];
        }
    }
    return NULL;
}

static int is_valid_pokemon(const Pokemon *p)
{
    if (p->name[0] == '\0')
    {
        fprintf(stderr, "pokemon: empty pokemon name\n");
        return 0;
    }
    return check_stat(p->name, "hp", p->hp) && check_stat(p->name, "attack", p->attack) &&
           check_stat(p->name, "defense", p->defense) && check_stat(p->name, "speed", p->speed);
}

static int parse_pokemon_stats(char **fields, Pokemon *out)
{
    snprintf(out->name, sizeof(out->name), "%s", fields[0]);
    if (str_parse_int(fields[1], &out->hp) != 0 || str_parse_int(fields[2], &out->attack) != 0 ||
        str_parse_int(fields[3], &out->defense) != 0 || str_parse_int(fields[4], &out->speed) != 0)
    {
        fprintf(stderr, "pokemon: invalid numeric stat for '%s'\n", out->name);
        return -1;
    }
    out->max_hp = out->hp;
    out->status = STATUS_NONE;
    if (type_from_string(fields[5], &out->type) != 0)
    {
        fprintf(stderr, "pokemon: invalid type '%s' for '%s'\n", fields[5], out->name);
        return -1;
    }
    return 0;
}

static int parse_pokemon_moves(char *field, const Move *moves, int move_count, Pokemon *out)
{
    char *cursor = field;
    char *comma = NULL;
    const char *name = NULL;
    int slot = 0;

    out->move_count = 0;
    while (cursor)
    {
        comma = strchr(cursor, ',');
        if (comma)
        {
            *comma = '\0';
        }

        name = str_trim(cursor);
        if (*name == '\0' || slot >= 4)
        {
            fprintf(stderr, "pokemon: invalid move list for '%s'\n", out->name);
            return -1;
        }
        slot++;

        if (strcmp(name, "-") != 0)
        {
            const Move *found = find_move(moves, move_count, name);
            if (!found)
            {
                fprintf(stderr, "pokemon: unknown move '%s' for '%s'\n", name, out->name);
                return -1;
            }
            out->moves[out->move_count++] = *found;
        }
        cursor = comma ? comma + 1 : NULL;
    }
    return 0;
}

static int add_pokemon_row(int field_count, char **fields, void *userdata)
{
    PokemonBuffer *buf = userdata;
    Pokemon p;

    if (field_count < 7)
    {
        fprintf(stderr, "pokemon: expected 7 fields, got %d\n", field_count);
        return -1;
    }
    if (parse_pokemon_stats(fields, &p) != 0 || !is_valid_pokemon(&p))
    {
        return -1;
    }
    if (has_pokemon_name(buf, p.name))
    {
        fprintf(stderr, "pokemon: duplicate pokemon '%s'\n", p.name);
        return -1;
    }
    if (parse_pokemon_moves(fields[6], buf->moves, buf->move_count, &p) != 0)
    {
        return -1;
    }
    return append_pokemon(buf, &p);
}

Pokemon *load_pokemons(const char *path, const Move *moves, int move_count, int *out_count)
{
    if (out_count)
    {
        *out_count = 0;
    }
    if (!path || !moves || move_count <= 0)
    {
        fprintf(stderr, "pokemon: invalid arguments to load_pokemons\n");
        return NULL;
    }

    PokemonBuffer buf = {NULL, 0, 0, moves, move_count};
    if (tsv_parse_file(path, add_pokemon_row, &buf) < 0)
    {
        free(buf.items);
        return NULL;
    }

    if (out_count)
    {
        *out_count = buf.count;
    }
    return buf.items;
}

void free_pokemons(Pokemon *pokemons)
{
    free(pokemons);
}

const char *status_to_string(enum Status status)
{
    static const char *const names[STATUS_COUNT] = {
        [STATUS_NONE] = "OK",
        [STATUS_POISON] = "PSN",
        [STATUS_BURN] = "BRN",
        [STATUS_PARALYSIS] = "PAR",
    };

    if (status < 0 || status >= STATUS_COUNT)
    {
        return "UNKNOWN";
    }
    return names[status];
}
