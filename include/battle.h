#pragma once

#include "team.h"

typedef struct
{
    Team player_team;
    Team opponent_team;

    int player_active;
    int opponent_active;

    int turn;
    int forfeited;
    int difficulty;
} Battle;

void start_battle(Battle *battle);
int calculate_damage(Pokemon attacker, Pokemon defender, Move move);
int is_pokemon_alive(Pokemon pokemon);
int is_team_defeated(Team team);
