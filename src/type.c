#include "type.h"

#include <stddef.h>
#include <string.h>

const char *const TYPE_NAMES[TYPE_COUNT] = {
    [TYPE_NORMAL]   = "TYPE_NORMAL",
    [TYPE_FIRE]     = "TYPE_FIRE",
    [TYPE_WATER]    = "TYPE_WATER",
    [TYPE_GRASS]    = "TYPE_GRASS",
    [TYPE_ELECTRIC] = "TYPE_ELECTRIC",
    [TYPE_ICE]      = "TYPE_ICE",
    [TYPE_FIGHTING] = "TYPE_FIGHTING",
    [TYPE_POISON]   = "TYPE_POISON",
    [TYPE_GROUND]   = "TYPE_GROUND",
    [TYPE_FLYING]   = "TYPE_FLYING",
    [TYPE_PSYCHIC]  = "TYPE_PSYCHIC",
    [TYPE_BUG]      = "TYPE_BUG",
    [TYPE_ROCK]     = "TYPE_ROCK",
    [TYPE_GHOST]    = "TYPE_GHOST",
    [TYPE_DRAGON]   = "TYPE_DRAGON",
    [TYPE_DARK]     = "TYPE_DARK",
    [TYPE_STEEL]    = "TYPE_STEEL",
};

int type_from_string(const char *str, enum Type *out_type) {
    if (!str || !out_type) {
        return -1;
    }

    for (int i = 0; i < TYPE_COUNT; i++) {
        if (strcmp(str, TYPE_NAMES[i]) == 0) {
            *out_type = (enum Type)i;
            return 0;
        }
    }
    return -1;
}

const char *type_to_string(enum Type type) {
    if (type < 0 || type >= TYPE_COUNT) {
        return NULL;
    }
    return TYPE_NAMES[type];
}
