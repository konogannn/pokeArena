#pragma once

#include "pokemon.h"

typedef struct
{
    Pokemon pokemons[6];
    int count;
} Team;

void add_pokemon(Team *team, Pokemon pokemon);
void remove_pokemon(Team *team, int index);
void swap_pokemon(Team *team, int i, int j);
void heal_team(Team *team);
int is_team_valid(Team team);
