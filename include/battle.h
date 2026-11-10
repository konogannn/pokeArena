#pragma once

#include "team.h"

typedef struct
{
    Team player_team;
    Team opponent_team;

    int player_active;
    int opponent_active;

    int turn;
} Battle;
