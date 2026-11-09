#pragma once

#include "type.h"

typedef struct {
    char name[32];
    int power;
    int accuracy;
    enum Type type;
} Move;

/* Loads moves from a TSV file into a heap-allocated array.
 * Sets *out_count to the number of loaded moves (if non-NULL).
 * Returns NULL on error (and frees any partial allocations).
 * Caller must free the returned array with free_moves(). */
Move *load_moves(const char *path, int *out_count);

void free_moves(Move *moves);
