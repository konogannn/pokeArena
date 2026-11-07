#pragma once

#include "moves.h"

typedef struct {
    char name[32];

    int hp;
    int max_hp;

    int attack;
    int defense;
    int speed;

    Type type;

    Move moves[4];
    int move_count;
} Pokemon;
