#pragma once

#include "type.h"

enum MoveEffect
{
    EFFECT_NONE,
    EFFECT_POISON,
    EFFECT_BURN,
    EFFECT_PARALYSIS,
    EFFECT_CRIT,
    EFFECT_HEAL,
    EFFECT_COUNT
};

typedef struct
{
    char name[32];
    int power;
    int accuracy;
    enum Type type;
    enum MoveEffect effect;
    int effect_chance;
    int pp;
    int max_pp;
} Move;

/* Loads moves from a TSV file into a heap-allocated array.
 * Sets *out_count to the number of loaded moves (if non-NULL).
 * Returns NULL on error (and frees any partial allocations).
 * Caller must free the returned array with free_moves(). */
Move *load_moves(const char *path, int *out_count);

void free_moves(Move *moves);

const char *move_effect_to_string(enum MoveEffect effect);
