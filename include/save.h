#pragma once

#include "pokemon.h"
#include "team.h"

int save_team(const char *path, const Team *team, int badges_mask);
int load_team(const char *path, const Pokemon *pokedex, int pokedex_count, Team *out_team,
              int *out_badges_mask);
