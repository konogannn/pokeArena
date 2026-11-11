#pragma once

#include "moves.h"

enum Status {
    STATUS_NONE,
    STATUS_POISON,
    STATUS_BURN,
    STATUS_PARALYSIS,
    STATUS_COUNT
};

typedef struct {
    char name[32];

    int hp;
    int max_hp;

    int attack;
    int defense;
    int speed;

    enum Type type;
    enum Status status;

    Move moves[4];
    int move_count;
} Pokemon;

/* Loads pokemon from a TSV file and validates their moves against moves[0..move_count-1].
 * Sets *out_count to the number of loaded pokemon (if non-NULL).
 * Returns NULL on error (and frees any partial allocations).
 * Caller must free the returned array with free_pokemons(). */
Pokemon *load_pokemons(const char *path, const Move *moves, int move_count, int *out_count);

void free_pokemons(Pokemon *pokemons);

void display_pokemon(const Pokemon *pokemon);
void display_pokemon_stats(const Pokemon *pokemon);
void animate_hp_change(const Pokemon *pokemon, int old_hp, int new_hp);
const char *status_to_string(enum Status status);
