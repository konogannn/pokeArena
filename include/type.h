#pragma once

enum Type {
    TYPE_NORMAL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_GRASS,
    TYPE_ELECTRIC,
    TYPE_ICE,
    TYPE_FIGHTING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_FLYING,
    TYPE_PSYCHIC,
    TYPE_BUG,
    TYPE_ROCK,
    TYPE_GHOST,
    TYPE_DRAGON,
    TYPE_DARK,
    TYPE_STEEL,
    TYPE_COUNT
};

extern const char *const TYPE_NAMES[TYPE_COUNT];

int type_from_string(const char *str, enum Type *out_type);
const char *type_to_string(enum Type type);
double type_effectiveness(enum Type attack_type, enum Type defense_type);
