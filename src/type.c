#include "type.h"

#include <stddef.h>
#include <string.h>

const char *const TYPE_NAMES[TYPE_COUNT] = {
    [TYPE_NORMAL]   = "Normal",
    [TYPE_FIRE]     = "Fire",
    [TYPE_WATER]    = "Water",
    [TYPE_GRASS]    = "Grass",
    [TYPE_ELECTRIC] = "Electric",
    [TYPE_ICE]      = "Ice",
    [TYPE_FIGHTING] = "Fighting",
    [TYPE_POISON]   = "Poison",
    [TYPE_GROUND]   = "Ground",
    [TYPE_FLYING]   = "Flying",
    [TYPE_PSYCHIC]  = "Psychic",
    [TYPE_BUG]      = "Bug",
    [TYPE_ROCK]     = "Rock",
    [TYPE_GHOST]    = "Ghost",
    [TYPE_DRAGON]   = "Dragon",
    [TYPE_DARK]     = "Dark",
    [TYPE_STEEL]    = "Steel",
};

int type_from_string(const char *str, enum Type *out_type)
{
    if (!str || !out_type)
    {
        return -1;
    }

    for (int i = 0; i < TYPE_COUNT; i++)
    {
        if (strcmp(str, TYPE_NAMES[i]) == 0)
        {
            *out_type = (enum Type)i;
            return 0;
        }
    }
    return -1;
}

const char *type_to_string(enum Type type)
{
    if (type < 0 || type >= TYPE_COUNT)
    {
        return NULL;
    }
    return TYPE_NAMES[type];
}

double type_effectiveness(enum Type attack_type, enum Type defense_type)
{
    static const signed char chart[TYPE_COUNT][TYPE_COUNT] = {
        [TYPE_NORMAL] = {
            [TYPE_ROCK] = 1, [TYPE_GHOST] = -1, [TYPE_STEEL] = 1,
        },
        [TYPE_FIRE] = {
            [TYPE_FIRE] = 1, [TYPE_WATER] = 1, [TYPE_GRASS] = 2, [TYPE_ICE] = 2,
            [TYPE_BUG] = 2, [TYPE_ROCK] = 1, [TYPE_DRAGON] = 1, [TYPE_STEEL] = 2,
        },
        [TYPE_WATER] = {
            [TYPE_FIRE] = 2, [TYPE_WATER] = 1, [TYPE_GRASS] = 1, [TYPE_GROUND] = 2,
            [TYPE_ROCK] = 2, [TYPE_DRAGON] = 1,
        },
        [TYPE_GRASS] = {
            [TYPE_FIRE] = 1, [TYPE_WATER] = 2, [TYPE_GRASS] = 1, [TYPE_POISON] = 1,
            [TYPE_GROUND] = 2, [TYPE_FLYING] = 1, [TYPE_BUG] = 1, [TYPE_ROCK] = 2,
            [TYPE_DRAGON] = 1, [TYPE_STEEL] = 1,
        },
        [TYPE_ELECTRIC] = {
            [TYPE_WATER] = 2, [TYPE_GRASS] = 1, [TYPE_ELECTRIC] = 1, [TYPE_GROUND] = -1,
            [TYPE_FLYING] = 2, [TYPE_DRAGON] = 1,
        },
        [TYPE_ICE] = {
            [TYPE_FIRE] = 1, [TYPE_WATER] = 1, [TYPE_GRASS] = 2, [TYPE_ICE] = 1,
            [TYPE_GROUND] = 2, [TYPE_FLYING] = 2, [TYPE_DRAGON] = 2, [TYPE_STEEL] = 1,
        },
        [TYPE_FIGHTING] = {
            [TYPE_NORMAL] = 2, [TYPE_ICE] = 2, [TYPE_POISON] = 1, [TYPE_FLYING] = 1,
            [TYPE_PSYCHIC] = 1, [TYPE_BUG] = 1, [TYPE_ROCK] = 2, [TYPE_GHOST] = -1,
            [TYPE_DARK] = 2, [TYPE_STEEL] = 2,
        },
        [TYPE_POISON] = {
            [TYPE_GRASS] = 2, [TYPE_POISON] = 1, [TYPE_GROUND] = 1, [TYPE_ROCK] = 1,
            [TYPE_GHOST] = 1, [TYPE_STEEL] = -1,
        },
        [TYPE_GROUND] = {
            [TYPE_FIRE] = 2, [TYPE_GRASS] = 1, [TYPE_ELECTRIC] = 2, [TYPE_POISON] = 2,
            [TYPE_FLYING] = -1, [TYPE_BUG] = 1, [TYPE_ROCK] = 2, [TYPE_STEEL] = 2,
        },
        [TYPE_FLYING] = {
            [TYPE_GRASS] = 2, [TYPE_ELECTRIC] = 1, [TYPE_FIGHTING] = 2, [TYPE_BUG] = 2,
            [TYPE_ROCK] = 1, [TYPE_STEEL] = 1,
        },
        [TYPE_PSYCHIC] = {
            [TYPE_FIGHTING] = 2, [TYPE_POISON] = 2, [TYPE_PSYCHIC] = 1, [TYPE_DARK] = -1,
            [TYPE_STEEL] = 1,
        },
        [TYPE_BUG] = {
            [TYPE_FIRE] = 1, [TYPE_GRASS] = 2, [TYPE_FIGHTING] = 1, [TYPE_POISON] = 1,
            [TYPE_FLYING] = 1, [TYPE_PSYCHIC] = 2, [TYPE_GHOST] = 1, [TYPE_DARK] = 2,
            [TYPE_STEEL] = 1,
        },
        [TYPE_ROCK] = {
            [TYPE_FIRE] = 2, [TYPE_ICE] = 2, [TYPE_FIGHTING] = 1, [TYPE_GROUND] = 1,
            [TYPE_FLYING] = 2, [TYPE_BUG] = 2, [TYPE_STEEL] = 1,
        },
        [TYPE_GHOST] = {
            [TYPE_NORMAL] = -1, [TYPE_PSYCHIC] = 2, [TYPE_GHOST] = 2, [TYPE_DARK] = 1,
            [TYPE_STEEL] = 1,
        },
        [TYPE_DRAGON] = {
            [TYPE_DRAGON] = 2, [TYPE_STEEL] = 1,
        },
        [TYPE_DARK] = {
            [TYPE_FIGHTING] = 1, [TYPE_PSYCHIC] = 2, [TYPE_GHOST] = 2, [TYPE_DARK] = 1,
            [TYPE_STEEL] = 1,
        },
        [TYPE_STEEL] = {
            [TYPE_FIRE] = 1, [TYPE_WATER] = 1, [TYPE_ELECTRIC] = 1, [TYPE_ICE] = 2,
            [TYPE_ROCK] = 2, [TYPE_STEEL] = 1,
        },
    };

    if (attack_type < 0 || attack_type >= TYPE_COUNT ||
        defense_type < 0 || defense_type >= TYPE_COUNT)
    {
        return 1.0;
    }

    switch (chart[attack_type][defense_type])
    {
        case -1:
            return 0.0;
        case 1:
            return 0.5;
        case 2:
            return 2.0;
        default:
            return 1.0;
    }
}
