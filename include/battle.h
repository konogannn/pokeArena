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
    int forfeited_player;
    int is_pvp;
    int difficulty;

    int damage_dealt;
    int damage_taken;
    int kos_inflicted;
    int kos_suffered;
} Battle;

void start_battle(Battle *battle);
void execute_turn(Battle *battle);
void execute_attack(Pokemon *attacker, Pokemon *defender, Move move);
int calculate_damage(Pokemon attacker, Pokemon defender, Move move);
int is_pokemon_alive(Pokemon pokemon);
int is_team_defeated(Team team);
