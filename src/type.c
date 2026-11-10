#include "type.h"

#include <stddef.h>
#include <string.h>

const char *const TYPE_NAMES[TYPE_COUNT] = {
    [TYPE_NORMAL] = "Normal",     [TYPE_FIRE] = "Fire",
    [TYPE_WATER] = "Water",       [TYPE_GRASS] = "Grass",
    [TYPE_ELECTRIC] = "Electric", [TYPE_ICE] = "Ice",
    [TYPE_FIGHTING] = "Fighting", [TYPE_POISON] = "Poison",
    [TYPE_GROUND] = "Ground",     [TYPE_FLYING] = "Flying",
    [TYPE_PSYCHIC] = "Psychic",   [TYPE_BUG] = "Bug",
    [TYPE_ROCK] = "Rock",         [TYPE_GHOST] = "Ghost",
    [TYPE_DRAGON] = "Dragon",     [TYPE_DARK] = "Dark",
    [TYPE_STEEL] = "Steel"
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
